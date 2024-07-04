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
