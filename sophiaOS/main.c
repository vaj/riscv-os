/*
MIT License

Copyright (c) 2021 VA Linux Systems Japan, K.K.

Author : Hirokazu Takahashi <taka@valinux.co.jp>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "api.h"
#include "sbi.h"
#include "vma.h"

#define NO_SEM NUMBER_OF_SEMS
#define KSTACKSIZE 0x800

typedef enum {
    CORE0 = 0,
    CORE1,
    CORE2,
    NUMBER_OF_CORES,
    CORE_UNASSIGNED = NUMBER_OF_CORES,
} CoreIdType;

typedef struct {
    unsigned long s0;
    unsigned long s1;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long s6;
    unsigned long s7;
    unsigned long s8;
    unsigned long s9;
    unsigned long s10;
    unsigned long s11;
    unsigned long ra;
} context;

typedef volatile int Lock_t;

struct TaskControl {
    enum { READY, BLOCKED } state;
    CoreIdType coreid;
    void (*entry)(void);
    long time_slice;
    long remaining_time;
    int expire;
    SemIdType target_sem;
    unsigned long sp;
    unsigned long task_kstack[KSTACKSIZE];
} TaskControl[NUMBER_OF_TASKS] = {
    {.coreid = CORE_UNASSIGNED, .entry = Task1, .state = READY, .time_slice = 2},
    {.coreid = CORE_UNASSIGNED, .entry = Task2, .state = READY, .time_slice = 4},
    {.coreid = CORE_UNASSIGNED, .entry = Task3, .state = READY, .time_slice = 1},
    {.coreid = CORE_UNASSIGNED, .entry = Task4, .state = READY, .time_slice = 3},
    {.coreid = CORE_UNASSIGNED, .entry = Task5, .state = READY, .time_slice = 1},
    {.coreid = CORE_UNASSIGNED, .entry = Idle, .state = READY, .time_slice = 1},
    {.coreid = CORE_UNASSIGNED, .entry = Idle, .state = READY, .time_slice = 1},
    {.coreid = CORE_UNASSIGNED, .entry = Idle, .state = READY, .time_slice = 1},
};

#define SEM_AVAILABLE TASKIDLE

struct SemaphoreControl {
    TaskIdType owner_task;
    Lock_t lock;
} SemaphoreControl[NUMBER_OF_SEMS] = {
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
};

__thread TaskIdType CurrentTask;
__thread CoreIdType ThisCore;

static Lock_t lock_sched;

extern void _AcquireSemaphore(SemIdType sem);
extern int _TryToAcquireSemaphore(SemIdType sem);
extern void _ReleaseSemaphore(SemIdType sem);
extern void _Snooze(int tim);
extern void _Schedule(void);
extern int switch_context(unsigned long *next_sp, unsigned long* sp);
extern void load_context(unsigned long *sp);
extern void TaskSwitch(struct TaskControl *current, struct TaskControl *next);
extern void TaskStart(void (*entry)(void), unsigned long *usp);
extern void EnableInterrupts(void);
extern unsigned long _get_time(void);
extern void trap_vectors(void);
extern void SetTrapVectors(unsigned long);
extern int TestAndSet(Lock_t *lock, int newval);
extern void MemBarrier(void);
extern void Pause(void);
extern void _secondary_start(unsigned long hartid);
extern void clear_IPI(void);

#define TVEC_VECTORED_MODE 0x1U

#define INT_SMODE_SOFT    1
#define INT_SMODE_TIMER   5

void SpinLock(Lock_t *lock)
{
    while (TestAndSet(lock, ThisCore + 1)) {
        Pause();
    }
}

void SpinUnlock(Lock_t *lock)
{
    MemBarrier();
    *lock = 0;
}

void broardcast_IPI(void)
{
    unsigned long core_mask = 0U;
    CoreIdType core = ThisCore + 1;
    do {
        core_mask |= (1U << core);
        core = (core + 1) % NUMBER_OF_CORES;
    } while (core != ThisCore);

    sbi_send_ipi(core_mask, 0U);
}

static void put_char(char c)
{
    volatile unsigned char * const uart = (unsigned char *)0x10000000U;
    *uart = c;
}

static void __print_message(const char *s, va_list ap)
{
    while (*s) {
        if (*s == '%' && *(s+1) == 'x') {
            unsigned long v = va_arg(ap, unsigned long);
            _Bool print_started = FALSE;
            int i;

            s += 2;
            for (i = 15; i >= 0; i--) {
                unsigned long x = (v & 0xFUL << i*4) >> i*4;
                if (print_started || x != 0U || i == 0) {
                    print_started = TRUE;
                    put_char((x < 10 ? '0' : 'a' - 10) + x);
                }
            }
        } else {
            put_char(*s++);
        }
    }
}

void _print_message(const char *s, ...)
{
    unsigned long coreid = ThisCore;
    static Lock_t lock_uart;

    SpinLock(&lock_uart);

    __print_message("core%x: ", &coreid);

    va_list ap;
    va_start (ap, s);
    __print_message(s, ap);
    va_end (ap);

    SpinUnlock(&lock_uart);
}

void TaskSwitch(struct TaskControl *current, struct TaskControl *next)
{
    switch_context(&next->sp, &current->sp);
}

static TaskIdType ChooseNextTask(void)
{
    /* roundrobin scheduling */
    TaskIdType task = CurrentTask;

    TaskControl[task].coreid = CORE_UNASSIGNED;

    do {
        task = (task + 1) % NUMBER_OF_TASKS;
    } while ((TaskControl[task].state != READY || TaskControl[task].coreid != CORE_UNASSIGNED || task >= TASKIDLE) && task != CurrentTask);

    if (TaskControl[task].state != READY) {
        task = TASKIDLE + ThisCore;
    }
    TaskControl[task].coreid = ThisCore;

    return task;
}

