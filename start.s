
    .section .reset,"ax",@progbits
    .option norelax
    .globl _start
_start:
    la    gp, __global_pointer$
    la    sp, _stack_end
    la    t0, _exc_stack_start
    csrw  mscratch, t0

    j     main


