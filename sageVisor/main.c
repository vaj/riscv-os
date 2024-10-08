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

#include <stdarg.h>
#include "sbi.h"
#include "vma.h"

#define TRUE  1
#define FALSE 0

#define STACKSIZE 0x800
#define NCORE 8

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

/* 2 phyical cores */
typedef enum {
    CORE0 = 0,
    CORE1,
    NUMBER_OF_CORES,
    CORE_UNASSIGNED = NUMBER_OF_CORES,
    CORE_RESERVED,
} CoreIdType;

/* 4 virtual cpus */
typedef enum {
    VCPU0 = 0,
    VCPU1,
    VCPU2,
    VCPU3,
    NUMBER_OF_VCPUS,
} VcpuIdType;

typedef enum {
    VM0 = 0,
    VM1,
    NUMBER_OF_VMS,
} VmIdType;

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

    unsigned long vsstatus;
    unsigned long vsepc;
    unsigned long vscause;
    unsigned long vstval;
    unsigned long vsie;
    unsigned long vstvec;
    unsigned long vsscratch;
    unsigned long vsatp;
    unsigned long hvip;
    unsigned long hstatus;
} context;

typedef volatile int Lock_t;
extern unsigned char ram_guest1_start[];
extern unsigned char ram_guest1_size[];
extern unsigned char ram_guest2_start[];
extern unsigned char ram_guest2_size[];

struct VcpuControl {
    volatile enum { CORE_STARTED = STARTED, CORE_STOPPED } state;
    CoreIdType coreid;  /* this phisical core assigned */
    unsigned long next_time;
    unsigned int vmid;
    volatile int ipi_request;
    void (*entry)(unsigned long param);
    unsigned long param;
    unsigned long sp;
    unsigned long stack[STACKSIZE];
} VcpuControl[NUMBER_OF_VCPUS] = {
    {.coreid = CORE_RESERVED,   .state = STOPPED, .vmid = VM0},
    {.coreid = CORE_RESERVED,   .state = STOPPED, .vmid = VM1},
    {.coreid = CORE_UNASSIGNED, .state = STOPPED, .vmid = VM1},
    {.coreid = CORE_UNASSIGNED, .state = STOPPED, .vmid = VM1},
};

#define REQ_VSSI 0x1U

struct VmControl {
    VcpuIdType boot_vcpu;
    unsigned long vm_hgatp;
    unsigned long vm_start;
    unsigned long vm_size;
    unsigned long vm_io_start;
    unsigned long vm_io_size;
} VmControl[NUMBER_OF_VCPUS] = {
    {.boot_vcpu = VCPU0, .vm_start = (unsigned long)ram_guest2_start, .vm_size = (unsigned long)ram_guest2_size, .vm_io_start = 0x10000000UL, .vm_io_size = 0x200000UL, },
    {.boot_vcpu = VCPU1, .vm_start = (unsigned long)ram_guest1_start, .vm_size = (unsigned long)ram_guest1_size, .vm_io_start = 0x10000000UL, .vm_io_size = 0x200000UL, },
};

/* the entry address of guests is the same as of the hypervisor */
#define VM_ENTRY_ADDR _start

__thread VcpuIdType CurrentVcpu;
__thread CoreIdType ThisCore;

static Lock_t lock_sched;

extern void _Snooze(int tim);
extern int switch_context(unsigned long *next_sp, unsigned long* sp);
extern void load_context(unsigned long *sp);
extern void VcpuSwitch(struct VcpuControl *current, struct VcpuControl *next);
extern void VcpuStart(void (*entry)(unsigned long vcpu), unsigned long vcpu);
extern void EnableInterrupts(void);
extern unsigned long _get_time(void);
extern void trap_vectors(void);
extern void SetTrapVectors(unsigned long);
extern int TestAndSet(Lock_t *lock, int newval);
extern void MemBarrier(void);
extern void Pause(void);
extern void SetupDeleg(void);
extern void _start(unsigned long vcpu);
extern void _secondary_start(unsigned long vcpu);
extern void clear_IPI(void);
extern void raiseVSI(unsigned long type);
extern void clearVSI(unsigned long type);
extern unsigned long GetVSTvec(void);
extern void SetVSmodeRegs(unsigned long sepc, unsigned long scause, unsigned long stval);
extern void EnableCounters(unsigned int types);
extern void SetHGATP(unsigned long pagetable);
extern unsigned long CreateVmSpace(unsigned long vm, unsigned long paddr, unsigned long size, unsigned long vaddr);
extern unsigned long CreateVmIoSpace(unsigned long vm, unsigned long paddr, unsigned long size); /* pass through I/O */

