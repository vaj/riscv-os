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

#include "sbi.h"
#include <stdarg.h>

#define TRUE  1
#define FALSE 0

extern void trap_vectors(void);
extern void SetTrapVectors(unsigned long);

static void _print_message(const char *s, ...)
{
    va_list ap;
    va_start (ap, s);

    while (*s) {
        if (*s == '%' && *(s+1) == 'x') {
            unsigned long v = va_arg(ap, unsigned long);
            _Bool print_started = FALSE;
            int i;

            s += 2;
            for (i = 15; i >= 0; i--) {
                unsigned long x = (v & 0xFUL << i*4) >> i*4;
                if (print_started || x != 0U || i == 0) {
                    print_started = TRUE;
                    sbi_debug_console_write_byte((x < 10 ? '0' : 'a' - 10) + x);
                }
            }
        } else {
            sbi_debug_console_write_byte(*s++);
        }
    }

    va_end (ap);
}

void main(void)
{
    SetTrapVectors((unsigned long)trap_vectors);

    _print_message("nowhere guest started\n");

    while (1) {
        static volatile unsigned int loop_count = 0;
        if (++loop_count % 0x8000000U == 0U) {
            _print_message("nowhere guest is running.\n");
        }
    }
}