void _Schedule(void)
{
    SpinLock(&lock_sched);
    TaskIdType from = CurrentTask;
    CurrentTask = ChooseNextTask();
    if (from != CurrentTask) {
        TaskSwitch(&TaskControl[from], &TaskControl[CurrentTask]);
    }
    SpinUnlock(&lock_sched);
}

static void TaskSetReady(TaskIdType task)
{
    MemBarrier();
    TaskControl[task].state = READY;
    broardcast_IPI();
}

void _TaskBlock(void)
{
    MemBarrier();
    TaskControl[CurrentTask].state = BLOCKED;
    _Schedule();
}

void _TaskUnblock(TaskIdType task)
{
    TaskSetReady(task);
    _Schedule();
}

void _Snooze(int tim)
{
    TaskControl[CurrentTask].expire = tim;
    _TaskBlock();
}

void _AcquireSemaphore(SemIdType sem)
{
    SpinLock(&SemaphoreControl[sem].lock);
    while (SemaphoreControl[sem].owner_task != SEM_AVAILABLE) {
        TaskControl[CurrentTask].target_sem = sem;
        TaskControl[CurrentTask].state = BLOCKED;
        SpinUnlock(&SemaphoreControl[sem].lock);
        _Schedule();
        SpinLock(&SemaphoreControl[sem].lock);
    }
    SemaphoreControl[sem].owner_task = CurrentTask;
    SpinUnlock(&SemaphoreControl[sem].lock);
}

int _TryToAcquireSemaphore(SemIdType sem)
{
    SpinLock(&SemaphoreControl[sem].lock);
    if (SemaphoreControl[sem].owner_task == SEM_AVAILABLE) {
        SemaphoreControl[sem].owner_task = CurrentTask;
    }
    SpinUnlock(&SemaphoreControl[sem].lock);
    return SemaphoreControl[sem].owner_task == CurrentTask;
}

void _ReleaseSemaphore(SemIdType sem)
{
    TaskIdType task;
    SpinLock(&SemaphoreControl[sem].lock);
    SemaphoreControl[sem].owner_task = SEM_AVAILABLE;
    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        if (TaskControl[task].state == BLOCKED && TaskControl[task].target_sem == sem) {
            TaskControl[task].target_sem = NO_SEM;
            TaskControl[task].expire = 0; /* XXX */
            SpinUnlock(&SemaphoreControl[sem].lock);
            _TaskUnblock(task);
            SpinLock(&SemaphoreControl[sem].lock);
        }
    }
    SpinUnlock(&SemaphoreControl[sem].lock);
}

static void TaskEntry(void)
{
    SpinUnlock(&lock_sched);
    TaskStart(TaskControl[CurrentTask].entry, &task_ustack[CurrentTask][USTACKSIZE]);
}

static void InitTask(TaskIdType task)
{
    context* p = (context *)&TaskControl[task].task_kstack[KSTACKSIZE] - 1;
    p->ra = (unsigned long)TaskEntry;
    TaskControl[task].sp = (unsigned long)p;
    TaskControl[task].remaining_time = TaskControl[task].time_slice;
    TaskControl[task].target_sem = NO_SEM;
}

static __thread unsigned long nexttime;

#define INTERVAL 10000000U

static int Timer(void)
{
    TaskIdType task;

    _print_message("Timer\n");

    do {
        nexttime += INTERVAL;
        sbi_set_timer(nexttime);
    } while ((long)(_get_time() - nexttime) >= 0);

    if (ThisCore == CORE0) {
        for (task = 0; task < NUMBER_OF_TASKS; task++) {
            if (TaskControl[task].state == BLOCKED && TaskControl[task].expire > 0) {
                if (--TaskControl[task].expire == 0) {
                    TaskSetReady(task);
                }
            }
        }
    }

    if (--TaskControl[CurrentTask].remaining_time <= 0) {
        TaskControl[CurrentTask].remaining_time = TaskControl[CurrentTask].time_slice;
        return TRUE;
    }

    return FALSE;
}

static int InterCoreInt(void)
{
    _print_message("Inter-Core Interrupt\n");

    clear_IPI();

    if (CurrentTask >= TASKIDLE) {
        return TRUE;   /* reschedule request */
    }
    return FALSE;
}

