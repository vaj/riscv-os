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

    .text
    .globl trap_vectors
    .type trap_vectors,@function
    .balign 256
trap_vectors:
    j   exc_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   ipi_handler          /* software interrupt */
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   timer_handler        /* timer interrupt */
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler    /* external interrupt */
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
    csrrw tp, mscratch, tp
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

    csrr  a1, mepc
    csrr  a2, mcause
    csrr  a3, mstatus
    csrr  a4, mtval
    sd    a1, 32*8(tp)
    sd    a2, 33*8(tp)
    sd    a3, 34*8(tp)
    sd    a4, 35*8(tp)

    mv    a0, tp
    csrrw tp, mscratch, tp
    sd    tp, 4*8(a0)

    la    t0, _CLV_SIZE
    sub   tp, a0, t0   /* restore tp, which might have been broken */
    la    t0, _ESTACK_SIZE
    add   sp, tp, t0   /* switch to the exception stack */

    jal   ExcHandler
1:
    j   1b
    mret

    .equ   EXC_ECALL, 8

    .balign 4
exc_handler:
    csrrw tp, mscratch, tp
    sd    t0, 5*8(tp)

    csrr  t0, mcause
    add   t0, t0, -EXC_ECALL
    bne   t0, zero, 1f

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, mscratch, tp
    mv    tp, t0       /* restore tp, which might have been broken */

    /* switch to the kernel stack */
    /* sp = &TaskControl[CurrentTask].task_kstack[KSTACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentTask)(tp)
    la    t1, TaskControl
    addi  t0, t0, 1    /* t0 = CurrentTask + 1 */
    li    t2, 0x4030   /* t2 = sizeof(struct TaskControl) */
    mul   t2, t2, t0   /* t2 = sizeof(struct TaskControl)*(CurrentTask + 1) */
    mv    t3, sp
    add   sp, t2, t1   /* &TaskControl[CurrentTask].task_kstack[KSTACKSIZE] */

    addi  sp, sp, -8*4
    sd    s0, 1*8(sp)
    sd    s1, 2*8(sp)
    sd    ra, 0*8(sp)
    sd    t3, 3*8(sp)   /* previous sp */

    csrr  s0, mepc
    addi  s0, s0, 4
    csrr  s1, mstatus
    csrw  mstatus, zero

    jal   SvcHandler

    csrw  mepc, s0
    csrw  mstatus, s1

    ld    ra, 0*8(sp)
    ld    s0, 1*8(sp)
    ld    s1, 2*8(sp)
    ld    sp, 3*8(sp)   /* switch back to the user stack */

    mret

1:
    ld    t0, 5*8(tp)
    csrrw tp, mscratch, tp
    j     undefined_handler
    .size exc_handler,.-exc_handler


    .balign 4
ipi_handler:
    csrrw tp, mscratch, tp
    sd    t0, 5*8(tp)
    sd    t1, 6*8(tp)
    sd    t2, 7*8(tp)
    sd    t3, 8*8(tp)
    mv    t3, tp

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, mscratch, tp
    mv    tp, t0       /* restore tp, which might have been broken */

    /* switch to the kernel stack */
    /* sp = &TaskControl[CurrentTask].task_kstack[KSTACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentTask)(tp)
    la    t1, TaskControl
    addi  t0, t0, 1
    li    t2, 0x4030   /* sizeof(TaskControl) */
    mul   t2, t2, t0
    mv    t0, sp
    add   sp, t2, t1   /* &TaskControl[CurrentTask].task_kstack[KSTACKSIZE] */

    addi  sp, sp, -8*19
    sd    t0, 18*8(sp)  /* save the previous sp */

    ld    t0, 5*8(t3)
    ld    t1, 6*8(t3)
    ld    t2, 7*8(t3)
    sd    t0, 9*8(sp)
    mv    t0, t3    /* scratch space */
    ld    t3, 8*8(t3)

    sd    ra, 0*8(sp)
    sd    a0, 1*8(sp)
    sd    a1, 2*8(sp)
    sd    a2, 3*8(sp)
    sd    a3, 4*8(sp)
    sd    a4, 5*8(sp)
    sd    a5, 6*8(sp)
    sd    a6, 7*8(sp)
    sd    a7, 8*8(sp)
    sd    t1, 10*8(sp)
    sd    t2, 11*8(sp)
    sd    t3, 12*8(sp)
    sd    t4, 13*8(sp)
    sd    t5, 14*8(sp)
    sd    t6, 15*8(sp)
    sd    s0, 16*8(sp)

    mv    s0, sp
    la    t1, _STACK_SIZE
    add   sp, t0, t1
    jal   InterCoreInt
    mv    sp, s0
    beqz  a0, 1f

    sd    s1, 17*8(sp)
    csrr  s0, mepc
    csrr  s1, mstatus
    csrw  mstatus, zero
    jal   _Schedule
    csrw  mepc, s0
    csrw  mstatus, s1
    ld    s1, 17*8(sp)
