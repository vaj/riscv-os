
typedef enum {
    TASK1 = 0,
    TASK2,
    TASK3,
    NUMBER_OF_TASKS,
} TaskIdType;

#define STACKSIZE 0x1000

typedef struct {
    unsigned long s0;
    unsigned long s1;
    unsigned long s2;
    unsigned long s3;
    unsigned long s4;
    unsigned long s5;
    unsigned long s6;
    unsigned long s7;
    unsigned long s8;
    unsigned long s9;
    unsigned long s10;
    unsigned long s11;
    unsigned long ra;
} context;

struct TaskControl {
    unsigned long sp;
    unsigned long task_stack[STACKSIZE];
} TaskControl[NUMBER_OF_TASKS];

TaskIdType CurrentTask;

extern void Schedule(void);
extern int switch_context(unsigned long *next_sp, unsigned long* sp);
extern void load_context(unsigned long *sp);
extern void TaskSwitch(struct TaskControl *current, struct TaskControl *next);
extern void EnableTimer(void);
extern void EnableInt(void);
extern void DisableInt(void);
extern void spend_time(void);
extern void trap_vectors(void);
extern void SetTrapVectors(unsigned long);

#define MTVEC_VECTORED_MODE 0x1U

static void put_char(char c)
{
    volatile unsigned char * const uart = (unsigned char *)0x10000000U;
    *uart = c;
}

void print_message(const char *s)
{
    while (*s) put_char(*s++);
}

void Task1(void)
{
    while (1) {
        print_message("Task1\n");
        spend_time();
        Schedule();
    }
}

void Task2(void)
{
    while (1) {
        print_message("Task2\n");
        spend_time();
        Schedule();
    }
}

void Task3(void)
{
    while (1) {
        print_message("Task3\n");
        spend_time();
        Schedule();
    }
}

void TaskSwitch(struct TaskControl *current, struct TaskControl *next)
{
    switch_context(&next->sp, &current->sp);
}

static TaskIdType ChooseNextTask(void)
{
    /* roundrobin scheduling */
    return (CurrentTask + 1) % NUMBER_OF_TASKS;
}

void Schedule(void)
{
    TaskIdType from = CurrentTask;
    CurrentTask = ChooseNextTask();
    TaskSwitch(&TaskControl[from], &TaskControl[CurrentTask]);
}

void InitTask(TaskIdType task, void (*entry)())
{
    context* p = (context *)&TaskControl[task].task_stack[STACKSIZE] - 1;
    p->ra = (unsigned long)entry;
    TaskControl[task].sp = (unsigned long)p;
}

volatile unsigned long * const reg_mtime = ((unsigned long *)0x200BFF8U);
volatile unsigned long * const reg_mtimecmp = ((unsigned long *)0x2004000U);

#define INTERVAL 10000000

void Timer(void)
{
    print_message("Timer\n");

    do {
        *reg_mtimecmp += INTERVAL;
    } while ((long)(*reg_mtime - *reg_mtimecmp) >= 0);
}

static void StartTimer(void)
{
    *reg_mtimecmp = *reg_mtime + INTERVAL;
    EnableTimer();
    EnableInt();
}

void spend_time(void)
{
    unsigned long t = *reg_mtime;
    while ( *reg_mtime - t < INTERVAL/4 );
}

static void clearbss(void)
{
    unsigned long long *p;
    extern unsigned long long _bss_start[];
    extern unsigned long long _bss_end[];

    for (p = _bss_start; p < _bss_end; p++) {
        *p = 0LL;
    }
}

void main(void) {
    clearbss();
    SetTrapVectors((unsigned long)trap_vectors + MTVEC_VECTORED_MODE);

    InitTask(TASK1, Task1);
    InitTask(TASK2, Task2);
    InitTask(TASK3, Task3);

    StartTimer();

    CurrentTask = TASK1;
    load_context(&TaskControl[CurrentTask].sp);
}
