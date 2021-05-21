
    .section .reset,"ax",@progbits
    .option norelax
    .globl _start
_start:
    la    gp, __global_pointer$
    la    sp, _stack_end

    j     main


