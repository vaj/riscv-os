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

    .section .reset,"ax",@progbits
    .option norelax
    .globl _start
    .globl _secondary_start
_start:
    mv    a0, zero

_secondary_start:
    /* a0 = mhartid */
    la    gp, __global_pointer$

    /* tp = _tls_start + mhartid*_TLS_SIZE */
    la    tp, _tls_start
    la    t1, _TLS_SIZE
    mul   t2, t1, a0
    add   tp, tp, t2

    /* sscratch = tp + _CLV_SIZE */
    la    t1, _CLV_SIZE
    add   t3, tp, t1
    csrw  sscratch, t3

    /* sp = sscratch + _STACK_SIZE */
    la    t1, _STACK_SIZE
    add   sp, t3, t1

    j     main

