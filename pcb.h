#ifndef PCB
#define PCB

#include <pthread.h>

struct Process_Control_Block
{
    int pid;
    char *state;
    int pLength;
    int total_time_spent;
    pthread_cond_t cond_var;
    int virtual_runtime;
};

#endif