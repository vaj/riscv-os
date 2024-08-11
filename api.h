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

#include <stdarg.h>

#define TRUE  1
#define FALSE 0

typedef enum {
    TASK1 = 0,
    TASK2,
    TASK3,
    TASK4,
    TASK5,
    TASKIDLE,
    TASKIDLE_CORE0 = TASKIDLE,
    TASKIDLE_CORE1,
    TASKIDLE_CORE2,
    NUMBER_OF_TASKS,
} TaskIdType;

typedef enum {
    SEM1 = 0,
    SEM2,
    SEM3,
    SEM4,
    SEM5,
    NUMBER_OF_SEMS,
} SemIdType;

extern void Schedule(void);
extern void AcquireSemaphore(SemIdType sem);
extern int TryToAcquireSemaphore(SemIdType sem);
extern void ReleaseSemaphore(SemIdType sem);
extern void Snooze(int tim);
extern void print_message(const char* s, ...);
extern unsigned long get_time(void);

#define USTACKSIZE 0x1000
extern unsigned long task_ustack[NUMBER_OF_TASKS][USTACKSIZE];

extern void Task1(void);
extern void Task2(void);
extern void Task3(void);
extern void Task4(void);
extern void Task5(void);
extern void Idle(void);