int InterruptHandler(unsigned long cause)
{
    switch ((unsigned short)cause) {
    case INT_SMODE_SOFT:
        return InterCoreInt();
        break;
    case INT_SMODE_TIMER:
        return Timer();
        break;
    default:
        break;
    }
    return FALSE;
}

long SvcHandler(long a0, long a1, long a2, long a3, long a4, long a5, long a6, long sysno)
{
    typedef long (*syscall_t)(long a0, long a1, long a2, long a3, long a4, long a5, long a6);
    const syscall_t systable[] = {
        (syscall_t)_Schedule,
        (syscall_t)_AcquireSemaphore,
        (syscall_t)_TryToAcquireSemaphore,
        (syscall_t)_ReleaseSemaphore,
        (syscall_t)_Snooze,
        (syscall_t)_print_message,
        (syscall_t)_get_time,
    };

    return systable[sysno](a0, a1, a2, a3, a4, a5, a6);
}

int ExcHandler(unsigned long* ctx, unsigned long sepc, unsigned long scause, unsigned long sstatus, unsigned long stval)
{
    _print_message("Exception: sepc(0x%x) scause(0x%x) sstatus(0x%x) stval(0x%x), sp(0x%x) ra(0x%x) s0(0x%x) s1(0x%x)\n", sepc, scause, sstatus, stval, ctx[2], ctx[1], ctx[8], ctx[9]);
    _print_message("           a0(0x%x) a1(0x%x) a2(0x%x) a3(0x%x) a4(0x%x) a5(0x%x)\n", ctx[10], ctx[11], ctx[12], ctx[13], ctx[14], ctx[15]);
}

static void StartTimer(void)
{
    nexttime = _get_time() + INTERVAL;
    sbi_set_timer(nexttime);
}

static void mem_clear(unsigned char *start, unsigned char *end)
{
    unsigned char *p;
    for (p = start; p < end; p++) {
        *p = 0LL;
    }
}

static void mem_copy(unsigned char *to, unsigned char *from, unsigned int size)
{
    unsigned int sz;
    for (sz = 0; sz < size; sz++) {
        to[sz] = from[sz];
    }
}

static void clearbss(void)
{
    extern unsigned char _system_bss_start[];
    extern unsigned char _system_bss_end[];
    extern unsigned char _bss_start[];
    extern unsigned char _bss_end[];

    mem_clear(_system_bss_start, _system_bss_end);
    mem_clear(_bss_start, _bss_end);
}

static void setupTLS(void)
{
    extern unsigned char _tls_start[];
    extern unsigned char _tdata_start[];
    extern unsigned char _tdata_end[];
    extern unsigned char _tbss_start[];
    extern unsigned char _tbss_end[];

    const unsigned int tdata_size = _tdata_end - _tdata_start;
    const unsigned int tbss_size = _tbss_end - _tbss_start;
    register unsigned char* threadp asm("tp");

    mem_copy(threadp, _tdata_start, tdata_size);
    mem_clear(&threadp[tbss_size], &threadp[tdata_size + tbss_size]);
}

static volatile CoreIdType BootCore = CORE_UNASSIGNED;

static void sync_cores(void)
{
    volatile static _Bool notyet = TRUE;

    if (ThisCore == BootCore) {
        MemBarrier();
        notyet = FALSE;
    } else {
        while (notyet);
    }
}

static void start_cores(void)
{
    int coreid;

    for (coreid = CORE0; coreid < NUMBER_OF_CORES; coreid++) {
        if (coreid != BootCore) {
            struct sbiret ret = sbi_hart_start(coreid, (unsigned long)_secondary_start, coreid);
            if (ret.error) {
                _print_message("sbi_hart_start(core%x) failed. error(0x%x)\n", coreid, ret.error);
            }
        }
    }
}

static void restart_bootcore(void)
{
    struct sbiret ret;
    CoreIdType coreid;

    for (coreid = CORE0; coreid < NUMBER_OF_CORES; coreid++) {
        struct sbiret ret = sbi_hart_get_status(coreid);
        if (ret.error == SBI_SUCCESS && ret.value == STARTED) {
            BootCore = coreid;
            _secondary_start(BootCore);
        }
    }
}

void main(CoreIdType coreid)
{
    TaskIdType task;

    SetTrapVectors((unsigned long)trap_vectors + TVEC_VECTORED_MODE);

    if (BootCore == CORE_UNASSIGNED) {
        restart_bootcore();
    }

    setupTLS();
    ThisCore = coreid;

    if (ThisCore == BootCore) {
        clearbss();
        SetupPageTables();

        for (task = 0; task < NUMBER_OF_TASKS; task++) {
            InitTask(task);
        }

        start_cores();
    }
    sync_cores();

    EnableMMU();

    StartTimer();
    EnableInterrupts();

    _print_message("Core%x started.\n", ThisCore);

    SpinLock(&lock_sched);
    CurrentTask = TASKIDLE + ThisCore;
    TaskControl[CurrentTask].coreid = ThisCore;
    load_context(&TaskControl[CurrentTask].sp);
}