#define TVEC_VECTORED_MODE 0x1U

#define INT_SMODE_SOFT    1
#define INT_VSMODE_SOFT   2
#define INT_SMODE_TIMER   5
#define INT_VSMODE_TIMER  6
#define IP_VSSI  (0x1U<<INT_VSMODE_SOFT)
#define IP_VSTI  (0x1U<<INT_VSMODE_TIMER)

#define STATUS_SPP       (1U<<8)
#define SPP_SMODE        STATUS_SPP     /* supervisor mode */
#define SPP_UMODE        (0U<<8)        /* user mode */
#define STATUS_SPIE      (1U<<5)
#define STATUS_SIE       (1U<<1)
#define STATUS_SUM       (1U<<18)
#define HSTATUS_SPV      (1U<<7)        /* virtualized mode */
#define HSTATUS_SPVP     (1U<<8)

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
                    sbi_debug_console_write_byte((x < 10 ? '0' : 'a' - 10) + x);
                }
            }
        } else {
            sbi_debug_console_write_byte(*s++);
        }
    }
}

void _print_message(const char *s, ...)
{
    unsigned long coreid = ThisCore;
    static Lock_t lock_uart;

    SpinLock(&lock_uart);

    __print_message("Physical core%x: ", &coreid);

    va_list ap;
    va_start (ap, s);
    __print_message(s, ap);
    va_end (ap);

    SpinUnlock(&lock_uart);
}

/*
 * Convert a logical cpu id in the current VM to the corresponding
 * virtual cpu id the hypervisor manages.
 */
static VcpuIdType Lcpu2Vcpu(unsigned int lcpu)
{
    VmIdType vm = VcpuControl[CurrentVcpu].vmid;
    return (VcpuIdType)(lcpu + VmControl[vm].boot_vcpu);
}

static struct sbiret SbiRaiseIPI(unsigned long hartmask, unsigned long firsthart)
{
    struct sbiret ret = {SBI_SUCCESS, 0};
    unsigned int lcpu;
    for (lcpu = 0U; lcpu < 32U; lcpu++) {
        VcpuIdType vcpu = Lcpu2Vcpu(lcpu);

        if (hartmask & (1U << lcpu)) {
            VcpuControl[vcpu].ipi_request = TRUE;
            MemBarrier();
            if (vcpu == CurrentVcpu) {
                raiseVSI(IP_VSSI);
            } else {
                SpinLock(&lock_sched);
                if (VcpuControl[vcpu].coreid >= CORE_UNASSIGNED) {
                    context* p = (context *)VcpuControl[vcpu].sp;
                    p->hvip |= IP_VSSI;
                } else {
                    sbi_send_ipi(1U<<VcpuControl[vcpu].coreid, 0U);
                }
                SpinUnlock(&lock_sched);
            }
        }
    }
    return ret;
}

static void deactivateTimer(struct VcpuControl *vcpu)
{
    vcpu->next_time = _get_time() + 10000000U;
}

static void ActivateVSTimer(void)
{
    deactivateTimer(&VcpuControl[CurrentVcpu]);
    raiseVSI(IP_VSTI);
}

void VcpuSwitch(struct VcpuControl *current, struct VcpuControl *next)
{
    context* p = (context *)next->sp;

    if ((long)(next->next_time - _get_time()) <= 0) {
        p->hvip |= IP_VSTI;
        deactivateTimer(next);
    }
    if (VcpuControl[CurrentVcpu].ipi_request) {
        VcpuControl[CurrentVcpu].ipi_request = FALSE;
        MemBarrier();
        p->hvip |= IP_VSSI;
    }
    /* switch guest's physical space */
    SetHGATP(VmControl[next->vmid].vm_hgatp);
    switch_context(&next->sp, &current->sp);
}

