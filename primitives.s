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
    j   undefined_handler    /* software interrupt */
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

    .balign 4
undefined_handler:
    j   undefined_handler
    mret

    .equ   EXC_ECALL, 8

    .balign 4
exc_handler:
    addi  sp, sp, -8*3
    sd    s0, 1*8(sp)
    sd    s1, 2*8(sp)

    csrr  s0, mcause
    li    s1, EXC_ECALL
    bne   s0, s1, 1f

    sd    ra, 0*8(sp)
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
    addi  sp, sp, 8*3
    mret

1:
    ld    s0, 1*8(sp)
    ld    s1, 2*8(sp)
    addi  sp, sp, 8*3
    j     undefined_handler
    .size exc_handler,.-exc_handler

    .balign 4
timer_handler:
    addi  sp, sp, -8*18
    sd    ra, 0*8(sp)
    sd    a0, 1*8(sp)
    sd    a1, 2*8(sp)
    sd    a2, 3*8(sp)
    sd    a3, 4*8(sp)
    sd    a4, 5*8(sp)
    sd    a5, 6*8(sp)
    sd    a6, 7*8(sp)
    sd    a7, 8*8(sp)
    sd    t0, 9*8(sp)
    sd    t1, 10*8(sp)
    sd    t2, 11*8(sp)
    sd    t3, 12*8(sp)
    sd    t4, 13*8(sp)
    sd    t5, 14*8(sp)
    sd    t6, 15*8(sp)
    sd    s0, 16*8(sp)

    mv    s0, sp
    la    sp, _stack_end
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
    addi  sp, sp, 8*18
    mret
    .size timer_handler,.-timer_handler

    .equ   MIE_MTIE, 0x80
    .equ   MSTATUS_MIE, 0x8
    .equ   MSTATUS_MPIE, 0x80
    .equ   MSTATUS_MPP_USER, 0x0           /* user mode */

    .global EnableTimer
    .type EnableTimer,@function
    .balign 4
EnableTimer:
    li    t0, MIE_MTIE
    csrrs zero, mie, t0
    ret
    .size EnableTimer,.-EnableTimer

    .global EnableInt
    .type EnableInt,@function
EnableInt:
    li    t0, MSTATUS_MIE
    csrrs zero, mstatus, t0
    ret
    .size EnableInt,.-EnableInt

    .global DisableInt
    .type DisableInt,@function
DisableInt:
    li    t0, MSTATUS_MIE
    csrrc zero, mstatus, t0
    ret
    .size DisableInt,.-DisableInt

    .global SetTrapVectors
    .type SetTrapVectors,@function
SetTrapVectors:
    csrw  mtvec, a0
    ret
    .size SetTrapVectors,.-SetTrapVectors

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
    mret
    .size TaskStart,.-TaskStart

