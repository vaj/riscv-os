
    .section .reset,"ax",@progbits
    .option norelax
    .globl _start
_start:
    la    gp, __global_pointer$

    csrr  t0, mhartid

    /* tp = _tls_start + mhartid*_TLS_SIZE */
    la    tp, _tls_start
    la    t1, _TLS_SIZE
    mul   t2, t1, t0
    add   tp, tp, t2

    /* mscratch = tp + _CLV_SIZE */
    la    t1, _CLV_SIZE
    add   t3, tp, t1
    csrw  mscratch, t3

    /* sp = mscratch + _STACK_SIZE */
    la    t1, _STACK_SIZE
    add   sp, t3, t1

    j     main