static VcpuIdType ChooseNextVcpu(void)
{
    /* roundrobin scheduling */
    VcpuIdType vcpu = CurrentVcpu;

    VcpuControl[vcpu].coreid = CORE_UNASSIGNED;

    do {
        vcpu = (vcpu + 1) % NUMBER_OF_VCPUS;
    } while ((VcpuControl[vcpu].state != STARTED || VcpuControl[vcpu].coreid != CORE_UNASSIGNED) && vcpu != CurrentVcpu);
    VcpuControl[vcpu].coreid = ThisCore;

    return vcpu;
}

void _Schedule(void)
{
    SpinLock(&lock_sched);
    VcpuIdType from = CurrentVcpu;
    CurrentVcpu = ChooseNextVcpu();
    if (from != CurrentVcpu) {
        VcpuSwitch(&VcpuControl[from], &VcpuControl[CurrentVcpu]);
    }
    SpinUnlock(&lock_sched);
}

static void VcpuEntry(void)
{
    SpinUnlock(&lock_sched);
    VcpuStart(VcpuControl[CurrentVcpu].entry, VcpuControl[CurrentVcpu].param);
}

static void InitVcpu(VcpuIdType vcpu)
{
    context* p = (context *)&VcpuControl[vcpu].stack[STACKSIZE] - 1;
    p->ra = (unsigned long)VcpuEntry;
    VcpuControl[vcpu].sp = (unsigned long)p;
    p->vsstatus = 0U;
    p->vsie = 0u;
    p->vsatp = 0u;
    p->hvip = 0u;
    p->hstatus = HSTATUS_SPV|HSTATUS_SPVP;
}

static void InitVm(VmIdType vm)
{
    VcpuIdType vcpu = VmControl[vm].boot_vcpu;
    VcpuControl[vcpu].entry = VM_ENTRY_ADDR;
    if (VmControl[vm].vm_io_size) {
        CreateVmIoSpace(vm, VmControl[vm].vm_io_start, VmControl[vm].vm_io_size);
    }
    VmControl[vm].vm_hgatp = CreateVmSpace(vm, VmControl[vm].vm_start, VmControl[vm].vm_size, (unsigned long)VM_ENTRY_ADDR);
}

static __thread unsigned long nexttime;

#define INTERVAL 1000000U

static int Timer(void)
{
    VcpuIdType vcpu;

    /* check if the current vcpu needs an interrupt */
    if ((long)(VcpuControl[CurrentVcpu].next_time - _get_time()) <= 0) {
        ActivateVSTimer();
    }

    /* set the next hypervisor timer */
    do {
        nexttime += INTERVAL;
        sbi_set_timer(nexttime);
    } while ((long)(_get_time() - nexttime) >= 0);

    return TRUE;     /* vcpu reschedule */
}

static int InterCoreInt(void)
{
    clear_IPI();

    if (VcpuControl[CurrentVcpu].ipi_request) {
        VcpuControl[CurrentVcpu].ipi_request = FALSE;
        MemBarrier();
        raiseVSI(IP_VSSI);
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
        _print_message("Unknown Interrupt: cause(0x%x)\n", cause);
        break;
    }
    return FALSE;
}

static struct sbiret SbiSetTimer(unsigned long tim)
{
    struct sbiret ret = {SBI_SUCCESS, 0};
    /* activate the next timer on this vcpu */
    VcpuControl[CurrentVcpu].next_time = tim;
    clearVSI(IP_VSTI);
    return ret;
}

static struct sbiret SbiVcpuStart(unsigned long lcpu, unsigned long start_addr, unsigned long opaque)
{
    struct sbiret ret = {SBI_SUCCESS, 0};
    VcpuIdType vcpu = Lcpu2Vcpu(lcpu);

