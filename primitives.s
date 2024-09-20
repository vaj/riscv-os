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
    .equ   INT_SMODE_TIMER,  5

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
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   undefined_handler
    .balign 4
    j   int_handler         /* s-mode timer interrupt */
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
    sd    a1, 32*8(tp)
    sd    a2, 33*8(tp)
    sd    a3, 34*8(tp)
    sd    a4, 35*8(tp)

    mv    a0, tp
    csrrw tp, sscratch, tp
    sd    tp, 4*8(a0)

    la    t0, _CLV_SIZE
    sub   tp, a0, t0   /* restore tp, which might have been broken */
    la    t0, _ESTACK_SIZE
    add   sp, tp, t0   /* switch to the exception stack */

    jal   ExcHandler
1:
    j   1b
    sret

    .equ   EXC_UMODE_ECALL, 8
    .equ   EXC_ECALL, EXC_UMODE_ECALL

    .equ   SIZEOF_TaskControl,     0x4030

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
    mv    tp, t0       /* restore tp, which might have been broken */

    /* switch to the kernel stack */
    /* sp = &TaskControl[CurrentTask].task_kstack[KSTACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentTask)(tp)
    la    t1, TaskControl
    addi  t0, t0, 1    /* t0 = CurrentTask + 1 */
    li    t2, SIZEOF_TaskControl   /* t2 = sizeof(struct TaskControl) */
    mul   t2, t2, t0   /* t2 = sizeof(struct TaskControl)*(CurrentTask + 1) */
    mv    t3, sp
    add   sp, t2, t1   /* &TaskControl[CurrentTask].task_kstack[KSTACKSIZE] */

    addi  sp, sp, -8*4
    sd    ra, 2*8(sp)
    sd    t3, 3*8(sp)   /* previous sp */

    csrr  t0, sstatus
    csrr  t1, sepc
    addi  t1, t1, 4
    sd    t0, 0*8(sp)   /* status */
    sd    t1, 1*8(sp)   /* epc */

    jal   SvcHandler

    ld    t0, 0*8(sp)   /* status */
    ld    t1, 1*8(sp)   /* epc */
    csrw  sstatus, t0
    csrw  sepc,    t1

    ld    ra, 2*8(sp)
    ld    sp, 3*8(sp)   /* switch back to the user stack */

    sret

1:
    ld    t0, 0*8(tp)
    csrrw tp, sscratch, tp
    j     undefined_handler
    .size exc_handler,.-exc_handler

    .balign 4
int_handler:
    csrrw tp, sscratch, tp
    /* save t0-t3 temprarily on the scratch space */
    sd    t0, 5*8(tp)
    sd    t1, 6*8(tp)
    sd    t2, 7*8(tp)
    sd    t3, 8*8(tp)
    mv    t3, tp

    la    t0, _CLV_SIZE
    sub   t0, tp, t0
    csrrw tp, sscratch, tp
    mv    tp, t0       /* restore tp, which might have been broken */

    /* switch to the kernel stack */
    /* sp = &TaskControl[CurrentTask].task_kstack[KSTACKSIZE]; */
    lwu   t0, %tprel_lo(CurrentTask)(tp)
    la    t1, TaskControl
    addi  t0, t0, 1
    li    t2, SIZEOF_TaskControl   /* sizeof(TaskControl) */
    mul   t2, t2, t0
    mv    t0, sp
    add   sp, t2, t1   /* &TaskControl[CurrentTask].task_kstack[KSTACKSIZE] */

    addi  sp, sp, -8*20
    sd    t0, 17*8(sp)  /* save the previous sp */

    /* move the context from the scratch space to the kernel stack */
    ld    t0, 5*8(t3)   /* the value of t0 */
    sd    t0, 9*8(sp)
    ld    t0, 6*8(t3)   /* the value of t1 */
    sd    t0, 10*8(sp)
    ld    t0, 7*8(t3)   /* the value of t2 */
    sd    t0, 11*8(sp)
    ld    t0, 8*8(t3)   /* the value of t3 */
    sd    t0, 12*8(sp)

    sd    ra, 0*8(sp)
    sd    a0, 1*8(sp)
    sd    a1, 2*8(sp)
    sd    a2, 3*8(sp)
    sd    a3, 4*8(sp)
    sd    a4, 5*8(sp)
    sd    a5, 6*8(sp)
    sd    a6, 7*8(sp)
    sd    a7, 8*8(sp)
    sd    t4, 13*8(sp)
    sd    t5, 14*8(sp)
    sd    t6, 15*8(sp)
    sd    s0, 16*8(sp)

    csrr  a0, scause
    csrr  a1, sstatus
    csrr  a2, sepc
    sd    a1, 18*8(sp)   /* status */
    sd    a2, 19*8(sp)   /* epc */

    mv    s0, sp
    /* switch to the interrupt stack */
    la    t0, _STACK_SIZE
    add   sp, t3, t0

    jal   InterruptHandler

    /* switch back to the kernel stack */
    mv    sp, s0
    beqz  a0, 1f

    jal   _Schedule

1:
    ld    a1, 18*8(sp)   /* status */
    ld    a2, 19*8(sp)   /* epc */
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
    /* switch back to the user stack */
    ld    sp, 17*8(sp)
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

    .equ   STATUS_SPIE,      (1U<<5)
    .equ   STATUS_SPP_USER,  (0<<8)         /* user mode */
    .equ   STATUS_SPP_SMODE, (1U<<8)        /* supervisor mode */
    .equ   STATUS_SUM,       (1U<<18)

    .globl TaskStart
    .type TaskStart,@function
    .balign 4
TaskStart:
    csrw  sepc, a0
    li    a0, STATUS_SPIE|STATUS_SPP_USER|STATUS_SUM
    csrw  sstatus, a0
    mv    sp, a1
    sret
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
    csrw  satp, a0
    ret
    .size SetSATP,.-SetSATP
