#define TRUE  1
#define FALSE 0

typedef enum {
    TASK1 = 0,
    TASK2,
    TASK3,
    TASK4,
    TASK5,
    TASKIDLE,
    NUMBER_OF_TASKS,
} TaskIdType;

typedef enum {
    SEM1 = 0,
    SEM2,
    SEM3,
    SEM4,
    SEM5,
    NUMBER_OF_SEMS,
} SemIdType;

#define NO_SEM NUMBER_OF_SEMS

extern void Task1(void);
extern void Task2(void);
extern void Task3(void);
extern void Task4(void);
extern void Task5(void);
extern void Idle(void);

#define STACKSIZE 0x1000

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

struct TaskControl {
    enum { READY, BLOCKED} state;
    void (*entry)(void);
    long time_slice;
    long remaining_time;
    int expire;
    SemIdType target_sem;
    unsigned long sp;
    unsigned long task_stack[STACKSIZE];
} TaskControl[NUMBER_OF_TASKS] = {
    {.entry = Task1, .state = READY, .time_slice = 2},
    {.entry = Task2, .state = READY, .time_slice = 4},
    {.entry = Task3, .state = READY, .time_slice = 1}, 
    {.entry = Task4, .state = READY, .time_slice = 3}, 
    {.entry = Task5, .state = READY, .time_slice = 1}, 
    {.entry = Idle,  .state = READY,  .time_slice = 1}, 
};

#define SEM_AVAILABLE TASKIDLE

struct SemaphoreControl {
    TaskIdType owner_task;
} SemaphoreControl[NUMBER_OF_SEMS] = {
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE},
    {.owner_task = SEM_AVAILABLE}, 
};

TaskIdType CurrentTask;

extern void AcquireSemaphore(SemIdType sem);
extern int TryToAcquireSemaphore(SemIdType sem);
extern void ReleaseSemaphore(SemIdType sem);
extern void _AcquireSemaphore(SemIdType sem);
extern int _TryToAcquireSemaphore(SemIdType sem);
extern void _ReleaseSemaphore(SemIdType sem);
extern void Snooze(int tim);
extern void _Snooze(int tim);
extern void Schedule(void);
extern void _Schedule(void);
extern int switch_context(unsigned long *next_sp, unsigned long* sp);
extern void load_context(unsigned long *sp);
extern void TaskSwitch(struct TaskControl *current, struct TaskControl *next);
extern void TaskStart(void (*entry)(void));
extern void EnableTimer(void);
extern void EnableInt(void);
extern void DisableInt(void);
extern void print_message(const char* s);
extern void spend_time(void);
extern unsigned long get_time(void);
extern unsigned long _get_time(void);
extern void trap_vectors(void);
extern void SetTrapVectors(unsigned long);
extern void _start(void);
extern void InitPMP(unsigned long pmpaddr);

#define MTVEC_VECTORED_MODE 0x1U

static void put_char(char c)
{
    volatile unsigned char * const uart = (unsigned char *)0x10000000U;
    *uart = c;
}

void _print_message(const char *s)
{
    while (*s) put_char(*s++);
}

unsigned int myrandom(void)
{
    static unsigned long long x = 11;
    x = (48271 * x) % 2147483647;
    return (unsigned int)x;
}


int GetForks(SemIdType fork_left, SemIdType fork_right)
{
    AcquireSemaphore(fork_left);
    if (TryToAcquireSemaphore(fork_right)) {
        return TRUE;
    }
    ReleaseSemaphore(fork_left);
    return FALSE;
}

void ReleaseForks(SemIdType fork_left, SemIdType fork_right)
{
    ReleaseSemaphore(fork_left);
    ReleaseSemaphore(fork_right);
}

void PhilosopherMeditate()
{
    print_message("    Meditating\n");
    Snooze(myrandom() % 2 + 1);
}

void PhilosopherEat()
{
    print_message("    Eating\n");
    spend_time();
    Snooze(myrandom() % 5 + 1);
}

void TaskJob(const TaskIdType task, const SemIdType fork_left, const SemIdType fork_right)
{
    char taskname[] = "Task?";
    taskname[4] = '1' + task;

    while (1) {
        while ( !GetForks(fork_left, fork_right) ) {
            print_message(taskname);
            PhilosopherMeditate();
        }
        print_message(taskname);
        PhilosopherEat();
        ReleaseForks(fork_left, fork_right);
    }
}

void Task1(void)
{
    TaskJob(TASK1, SEM1, SEM2);
}

void Task2(void)
{
    TaskJob(TASK2, SEM2, SEM3);
}

void Task3(void)
{
    TaskJob(TASK3, SEM3, SEM4);
}

void Task4(void)
{
    TaskJob(TASK4, SEM4, SEM5);
}

void Task5(void)
{
    TaskJob(TASK5, SEM5, SEM1);
}

