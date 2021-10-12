
#define TRUE  1
#define FALSE 0

typedef enum {
    TASK1 = 0,
    TASK2,
    TASK3,
    TASKIDLE,
    NUMBER_OF_TASKS,
} TaskIdType;

extern void Task1(void);
extern void Task2(void);
extern void Task3(void);
extern void Task4(void);
extern void Task5(void);
extern void Idle(void);

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
    enum { READY, BLOCKED} state;
    void (*entry)(void);
    long time_slice;
    long remaining_time;
    int expire;
    unsigned long sp;
    unsigned long task_stack[STACKSIZE];
} TaskControl[NUMBER_OF_TASKS] = {
    {.entry = Task1, .state = READY, .time_slice = 2},
    {.entry = Task2, .state = READY, .time_slice = 4},
    {.entry = Task3, .state = READY, .time_slice = 1}, 
    {.entry = Idle,  .state = READY,  .time_slice = 1}, 
};

TaskIdType CurrentTask;

extern void Snooze(int tim);
extern void Schedule(void);
extern void _Schedule(void);
extern int switch_context(unsigned long *next_sp, unsigned long* sp);
extern void load_context(unsigned long *sp);
extern void TaskSwitch(struct TaskControl *current, struct TaskControl *next);
extern void EnableTimer(void);
extern void EnableInt(void);
extern void DisableInt(void);
extern void print_message(const char* s);
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
        Snooze(2);
    }
}

void Task2(void)
{
    while (1) {
        print_message("Task2\n");
        Snooze(3);
    }
}

void Task3(void)
{
    while (1) {
        print_message("Task3\n");
        Snooze(4);
    }
}

void Idle(void)
{
    while (1) {
        /* do nothing */
    }
}

void TaskSwitch(struct TaskControl *current, struct TaskControl *next)
{
    switch_context(&next->sp, &current->sp);
}

static TaskIdType ChooseNextTask(void)
{
    /* roundrobin scheduling */
    TaskIdType task = CurrentTask;

    do {
        task = (task + 1) % NUMBER_OF_TASKS;
    } while (TaskControl[task].state != READY && task != CurrentTask && task != TASKIDLE);

    if (TaskControl[task].state != READY) {
        task = TASKIDLE;
    }

    return task;
}

void _Schedule(void)
{
    TaskIdType from = CurrentTask;
    CurrentTask = ChooseNextTask();
    if (from != CurrentTask) {
        TaskSwitch(&TaskControl[from], &TaskControl[CurrentTask]); 
    }
}

void Schedule(void)
{
    DisableInt();
    _Schedule();
    EnableInt();
}

void Snooze(int tim)
{
    DisableInt();
    TaskControl[CurrentTask].state = BLOCKED;
    TaskControl[CurrentTask].expire = tim;
    _Schedule();
    EnableInt();
}

static void TaskEntry(void)
{
    EnableInt();
    TaskControl[CurrentTask].entry();
}

static void InitTask(TaskIdType task)
{
    context* p = (context *)&TaskControl[task].task_stack[STACKSIZE] - 1;
    p->ra = (unsigned long)TaskEntry;
    TaskControl[task].sp = (unsigned long)p;
    TaskControl[task].remaining_time = TaskControl[task].time_slice;
}

volatile unsigned long * const reg_mtime = ((unsigned long *)0x200BFF8U);
volatile unsigned long * const reg_mtimecmp = ((unsigned long *)0x2004000U);

#define INTERVAL 10000000

int Timer(void)
{
    TaskIdType task;

    print_message("Timer\n");

    do {
        *reg_mtimecmp += INTERVAL;
    } while ((long)(*reg_mtime - *reg_mtimecmp) >= 0);

    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        if (TaskControl[task].state == BLOCKED && TaskControl[task].expire > 0) {
            if (--TaskControl[task].expire == 0) {
                TaskControl[task].state = READY;
            }
        }
    }

    if (--TaskControl[CurrentTask].remaining_time <= 0) {
        TaskControl[CurrentTask].remaining_time = TaskControl[CurrentTask].time_slice;
        return TRUE;
    }

    return FALSE;
}

static void StartTimer(void)
{
    *reg_mtimecmp = *reg_mtime + INTERVAL;
    EnableTimer();
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
    TaskIdType task;

    clearbss();
    SetTrapVectors((unsigned long)trap_vectors + MTVEC_VECTORED_MODE);

    for (task = 0; task < NUMBER_OF_TASKS; task++) {
        InitTask(task);
    }

    StartTimer();

    CurrentTask = TASK1;
    load_context(&TaskControl[CurrentTask].sp);
}