1:
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
    /* switch back to the user stack */
    ld    sp, 18*8(sp)
    mret
    .size ipi_handler,.-ipi_handler


    .balign 4
timer_handler:
    csrrw tp, mscratch, tp
    sd    t0, 5*8(tp)
    sd    t1, 6*8(tp)
    sd    t2, 7*8(tp)
    sd    t3, 8*8(tp)
    mv    t3, tp

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, mscratch, tp
    mv    tp, t0       /* restore tp, which might have been broken */

    /* switch to the kernel stack */
    /* sp = &TaskControl[CurrentTask].task_kstack[KSTACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentTask)(tp)
    la    t1, TaskControl
    addi  t0, t0, 1
    li    t2, 0x4030   /* sizeof(TaskControl) */
    mul   t2, t2, t0
    mv    t0, sp
    add   sp, t2, t1   /* &TaskControl[CurrentTask].task_kstack[KSTACKSIZE] */

    addi  sp, sp, -8*19
    sd    t0, 18*8(sp)  /* save the previous sp */

    ld    t0, 5*8(t3)
    ld    t1, 6*8(t3)
    ld    t2, 7*8(t3)

    sd    t0, 9*8(sp)
    mv    t0, t3    /* scratch space */
    ld    t3, 8*8(t3)

    sd    ra, 0*8(sp)
    sd    a0, 1*8(sp)
    sd    a1, 2*8(sp)
    sd    a2, 3*8(sp)
    sd    a3, 4*8(sp)
    sd    a4, 5*8(sp)
    sd    a5, 6*8(sp)
    sd    a6, 7*8(sp)
    sd    a7, 8*8(sp)
    sd    t1, 10*8(sp)
    sd    t2, 11*8(sp)
    sd    t3, 12*8(sp)
    sd    t4, 13*8(sp)
    sd    t5, 14*8(sp)
    sd    t6, 15*8(sp)
    sd    s0, 16*8(sp)

    mv    s0, sp
    la    t1, _STACK_SIZE
    add   sp, t0, t1
    jal   Timer
    mv    sp, s0
    beqz  a0, 1f

    sd    s1, 17*8(sp)
    csrr  s0, mepc
    csrr  s1, mstatus
    csrw  mstatus, zero
    jal   _Schedule
    csrw  mepc, s0
    csrw  mstatus, s1
    ld    s1, 17*8(sp)
1:
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
    /* switch back to the user stack */
    ld    sp, 18*8(sp)
    mret
    .size timer_handler,.-timer_handler

    .equ   MIE_MTIE, 0x80
    .equ   MIE_MSIE, 0x8
    .equ   MSTATUS_MIE, 0x8
    .equ   MSTATUS_MPIE, 0x80
    .equ   MSTATUS_MPP_USER, 0x0           /* user mode */

    .global EnableTimer
    .type EnableTimer,@function
    .balign 4
EnableTimer:
    li    t0, MIE_MTIE
    csrs  mie, t0
    ret
    .size EnableTimer,.-EnableTimer

    .global EnableIPI
    .type EnableIPI,@function
    .balign 4
EnableIPI:
    csrsi  mie, MIE_MSIE
    ret
    .size EnableIPI,.-EnableIPI

    .global SetTrapVectors
    .type SetTrapVectors,@function
SetTrapVectors:
    csrw  mtvec, a0
    ret
    .size SetTrapVectors,.-SetTrapVectors

    .global GetHartID
    .type GetHartID,@function
    .balign 4
GetHartID:
    csrr  a0, mhartid
    ret
    .size GetHartID,.-GetHartID

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
    addi  sp, sp, -8*13
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
    addi  sp, sp, 8*13
    ret
    .size switch_context,.-switch_context

    .equ  PMP_R,  0x1
    .equ  PMP_W,  0x2
    .equ  PMP_X,  0x4
    .equ  PMP_NAPOT,  0x18

    .globl InitPMP
    .type InitPMP,@function
    .balign 4
InitPMP:
    li     t0, PMP_NAPOT | PMP_R | PMP_W | PMP_X
    csrw   pmpaddr0, a0
    csrw   pmpcfg0, t0
    ret
    .size InitPMP,.-InitPMP

    .globl TaskStart
    .type TaskStart,@function
    .balign 4
TaskStart:
    csrw  mepc, a0
    li    a0, MSTATUS_MPIE|MSTATUS_MPP_USER
    csrw  mstatus, a0
    mv    sp, a1
    mret
    .size TaskStart,.-TaskStart

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
