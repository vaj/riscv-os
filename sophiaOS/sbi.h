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

struct sbiret {
    long error;
    long value;
};

extern struct sbiret sbi_set_timer(unsigned long stime_value);
extern struct sbiret sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base);
extern struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque);
extern struct sbiret sbi_hart_get_status(unsigned long hartid);
extern struct sbiret sbi_debug_console_write_byte(unsigned char byte);

#define SBI_SUCCESS                  0
#define SBI_ERR_FAILED              -1
#define SBI_ERR_NOT_SUPPORTED       -2
#define SBI_ERR_INVALID_PARAM       -3
#define SBI_ERR_DENIED              -4
#define SBI_ERR_INVALID_ADDRESS     -5
#define SBI_ERR_ALREADY_AVAILABLE   -6
#define SBI_ERR_ALREADY_STARTED     -7
#define SBI_ERR_ALREADY_STOPPED     -8
#define SBI_ERR_NO_SHMEM            -9
#define SBI_ERR_INVALID_STATE       -10
#define SBI_ERR_BAD_RANGE           -11

#define STARTED           0
#define STOPPED           1
#define START_PENDING     2
#define STOP_PENDING      3
#define SUSPENDED         4
#define SUSPEND_PENDING   5
#define RESUME_PENDING    6

