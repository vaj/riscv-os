#include "api.h"

unsigned long task_ustack[NUMBER_OF_TASKS][USTACKSIZE];

#define WAITTIME 2500000


unsigned int myrandom(void)
{
    static unsigned long long x = 11;
    x = (48271 * x) % 2147483647;
    return (unsigned int)x;
}

void spend_time(void)
{
    unsigned long t = get_time();
    while ( get_time() - t < WAITTIME );
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

void PhilosopherMeditate()
{
    print_message("    Meditating\n");
    Snooze(myrandom() % 2 + 1);
}

void PhilosopherEat()
{
    print_message("    Eating\n");
    spend_time();
    Snooze(myrandom() % 5 + 1);
}

void TaskJob(const TaskIdType task, const SemIdType fork_left, const SemIdType fork_right)
{
    while (1) {
        while ( !GetForks(fork_left, fork_right) ) {
            print_message("Task%x", task + 1);
            PhilosopherMeditate();
        }
        print_message("Task%x", task + 1);
        PhilosopherEat();
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
    while (1) {
        /* do nothing */
    }
}

