#include "posix_types.h"

struct k22info {
    char comm[64];                      /* name of the executable */
    __kernel_pid_t pid;                 /* process ID */
    __kernel_pid_t parent_pid;          /* parent process ID */
    __kernel_pid_t first_child_pid;     /* PID of youngest alive child */
    __kernel_pid_t next_sibling_pid;    /* PID of next (i.e., "youngest") alive sibling */
    unsigned long nvcsw;                /* number of voluntary context switches */
    unsigned long nivcsw;               /* number of involuntary context switches */
    unsigned long start_time;           /* monotonic start time in nanoseconds */
    unsigned long userpc;               /* pc/ip when the task returns to user mode */
    unsigned long kernelpc;             /* pc/ip when the task is run by schedule */
};


int k22tree(struct k22info *buf, int *ne);  //Prototype of the syscall
