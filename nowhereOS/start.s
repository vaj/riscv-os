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
_start:
    la    gp, __global_pointer$
    la    sp, _stack_end
li tp, 0xDEADBEEFU
    j     main


    .text
    .globl trap_vectors
    .globl undefined_handler
    .type trap_vectors,@function
    .balign 256
trap_vectors:
undefined_handler:
    j   trap_vectors
    .size trap_vectors,.-trap_vectors

    .global SetTrapVectors
    .type SetTrapVectors,@function
SetTrapVectors:
    csrw  stvec, a0
    ret
    .size SetTrapVectors,.-SetTrapVectors
