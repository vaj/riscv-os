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

    .equ   INT_SMODE_SOFT,   1
    .equ   INT_VSMODE_SOFT,  2
    .equ   INT_SMODE_TIMER,  5
    .equ   INT_VSMODE_TIMER, 6

    .equ   IP_SSI,  (1U<<INT_SMODE_SOFT)
    .equ   IP_STI,  (1U<<INT_SMODE_TIMER)
    .equ   IE_SSIE, IP_SSI
    .equ   IE_STIE, IP_STI

    .text
    .globl trap_vectors
    .type trap_vectors,@function
    .balign 256
trap_vectors:
    j   exc_handler
    .balign 4
    j   int_handler          /* s-mode software interrupt */
    .balign 4
    j   undefined_handler    /* virtual s-mode software interrupt */
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   int_handler          /* s-mode timer interrupt */
    .balign 4
    j   undefined_handler    /* virtual s-mode timer interrupt */
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler    /* s-mode external interrupt */
    .balign 4
    j   undefined_handler    /* virtual s-mode external interrupt */
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .size trap_vectors,.-trap_vectors

    .global ExcHandler
    .balign 4

undefined_handler:
    csrrw tp, sscratch, tp
    sd    zero, 0*8(tp)
    sd    ra, 1*8(tp)
    sd    sp, 2*8(tp)
    sd    gp, 3*8(tp)
    sd    t0, 5*8(tp)
    sd    t1, 6*8(tp)
    sd    t2, 7*8(tp)
    sd    s0, 8*8(tp)
    sd    s1, 9*8(tp)
    sd    a0, 10*8(tp)
    sd    a1, 11*8(tp)
    sd    a2, 12*8(tp)
    sd    a3, 13*8(tp)
    sd    a4, 14*8(tp)
    sd    a5, 15*8(tp)
    sd    a6, 16*8(tp)
    sd    a7, 17*8(tp)
    sd    s2, 18*8(tp)
    sd    s3, 19*8(tp)
    sd    s4, 20*8(tp)
    sd    s5, 21*8(tp)
    sd    s6, 22*8(tp)
    sd    s7, 23*8(tp)
    sd    s8, 24*8(tp)
    sd    s9, 25*8(tp)
    sd    s10, 26*8(tp)
    sd    s11, 27*8(tp)
    sd    t3, 28*8(tp)
    sd    t4, 29*8(tp)
    sd    t5, 30*8(tp)
    sd    t6, 31*8(tp)

    csrr  a1, sepc
    csrr  a2, scause
    csrr  a3, sstatus
    csrr  a4, stval
    csrr  a5, hstatus
    sd    a1, 32*8(tp)
    sd    a2, 33*8(tp)
    sd    a3, 34*8(tp)
    sd    a4, 35*8(tp)

    mv    a0, tp
    mv    s0, tp
    csrrw tp, sscratch, tp
    sd    tp, 4*8(a0)

    la    t0, _CLV_SIZE
    sub   tp, a0, t0   /* restore tp */
    la    t0, _ESTACK_SIZE
    add   sp, tp, t0   /* switch to the exception stack */