    if (VcpuControl[vcpu].state == STOPPED) {
        VcpuControl[vcpu].entry = (void (*)(unsigned long))start_addr;
        VcpuControl[vcpu].param = opaque;
        MemBarrier();
        VcpuControl[vcpu].state = STARTED;
    } else {
        ret.error = SBI_ERR_ALREADY_STARTED;
    }
    return ret;
}

static struct sbiret SbiVcpuStatus(unsigned long lcpu)
{
    VcpuIdType vcpu = Lcpu2Vcpu(lcpu);
    struct sbiret ret = {SBI_SUCCESS, VcpuControl[vcpu].state};
    return ret;
}

static struct sbiret SbiNotSupported(void)
{
    struct sbiret ret = {SBI_ERR_NOT_SUPPORTED, 0};
    return ret;
}

#define GenerateEID(k1, k2, k3, k4) (k1<<24U | k2<<16U | k3<<8U | k4<<0U)
/* SBI version 0.2 */
#define EID_TIME                    GenerateEID('T', 'I', 'M', 'E')
#define EID_sPI                     GenerateEID(0U, 's', 'P', 'I')
#define EID_HSM                     GenerateEID(0U, 'H', 'S', 'M')
/* SBI version 2.0 */
#define EID_DBCN                    GenerateEID('D', 'B', 'C', 'N')

#define FID_TIME_SETTIMER            0
#define FID_IPI_SENDIPI              0
#define FID_HART_START               0
#define FID_HART_GETSTATUS           2
#define FID_DBCN_WRITE               0
#define FID_DBCN_READ                1
#define FID_DBCN_WRITE_BYTE          2

struct sbiret HypHandler(unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3, unsigned long a4, unsigned long a5, unsigned long fid, unsigned long eid)
{
    switch (eid) {
    case EID_TIME:
        switch (fid) {
        case FID_TIME_SETTIMER:
            return SbiSetTimer(a0);
        default:
            break;
        }
        break;
    case EID_sPI:
        switch (fid) {
        case FID_IPI_SENDIPI:
            return SbiRaiseIPI(a0, a1);
        default:
            break;
        }
        break;
    case EID_HSM:
        switch (fid) {
        case FID_HART_START:
            return SbiVcpuStart(a0, a1, a2);
        case FID_HART_GETSTATUS:
            return SbiVcpuStatus(a0);
        default:
            break;
        }
        break;
    case EID_DBCN:
        switch (fid) {
        case FID_DBCN_WRITE_BYTE:
            return sbi_debug_console_write_byte(a0);
        default:
            break;
        }
        break;
    default:
        break;
    }

    return SbiNotSupported();
}

#define EXCCTX_EPC        32
#define EXCCTX_CAUSE      33
#define EXCCTX_STATUS     34
#define EXCCTX_TVAL       35

#define EXC_ILLEGAL_INST  0x2U
#define EXC_VIRTUAL_INST  0x16U
#define OP_SYSTEM         0x73U
#define CSR_TIME          0xC01U
#define FUNC_CSSRS        0x2U

#define EXCCTX_EPC        32
#define EXCCTX_CAUSE      33
#define EXCCTX_STATUS     34
#define EXCCTX_TVAL       35
#define EXCCTX_SP         2

static _Bool emulate_inst(unsigned long* ctx, unsigned int inst)
{
    unsigned int opcode = inst & 0x7FU;

    switch (opcode) {
    case OP_SYSTEM:
    {
        unsigned int funct3 = inst >> 12U & 0x7U;
        unsigned int csr  = inst >> 20U;

        if (funct3 == FUNC_CSSRS && csr == CSR_TIME) {
            /* This emmulation is requred on OpenSBI */
            unsigned int rd   = inst >> 7 & 0x1F;
            /* emulate rdtime instruction */
            ctx[rd] = _get_time();
            ctx[EXCCTX_EPC] += 4U;
            return TRUE;
        }
        break;
    }
    default:
        break;
    }
    return FALSE;
}

