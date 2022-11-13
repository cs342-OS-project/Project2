#ifndef PCB
#define PCB

#include <pthread.h>

#define WAITING 0
#define RUNNING 1
#define FINISHED 2

struct Process_Control_Block
{
    int pid;
    //int state;
    int pLength;
    int total_time_spent;
    //pthread_cond_t cond_var;
    double virtual_runtime;
    int priority;
};

#endif