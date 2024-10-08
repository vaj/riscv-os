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

#include "api.h"

unsigned long task_ustack[NUMBER_OF_TASKS][USTACKSIZE];

#define WAITTIME 20000000

unsigned int myrandom(void)
{
    static unsigned long long x = 11;
    x = (48271 * x) % 2147483647;
    return (unsigned int)x;
}

void spend_time(void)
{
    unsigned long t = get_time();
    unsigned long w = WAITTIME + myrandom()%10000000U;
    while ( get_time() - t < w );
}

int GetForks(SemIdType fork_left, SemIdType fork_right)
{
    AcquireSemaphore(fork_left);
    if (TryToAcquireSemaphore(fork_right)) {
        return TRUE;
    }
    ReleaseSemaphore(fork_left);
    return FALSE;
}

void ReleaseForks(SemIdType fork_left, SemIdType fork_right)
{
    ReleaseSemaphore(fork_left);
    ReleaseSemaphore(fork_right);
}

void PhilosopherMeditate(const TaskIdType task)
{
    print_message("Task%x Meditating\n", task + 1);
    Snooze(myrandom() % 2 + 1);
}

void PhilosopherEat(const TaskIdType task)
{
    print_message("Task%x Eating\n", task + 1);
    spend_time();
}

void TaskJob(const TaskIdType task, const SemIdType fork_left, const SemIdType fork_right)
{
    while (1) {
        while ( !GetForks(fork_left, fork_right) ) {
            PhilosopherMeditate(task);
        }
        PhilosopherEat(task);
        ReleaseForks(fork_left, fork_right);
    }
}

void Task1(void)
{
    TaskJob(TASK1, SEM1, SEM2);
}

void Task2(void)
{
    TaskJob(TASK2, SEM2, SEM3);
}

void Task3(void)
{
    TaskJob(TASK3, SEM3, SEM4);
}

void Task4(void)
{
    TaskJob(TASK4, SEM4, SEM5);
}

void Task5(void)
{
    TaskJob(TASK5, SEM5, SEM1);
}

void Idle(void)
{
    Schedule();
    while (1) {
        /* do nothing */
    }
}