static _Bool redirect_trap(unsigned long* ctx, unsigned long sepc, unsigned long scause, unsigned long sstatus, unsigned long stval)
{
    SetVSmodeRegs(sepc, scause, stval);

    ctx[EXCCTX_STATUS] |= STATUS_SPP;   /* return to VS-mode */
    ctx[EXCCTX_EPC] = GetVSTvec() & ~TVEC_VECTORED_MODE;
    ctx[EXCCTX_STATUS] &= ~(STATUS_SIE|STATUS_SPIE); /* disable VS-mode interrupts */
    if (sstatus & STATUS_SIE) {
        ctx[EXCCTX_STATUS] |= STATUS_SPIE;
    }

    return TRUE;
}

void ExcHandler(unsigned long* ctx, unsigned long sepc, unsigned long scause, unsigned long sstatus, unsigned long stval, unsigned long hstatus)
{
    if (hstatus & HSTATUS_SPV) {  /* trapped in a virtualized mode */
        if (scause == EXC_VIRTUAL_INST) {
            if (emulate_inst(ctx, stval)) {
                return;
            }
            scause = EXC_ILLEGAL_INST;
        }

        if (redirect_trap(ctx, sepc, scause, sstatus, stval)) {
            return;
        }
    }

    _print_message("Exception: sepc(0x%x) scause(0x%x) sstatus(0x%x) stval(0x%x), hstatus(0x%x) sp(0x%x) ra(0x%x) s0(0x%x) s1(0x%x)\n", sepc, scause, sstatus, stval, hstatus, ctx[2], ctx[1], ctx[8], ctx[9]);
    _print_message("           a0(0x%x) a1(0x%x) a2(0x%x) a3(0x%x) a4(0x%x) a5(0x%x)\n", ctx[10], ctx[11], ctx[12], ctx[13], ctx[14], ctx[15]);
    while (1);
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
    extern unsigned char _bss_start[];
    extern unsigned char _bss_end[];

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

    for (coreid = CORE0; coreid < min(NCORE, NUMBER_OF_VCPUS); coreid++) {
        if (coreid != BootCore) {
            /* start phisical cores */
            struct sbiret ret = sbi_hart_start(coreid, (unsigned long)_secondary_start, coreid);
            if (ret.error == SBI_ERR_INVALID_PARAM) {
                /* The specified core doesn't exit */
            } else if (ret.error) {
                _print_message("sbi_hart_start(core%x) failed. error(0x%x)\n", coreid, ret.error);
            }
        }
    }
}

static void restart_bootcore(void)
{
    struct sbiret ret;
    CoreIdType coreid;

    for (coreid = CORE0; coreid < NCORE; coreid++) {
        struct sbiret ret = sbi_hart_get_status(coreid);
        if (ret.error == SBI_SUCCESS && ret.value == STARTED) {
            BootCore = coreid;
            _secondary_start(BootCore);
        }
    }
}

void main(CoreIdType coreid)
{
    VcpuIdType vcpu;

    SetTrapVectors((unsigned long)trap_vectors + TVEC_VECTORED_MODE);

    if (BootCore == CORE_UNASSIGNED) {
        restart_bootcore();
    }

    setupTLS();
    ThisCore = coreid;
    SetupDeleg();

    if (ThisCore == BootCore) {
        VmIdType vm;

        clearbss();

        for (vcpu = 0; vcpu < NUMBER_OF_VCPUS; vcpu++) {
            InitVcpu(vcpu);
        }
        for (vm = 0; vm < NUMBER_OF_VMS; vm++) {
            InitVm(vm);
        }

        start_cores();
    }
    sync_cores();

    StartTimer();
    EnableInterrupts();

    _print_message("Core%x started.\n", ThisCore);

    /* currently only supports two VMs */
    if (ThisCore == BootCore) {
        CurrentVcpu = VmControl[VM0].boot_vcpu;
    } else {
        CurrentVcpu = VmControl[VM1].boot_vcpu;
    }

    SpinLock(&lock_sched);
    VcpuControl[CurrentVcpu].state = STARTED;
    VcpuControl[CurrentVcpu].coreid = ThisCore;
    SetHGATP(VmControl[VcpuControl[CurrentVcpu].vmid].vm_hgatp);
    load_context(&VcpuControl[CurrentVcpu].sp);
}