/*    add   sp, a0, t0   /* switch to the exception stack */

    jal   ExcHandler

    ld    a1, 32*8(s0)
    ld    a2, 33*8(s0)
    ld    a3, 34*8(s0)
    ld    a4, 35*8(s0)
    csrw  sepc,    a1
    csrw  scause,  a2
    csrw  sstatus, a3
    csrw  stval,   a4

    ld    ra, 1*8(s0)
    ld    sp, 2*8(s0)
    ld    gp, 3*8(s0)
    ld    tp, 4*8(s0)
    ld    t0, 5*8(s0)
    ld    t1, 6*8(s0)
    ld    t2, 7*8(s0)
    ld    s1, 9*8(s0)
    ld    a0, 10*8(s0)
    ld    a1, 11*8(s0)
    ld    a2, 12*8(s0)
    ld    a3, 13*8(s0)
    ld    a4, 14*8(s0)
    ld    a5, 15*8(s0)
    ld    a6, 16*8(s0)
    ld    a7, 17*8(s0)
    ld    s2, 18*8(s0)
    ld    s3, 19*8(s0)
    ld    s4, 20*8(s0)
    ld    s5, 21*8(s0)
    ld    s6, 22*8(s0)
    ld    s7, 23*8(s0)
    ld    s8, 24*8(s0)
    ld    s9, 25*8(s0)
    ld    s10, 26*8(s0)
    ld    s11, 27*8(s0)
    ld    t3, 28*8(s0)
    ld    t4, 29*8(s0)
    ld    t5, 30*8(s0)
    ld    t6, 31*8(s0)
    ld    s0, 8*8(s0)

    sret

    .equ   EXC_INST_MISALIGNED,    0
    .equ   EXC_INST_ACCESS,        1
    .equ   EXC_INST_ILLEGAL,       2
    .equ   EXC_BREAKPOINT,         3
    .equ   EXC_LOAD_MISALIGNED,    4
    .equ   EXC_LOAD_ACCESS,        5
    .equ   EXC_STORE_MISALIGNED,   6
    .equ   EXC_STORE_ACCESS,       7
    .equ   EXC_UMODE_ECALL,        8
    .equ   EXC_SMODE_ECALL,        9
    .equ   EXC_HSMODE_ECALL, EXC_SMODE_ECALL
    .equ   EXC_VSMODE_ECALL,       10
    .equ   EXC_ECALL, EXC_VSMODE_ECALL
    .equ   EXC_INST_PAGE_FAULT,    12
    .equ   EXC_LOAD_PAGE_FAULT,    13
    .equ   EXC_GUEST_PAGE_FAULT,   14
    .equ   EXC_STORE_PAGE_FAULT,   15
    .equ   EXC_INSTRUCTION_GUEST_PAGE_FAULT,  20
    .equ   EXC_LOAD_GUEST_PAGE_FAULT,         21
    .equ   EXC_VIRTUAL_INSTRUCTION,           22
    .equ   EXC_STORE_GUEST_PAGE_FAULT,        23

    .equ   SIZEOF_VcpuControl,     0x4030

    .balign 4
exc_handler:
    csrrw tp, sscratch, tp
    sd    t0, 0*8(tp)

    csrr  t0, scause
    add   t0, t0, -EXC_ECALL
    bne   t0, zero, 1f

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, sscratch, tp
    mv    t4, tp       /* guest's tp */
    mv    t5, gp       /* guest's gp */
    mv    tp, t0       /* restore tp */

    .option norelax
    la    gp, __global_pointer$
    .option relax

    /* switch to the vcpu stack */
    /* sp = &VcpuControl[CurrentVcpu].vcpu_stack[STACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentVcpu)(tp)
    la    t1, VcpuControl
    addi  t0, t0, 1    /* t0 = CurrentVcpu + 1 */
    li    t2, SIZEOF_VcpuControl   /* t2 = sizeof(struct VcpuControl) */
    mul   t2, t2, t0   /* t2 = sizeof(struct VcpuControl)*(CurrentVcpu + 1) */
    mv    t3, sp
    add   sp, t2, t1   /* &VcpuControl[CurrentVcpu].vcpu_stack[STACKSIZE] */

    addi  sp, sp, -8*6
    csrr  t0, sstatus
    csrr  t1, sepc
    addi  t1, t1, 4
    sd    t0, 0*8(sp)   /* status */
    sd    t1, 1*8(sp)   /* epc */
    sd    ra, 2*8(sp)
    sd    t4, 3*8(sp)   /* guest's tp */
    sd    t5, 4*8(sp)   /* guest's gp */
    sd    t3, 5*8(sp)   /* previous sp */

    jal   HypHandler

    ld    t0, 0*8(sp)   /* status */
    ld    t1, 1*8(sp)   /* epc */
    csrw  sstatus, t0
    csrw  sepc,    t1

    ld    ra, 2*8(sp)
    ld    tp, 3*8(sp)
    ld    gp, 4*8(sp)
    ld    sp, 5*8(sp)   /* switch back to the guest stack */
    sret

