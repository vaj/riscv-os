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

    .equ  SYS_SCHEDULE,                0
    .equ  SYS_ACQUIRESEMAPHORE,        1
    .equ  SYS_TRYTOACQUIRESEMAPHORE,   2
    .equ  SYS_RELEASESEMAPHORE,        3
    .equ  SYS_SNOOZE,                  4
    .equ  SYS_PRINTMSG,                5
    .equ  SYS_GETTIME,                 6

    .global Schedule
    .type Schedule,@function
Schedule:
    li a7, SYS_SCHEDULE
    ecall
    ret
    .size Schedule,.-Schedule

    .global AcquireSemaphore
    .type AcquireSemaphore,@function
AcquireSemaphore:
    li a7, SYS_ACQUIRESEMAPHORE
    ecall
    ret
    .size AcquireSemaphore,.-AcquireSemaphore

    .global TryToAcquireSemaphore
    .type TryToAcquireSemaphore,@function
TryToAcquireSemaphore:
    li a7, SYS_TRYTOACQUIRESEMAPHORE
    ecall
    ret
    .size TryToAcquireSemaphore,.-TryToAcquireSemaphore

    .global ReleaseSemaphore
    .type ReleaseSemaphore,@function
ReleaseSemaphore:
    li a7, SYS_RELEASESEMAPHORE
    ecall
    ret
    .size ReleaseSemaphore,.-ReleaseSemaphore

    .global Snooze
    .type Snooze,@function
Snooze:
    li a7, SYS_SNOOZE
    ecall
    ret
    .size Snooze,.-Snooze

    .global print_message
    .type print_message,@function
print_message:
    li a7, SYS_PRINTMSG
    ecall
    ret
    .size print_message,.-print_message

    .global get_time
    .type get_time,@function
get_time:
    li a7, SYS_GETTIME
    ecall
    ret
    .size get_time,.-get_time
