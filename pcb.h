#ifndef PCB
#define PCB

#include <pthread.h>

#define WAITING 0
#define RUNNING 1
#define FINISHED 2

struct Process_Control_Block
{
    int pid;
    int pLength;
    int remaining_pLength;
    int total_time_spent;
    double virtual_runtime;
    int priority;
    int arrival_time;
    int finish_time;
    int context_switch;
};

#endif