1:
    ld    t0, 0*8(tp)
    csrrw tp, sscratch, tp
    j     undefined_handler
    .size exc_handler,.-exc_handler

    .balign 4
int_handler:
    csrrw tp, sscratch, tp
    /* save t0-t5 temprarily on the scratch space */
    sd    t0, 5*8(tp)
    sd    t1, 6*8(tp)
    sd    t2, 7*8(tp)
    sd    t3, 8*8(tp)
    sd    t4, 9*8(tp)
    sd    t5, 10*8(tp)
    mv    t3, tp

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, sscratch, tp
    mv    t4, tp       /* guest's tp */
    mv    t5, gp       /* guest's gp */
    mv    tp, t0       /* guest's tp */

    .option norelax
    la    gp, __global_pointer$
    .option relax

    /* switch to the vcpu stack */
    /* sp = &VcpuControl[CurrentVcpu].stack[STACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentVcpu)(tp)
    la    t1, VcpuControl
    addi  t0, t0, 1
    li    t2, SIZEOF_VcpuControl   /* sizeof(VcpuControl) */
    mul   t2, t2, t0
    mv    t0, sp
    add   sp, t2, t1   /* &VcpuControl[CurrentVcpu].stack[STACKSIZE] */

    addi  sp, sp, -8*22
    sd    t4, 17*8(sp)  /* save the guest's tp */
    sd    t5, 18*8(sp)  /* save the guest's gp */
    sd    t0, 19*8(sp)  /* save the guest's sp */

    /* move the context from the scratch space to the vcpu stack */
    ld    t0, 5*8(t3)   /* the value of t0 */
    sd    t0, 9*8(sp)
    ld    t0, 6*8(t3)   /* the value of t1 */
    sd    t0, 10*8(sp)
    ld    t0, 7*8(t3)   /* the value of t2 */
    sd    t0, 11*8(sp)
    ld    t0, 8*8(t3)   /* the value of t3 */
    sd    t0, 12*8(sp)
    ld    t0, 9*8(t3)   /* the value of t4 */
    sd    t0, 13*8(sp)
    ld    t0, 10*8(t3)   /* the value of t5 */
    sd    t0, 14*8(sp)

    sd    ra, 0*8(sp)
    sd    a0, 1*8(sp)
    sd    a1, 2*8(sp)
    sd    a2, 3*8(sp)
    sd    a3, 4*8(sp)
    sd    a4, 5*8(sp)
    sd    a5, 6*8(sp)
    sd    a6, 7*8(sp)
    sd    a7, 8*8(sp)
    sd    t6, 15*8(sp)
    sd    s0, 16*8(sp)

    csrr  a0, scause
    csrr  a1, sstatus
    csrr  a2, sepc
    sd    a1, 20*8(sp)   /* status */
    sd    a2, 21*8(sp)   /* epc */

    mv    s0, sp
    /* switch to the interrupt stack */
    la    t0, _STACK_SIZE
    add   sp, t3, t0

    jal   InterruptHandler

    /* switch back to the vcpu stack */
    mv    sp, s0
    beqz  a0, 1f

    jal   _Schedule

1:
    ld    a1, 20*8(sp)   /* status */
    ld    a2, 21*8(sp)   /* epc */
    csrw  sstatus, a1
    csrw  sepc,    a2
    ld    ra, 0*8(sp)
    ld    a0, 1*8(sp)
    ld    a1, 2*8(sp)
    ld    a2, 3*8(sp)
    ld    a3, 4*8(sp)
    ld    a4, 5*8(sp)
    ld    a5, 6*8(sp)
    ld    a6, 7*8(sp)
    ld    a7, 8*8(sp)
    ld    t0, 9*8(sp)
    ld    t1, 10*8(sp)
    ld    t2, 11*8(sp)
    ld    t3, 12*8(sp)
    ld    t4, 13*8(sp)
    ld    t5, 14*8(sp)
    ld    t6, 15*8(sp)
    ld    s0, 16*8(sp)
    ld    tp, 17*8(sp)  /* load the guest's tp */
    ld    gp, 18*8(sp)  /* load the guest's gp */

    /* switch back to the guest stack */
    ld    sp, 19*8(sp)
    sret
    .size int_handler,.-int_handler

    .global clear_IPI
    .type clear_IPI,@function
