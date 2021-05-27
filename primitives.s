
    .text
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

