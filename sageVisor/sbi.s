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

    .equ  EID_TIME,                    ('T'<<24U|'I'<<16U|'M'<<8U|'E')
    .equ  EID_sPI,                     ('s'<<16U|'P'<<8U|'I')
    .equ  EID_HSM,                     ('H'<<16U|'S'<<8U|'M')
    .equ  EID_DBCN,                    ('D'<<24U|'B'<<16U|'C'<<8U|'N')

    .equ  FID_TIME_SETTIMER,           0
    .equ  FID_sPI_SENDIPI,             0
    .equ  FID_HART_START,              0
    .equ  FID_HART_GETSTATUS,          2
    .equ  FID_DBCN_WRITE,              0
    .equ  FID_DBCN_READ,               1
    .equ  FID_DBCN_WRITE_BYTE,         2

    .global sbi_set_timer
    .type sbi_set_timer,@function
sbi_set_timer:
    li a7, EID_TIME
    li a6, FID_TIME_SETTIMER
    ecall
    ret
    .size sbi_set_timer,.-sbi_set_timer

    .global sbi_send_ipi
    .type sbi_send_ipi,@function
sbi_send_ipi:
    li a7, EID_sPI
    li a6, FID_sPI_SENDIPI
    ecall
    ret
    .size sbi_send_ipi,.-sbi_send_ipi

    .global sbi_hart_start
    .type sbi_hart_start,@function
sbi_hart_start:
    li a7, EID_HSM
    li a6, FID_HART_START
    ecall
    ret
    .size sbi_hart_start,.-sbi_hart_start

    .global sbi_hart_get_status
    .type sbi_hart_get_status,@function
sbi_hart_get_status:
    li a7, EID_HSM
    li a6, FID_HART_GETSTATUS
    ecall
    ret
    .size sbi_hart_get_status,.-sbi_hart_get_status

    .global sbi_debug_console_write_byte
    .type sbi_debug_console_write_byte,@function
sbi_debug_console_write_byte:
    li a7, EID_DBCN
    li a6, FID_DBCN_WRITE_BYTE
    ecall
    ret
    .size sbi_debug_console_write_byte,.-sbi_debug_console_write_byte