clear_IPI:
    csrci  sip, IP_SSI
    ret
    .size clear_IPI,.-clear_IPI

    .global EnableInterrupts
    .type EnableInterrupts,@function
    .balign 4
EnableInterrupts:
    li    t0, IE_STIE | IE_SSIE
    csrs  sie, t0
    ret
    .size EnableInterrupts,.-EnableInterrupts

    .global SetTrapVectors
    .type SetTrapVectors,@function
SetTrapVectors:
    csrw  stvec, a0
    ret
    .size SetTrapVectors,.-SetTrapVectors

    .global TestAndSet
    .type TestAndSet,@function
    .balign 4
TestAndSet:
    mv    a2, a0
    lr.w  a0, (a2)
    bne   a0, zero, 1f
    sc.w  a0, a1, (a2)
1:
    ret
    .size TestAndSet,.-TestAndSet

    .globl switch_context
    .globl load_context
    .type switch_context,@function
    .balign 4
switch_context:
    addi  sp, sp, -8*23
    sd    s0, 0*8(sp)
    sd    s1, 1*8(sp)
    sd    s2, 2*8(sp)
    sd    s3, 3*8(sp)
    sd    s4, 4*8(sp)
    sd    s5, 5*8(sp)
    sd    s6, 6*8(sp)
    sd    s7, 7*8(sp)
    sd    s8, 8*8(sp)
    sd    s9, 9*8(sp)
    sd    s10, 10*8(sp)
    sd    s11, 11*8(sp)
    sd    ra, 12*8(sp)

    csrr  t0, vsstatus
    sd    t0, 13*8(sp)
    csrr  t0, vsepc
    sd    t0, 14*8(sp)
    csrr  t0, vscause
    sd    t0, 15*8(sp)
    csrr  t0, vstval
    sd    t0, 16*8(sp)
    csrr  t0, vsie
    sd    t0, 17*8(sp)
    csrr  t0, vstvec
    sd    t0, 18*8(sp)
    csrr  t0, vsscratch
    sd    t0, 19*8(sp)
    csrr  t0, vsatp
    sd    t0, 20*8(sp)
    csrr  t0, hvip
    sd    t0, 21*8(sp)
    csrr  t0, hstatus
    sd    t0, 22*8(sp)

    sd    sp, (a1)

load_context:
    ld    sp, (a0)
    ld    s0, 0*8(sp)
    ld    s1, 1*8(sp)
    ld    s2, 2*8(sp)
    ld    s3, 3*8(sp)
    ld    s4, 4*8(sp)
    ld    s5, 5*8(sp)
    ld    s6, 6*8(sp)
    ld    s7, 7*8(sp)
    ld    s8, 8*8(sp)
    ld    s9, 9*8(sp)
    ld    s10, 10*8(sp)
    ld    s11, 11*8(sp)
    ld    ra, 12*8(sp)

    ld    t0, 13*8(sp)
    csrw  vsstatus, t0
    ld    t0, 14*8(sp)
    csrw  vsepc, t0
    ld    t0, 15*8(sp)
    csrw  vscause, t0
    ld    t0, 16*8(sp)
    csrw  vstval, t0
    ld    t0, 17*8(sp)
    csrw  vsie, t0
    ld    t0, 18*8(sp)
    csrw  vstvec, t0
    ld    t0, 19*8(sp)
    csrw  vsscratch, t0
    ld    t0, 20*8(sp)
    csrw  vsatp, t0
    ld    t0, 21*8(sp)
    csrw  hvip, t0
    ld    t0, 22*8(sp)
    csrw  hstatus, t0

    addi  sp, sp, 8*23
    ret
    .size switch_context,.-switch_context

    .globl SetupDeleg
    .type SetupDeleg,@function
    .balign 4
