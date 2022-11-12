#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "distribute.h"
#include "priority_queue.h"

#define MIN_ARGS_C 15
#define MIN_ARGS_F 6
#define MIN_ARGS 2

// Global (Shared) Data

struct priority_queue runqueue;

pthread_mutex_t lock_runqueue;

pthread_cond_t scheduler_cond_var;

int scheduler_sleep = 0;


// Functions and definitions

void *generator(void *args);
void *scheduler(void *args);
void *process(void *args);

struct generator_params
{
    char *distPL, distIAT;
    int avgPL, avgIAT;
    int minPL, minIAT;
    int maxPL, maxIAT;
    int minPrio, maxPrio;
    int allp;
}

struct process_params
{
    int process_length;
    int priority;
    int pid;
}

int main(int argc, char const *argv[])
{
    if ( argc < MIN_ARGS )
    {
        fprintf(stderr, "Argument number is not sufficient");
        exit(-1);
    }
    else
    {
        int minPrio, maxPrio;
        int avgPL, minPL, maxPL;
        char *distPL, *distIAT;
        int avgIAT, minIAT, maxIAT;
        int rqLen, allp, outmode;
        char *outfile = NULL;
        char *infile = NULL;

        char prog_mode[2];
        strcpy(prog_mode, argv[1]);

        if ( strcmp(prog_mode, "C") == 0)
        {
            if ( argc < MIN_ARGS_C )
            {
                fprintf(stderr, "Argument number is not sufficient");
                exit(-1);
            }
            // Take command line parameters
            minPrio = atoi(argv[2]); maxPrio = atoi(argv[3]);
            distPL = argv[4]; avgPL = atoi(argv[5]); minPL = atoi(argv[6]); maxPL = atoi(argv[7]);
            distIAT = argv[8]; avgIAT = atoi(argv[9]); minIAT = atoi(argv[10]); maxIAT = atoi(argv[11]);
            rqLen = atoi(argv[12]); allp = atoi(argv[13]); outmode = atoi(argv[14]);

            if ( argc == MIN_ARGS_C + 1 )
            {
                outfile = argv[15];
            }

        }
        else if ( strcmp(prog_mode, "F") == 0 )
        {
            if ( argc < MIN_ARGS_F )
            {
                fprintf(stderr, "Argument number is not sufficient");
                exit(-1);
            }
            // Take command line parameters
            rqLen = atoi(argv[2]);
            allp = atoi(argv[3]);
            outmode = atoi(argv[4]);
            infile = argv[5];

            if ( argc == MIN_ARGS_F + 1 )
            {
                outfile = argv[6];
            }
        }

        // Continue execution
        init_queue(&runqueue, allp);
        pthread_mutex_init(&lock_runqueue, NULL);
        pthread_cond_init(&scheduler_cond_var, NULL);

        pthread_t generator_tid, scheduler_tid;

        struct generator_params params;
        params.distPL = distPL; params.distIAT = distIAT;
        params.avgPL = avgPL; params.avgIAT = avgIAT;
        params.minPL = minPL; params.minIAT = minIAT;
        params.maxPL = maxPL; params.maxIAT = maxIAT;
        params.minPrio = minPrio; params.maxPrio = maxPrio;
        params.allp = allp;

        pthread_create(&generator_tid, NULL, generator, (void *) params);
        pthread_create(&scheduler_tid, NULL, scheduler, NULL);
    }

    return 0;
}

void *generator(void *args)
{
    struct generator_params params = (struct generator_params) args;
    int numOfProcesses = params.allp;
    int pid_counter = 1;

    pthread_t *thread_id_array = malloc(sizeof(pthread_t) * numOfProcesses);

    for (int i = 0; i < numOfProcesses; i++)
    {
        int process_length = generate_process_length(params.distPL, params.avgPL, params.minPL, params.maxPL);
        int interarrival_time = generate_interarrival_time(params.distIAT, params.avgIAT, params.minIAT, params.maxIAT);
        int priority = generate_priority(params.minPrio, params.maxPrio);

        struct process_params p_params;
        p_params.priority = priority;
        p_params.process_length = process_length;
        p_params.pid = pid_counter;

        // Create thread
        pthread_create(&thread_id_array[i], NULL, process, p_params);

        usleep(interarrival_time * 1000);
    }

    for (int i = 0; i < numOfProcesses; i++)
    {
        pthread_join(thread_id_array[i], NULL);
    }

    free(thread_id_array);
    pthread_exit(0);


}

void *process(void *args)
{
    struct process_params params = (struct process_params) args;

    struct Process_Control_Block pcb;
    pcb.pid = params.pid;
    pcb.priority = pcb.priority;
    pcb.pLength = params.process_length;

    pcb.virtual_runtime = 0.0;
    pcb.total_time_spent = 0; //?
    pcb.state = WAITING;
    pthread_cond_init(&(pcb.cond_var), NULL);

    pthread_mutex_lock(&lock_runqueue);

    // Critical Section
    insert_pcb(&runqueue, pcb);

    while (pcb.state == WAITING)
    {
        pthread_cond_wait(&pcb.cond_var, &lock_runqueue);
    }
    
    pthread_mutex_unlock(&lock_runqueue);
}

void *scheduler(void *args)
{
    pthread_mutex_lock(&lock_runqueue);

    while ( scheduler_sleep == 1)
    {
        pthread_cond_wait(&scheduler_cond_var, &lock_run_queue);
    }

    pthread_mutex_unlock(&lock_runqueue);
}
