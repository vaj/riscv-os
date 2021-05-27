
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
        Schedule();
    }
}

void Task2(void)
{
    while (1) {
        print_message("Task2\n");
        Schedule();
    }
}

void Task3(void)
{
    while (1) {
        print_message("Task3\n");
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

    InitTask(TASK1, Task1);
    InitTask(TASK2, Task2);
    InitTask(TASK3, Task3);

    CurrentTask = TASK1;
    load_context(&TaskControl[CurrentTask].sp);
}