SetupDeleg:
    li    t0, (1U << EXC_INST_MISALIGNED) | (1U << EXC_INST_ACCESS) | (1U << EXC_BREAKPOINT) | (1U << EXC_LOAD_MISALIGNED) | (1U << EXC_LOAD_ACCESS) | (1U << EXC_STORE_MISALIGNED) | (1U << EXC_STORE_ACCESS) | (1U << EXC_UMODE_ECALL) | (1U << EXC_INST_PAGE_FAULT) | (1U << EXC_LOAD_PAGE_FAULT) | (1U << EXC_STORE_PAGE_FAULT) | (1U << EXC_INST_ILLEGAL)
    csrs  hedeleg, t0

    li    t0, (1U << INT_VSMODE_SOFT) | (1U << INT_VSMODE_TIMER)
    csrs  hideleg, t0
    ret
    .size SetupDeleg,.-SetupDeleg

    .equ   STATUS_SPIE,      (1U<<5)
    .equ   STATUS_SIE,       (1U<<1)
    .equ   STATUS_SPP_USER,  (0<<8)         /* user mode */
    .equ   STATUS_SPP_SMODE, (1U<<8)        /* supervisor mode */
    .equ   STATUS_SUM,       (1U<<18)
    .equ   HSTATUS_SPV,      (1U<<8)        /* virtualized mode */
    .equ   HSTATUS_SPVP,     (1U<<7)

    .globl VcpuStart
    .type VcpuStart,@function
    .balign 4
VcpuStart:  /* turns the mode into VS-mode */
    csrw  sepc, a0
    li    a0, STATUS_SPP_SMODE
    csrw  sstatus, a0
    mv    a0, a1
    sret
    .size VcpuStart,.-VcpuStart

    .globl MemBarrier
    .type MemBarrier,@function
    .balign 4
MemBarrier:
    fence w, w
    ret
    .size MemBarrier,.-MemBarrier

    .globl Pause
    .type Pause,@function
    .balign 4
Pause:
    fence.i
    ret
    .size Pause,.-Pause

    .globl _get_time
    .type _get_time,@function
    .balign 4
_get_time:
    rdtime a0
    ret
    .size _get_time,.-_get_time

    .globl SetSATP
    .type SetSATP,@function
    .balign 4
SetSATP:
    li    t0, STATUS_SUM
    csrs  sstatus, t0
    nop
    csrw  satp, a0
    ret
    .size SetSATP,.-SetSATP

    .globl SetHGATP
    .type SetHGATP,@function
    .balign 4
SetHGATP:
    csrw  hgatp, a0
    ret
    .size SetHGATP,.-SetHGATP

    .globl raiseVSI
    .type raiseVSI,@function
    .balign 4
raiseVSI:
    csrs  hvip, a0
    ret
    .size raiseVSI,.-raiseVSI

    .globl clearVSI
    .type clearVSI,@function
    .balign 4
clearVSI:
    csrc  hvip, a0
    ret
    .size clearVSI,.-clearVSI

    .global GetVSTvec
    .type GetVSTvec,@function
    .balign 4
GetVSTvec:
    csrr  a0, vstvec
    ret
    .size GetVSTvec,.-GetVSTvec

    .global SetVSmodeRegs
    .type SetVSmodeRegs,@function
    .balign 4
SetVSmodeRegs:
    csrw  vsepc,    a0
    csrw  vscause,  a1
    csrw  vstval,   a2

    csrr  t0, hstatus
    li    t1, HSTATUS_SPV|HSTATUS_SPVP
    or    t0, t0, t1
    csrw  hstatus, t0

    li    t1, STATUS_SIE
    csrc  vsstatus, t1

    ret
    .size SetVSmodeRegs,.-SetVSmodeRegs

