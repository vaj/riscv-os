
    .equ  SYS_SCHEDULE,                0
    .equ  SYS_ACQUIRESEMAPHORE,        1
    .equ  SYS_TRYTOACQUIRESEMAPHORE,   2
    .equ  SYS_RELEASESEMAPHORE,        3
    .equ  SYS_SNOOZE,                  4
    .equ  SYS_PRINTMSG,                5
    .equ  SYS_GETTIME,                 6

    .global Schedule
    .type Schedule,@function
Schedule:
    li a7, SYS_SCHEDULE
    ecall
    ret
    .size Schedule,.-Schedule

    .global AcquireSemaphore
    .type AcquireSemaphore,@function
AcquireSemaphore:
    li a7, SYS_ACQUIRESEMAPHORE
    ecall
    ret
    .size AcquireSemaphore,.-AcquireSemaphore

    .global TryToAcquireSemaphore
    .type TryToAcquireSemaphore,@function
TryToAcquireSemaphore:
    li a7, SYS_TRYTOACQUIRESEMAPHORE
    ecall
    ret
    .size TryToAcquireSemaphore,.-TryToAcquireSemaphore

    .global ReleaseSemaphore
    .type ReleaseSemaphore,@function
ReleaseSemaphore:
    li a7, SYS_RELEASESEMAPHORE
    ecall
    ret
    .size ReleaseSemaphore,.-ReleaseSemaphore

    .global Snooze
    .type Snooze,@function
Snooze:
    li a7, SYS_SNOOZE
    ecall
    ret
    .size Snooze,.-Snooze

    .global print_message
    .type print_message,@function
print_message:
    li a7, SYS_PRINTMSG
    ecall
    ret
    .size print_message,.-print_message

    .global get_time
    .type get_time,@function
get_time:
    li a7, SYS_GETTIME
    ecall
    ret
    .size get_time,.-get_time