void Idle(void)
{
    while (1) {
        /* do nothing */
    }
}

void TaskSwitch(struct TaskControl *current, struct TaskControl *next)
{
    switch_context(&next->sp, &current->sp);
}

static TaskIdType ChooseNextTask(void)
{
    /* roundrobin scheduling */
    TaskIdType task = CurrentTask;

    do {
        task = (task + 1) % NUMBER_OF_TASKS;
    } while ((TaskControl[task].state != READY || task == TASKIDLE) && task != CurrentTask);

    if (TaskControl[task].state != READY) {
        task = TASKIDLE;
    }

    return task;
}

void _Schedule(void)
{
    TaskIdType from = CurrentTask;
    CurrentTask = ChooseNextTask();
    if (from != CurrentTask) {
        TaskSwitch(&TaskControl[from], &TaskControl[CurrentTask]); 
    }
}

void _TaskBlock(void)
{
    TaskControl[CurrentTask].state = BLOCKED;
    _Schedule();
}

void _TaskUnblock(TaskIdType task)
{
    TaskControl[task].state = READY;
    _Schedule();
}

void _Snooze(int tim)
{
    TaskControl[CurrentTask].expire = tim;
    _TaskBlock();
}

void _AcquireSemaphore(SemIdType sem)
{
    while (SemaphoreControl[sem].owner_task != SEM_AVAILABLE) {
        TaskControl[CurrentTask].target_sem = sem;
        _TaskBlock();
    }
    SemaphoreControl[sem].owner_task = CurrentTask;
}

int _TryToAcquireSemaphore(SemIdType sem)
{
    if (SemaphoreControl[sem].owner_task == SEM_AVAILABLE) {
        SemaphoreControl[sem].owner_task = CurrentTask;
    }
    return SemaphoreControl[sem].owner_task == CurrentTask;
}

void _ReleaseSemaphore(SemIdType sem)
{
    TaskIdType task;
    SemaphoreControl[sem].owner_task = SEM_AVAILABLE;
    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        if (TaskControl[task].state == BLOCKED && TaskControl[task].target_sem == sem) {
            TaskControl[task].target_sem = NO_SEM;
            TaskControl[task].expire = 0; /* XXX */
            _TaskUnblock(task);
        }
    }
}

static void TaskEntry(void)
{
    TaskStart(TaskControl[CurrentTask].entry);
}

static void InitTask(TaskIdType task)
{
    context* p = (context *)&TaskControl[task].task_stack[STACKSIZE] - 1;
    p->ra = (unsigned long)TaskEntry;
    TaskControl[task].sp = (unsigned long)p;
    TaskControl[task].remaining_time = TaskControl[task].time_slice;
    TaskControl[task].target_sem = NO_SEM;
}

volatile unsigned long * const reg_mtime = ((unsigned long *)0x200BFF8U);
volatile unsigned long * const reg_mtimecmp = ((unsigned long *)0x2004000U);

#define INTERVAL 10000000

int Timer(void)
{
    TaskIdType task;

    _print_message("Timer\n");

    do {
        *reg_mtimecmp += INTERVAL;
    } while ((long)(*reg_mtime - *reg_mtimecmp) >= 0);

    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        if (TaskControl[task].state == BLOCKED && TaskControl[task].expire > 0) {
            if (--TaskControl[task].expire == 0) {
                TaskControl[task].state = READY;
            }
        }
    }

    if (--TaskControl[CurrentTask].remaining_time <= 0) {
        TaskControl[CurrentTask].remaining_time = TaskControl[CurrentTask].time_slice;
        return TRUE;
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

static void StartTimer(void)
{
    *reg_mtimecmp = *reg_mtime + INTERVAL;
    EnableTimer();
}

unsigned long _get_time(void)
{
    return *reg_mtime;
}

void spend_time(void)
{
    unsigned long t = get_time();
    while ( get_time() - t < INTERVAL/4 );
}

static void clearbss(void)
{
    unsigned long long *p;
    extern unsigned long long _bss_start[];
    extern unsigned long long _bss_end[];

    for (p = _bss_start; p < _bss_end; p++) {
        *p = 0LL;
    }
}

static void SetupPMP(void)
{
    extern unsigned char ram_size[];  /* The size must be a power of 2 */
    extern unsigned char ram_start[]; /* The address must be multiples of the size */
    /* map the whole ram region */
    InitPMP(((unsigned long)ram_start >> 2U) + ((unsigned long)ram_size >> 3U) - 1U);
}

void main(void) {
    TaskIdType task;

    clearbss();
    SetTrapVectors((unsigned long)trap_vectors + MTVEC_VECTORED_MODE);
    SetupPMP();

    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        InitTask(task);
    }

    StartTimer();

    CurrentTask = TASK1;
    load_context(&TaskControl[CurrentTask].sp);
}
