#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "distribute.h"
#include "priority_queue.h"
#include "scheduling.h"
#include "pcb.h"

#define MIN_ARGS_C 15
#define MIN_ARGS_F 6
#define MIN_ARGS 2
#define STR_SIZE 20

// Global (Shared) Data

struct priority_queue runqueue;

pthread_mutex_t lock_runqueue;

pthread_mutex_t lock_cpu;

pthread_cond_t scheduler_cond_var;

pthread_cond_t *cond_var_array;

int *states_array;

struct timeval start, arrival, finish, running;

struct Process_Control_Block *pcb_array;
int pcb_array_currentSize;

// Functions and definitions

void *generator(void *args);
void *scheduler(void *args);
void *process(void *args);

struct generator_params
{
    char distPL[STR_SIZE], distIAT[STR_SIZE];
    int avgPL, avgIAT;
    int minPL, minIAT;
    int maxPL, maxIAT;
    int minPrio, maxPrio;
    int allp;
    int mode;
    char infile[STR_SIZE];
    int outmode;
};

struct process_params
{
    int process_length;
    int priority;
    int pid;
    int outmode;
};

int isAllpFinished();

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
        char distPL[STR_SIZE], distIAT[STR_SIZE];
        int avgIAT, minIAT, maxIAT;
        int rqLen, allp, outmode;
        char outfile[STR_SIZE];
        char infile[STR_SIZE];

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
            //distPL = argv[4];
            strcpy(distPL, argv[4]); avgPL = atoi(argv[5]); minPL = atoi(argv[6]); maxPL = atoi(argv[7]);
            strcpy(distIAT, argv[8]); avgIAT = atoi(argv[9]); minIAT = atoi(argv[10]); maxIAT = atoi(argv[11]);
            rqLen = atoi(argv[12]); allp = atoi(argv[13]); outmode = atoi(argv[14]);

            printf("Test1\n");

            if ( argc == MIN_ARGS_C + 1 )
            {
                strcpy(outfile,argv[15]);
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
            strcpy(infile, argv[5]);

            if ( argc == MIN_ARGS_F + 1 )
            {
                strcpy(outfile, argv[6]);
            }
        }

        printf("Test2\n");
        // Start Simulation
        gettimeofday(&start, NULL);

        init_queue(&runqueue, rqLen);
        pthread_mutex_init(&lock_runqueue, NULL);
        pthread_mutex_init(&lock_cpu, NULL);
        pthread_cond_init(&scheduler_cond_var, NULL);

        cond_var_array = malloc(sizeof(pthread_cond_t) * allp);
        for (int i = 0; i < allp; i++)
        {
            pthread_cond_init(&(cond_var_array[i]), NULL);
        }

        states_array = malloc(sizeof(int) * allp);
        for (int i = 0; i < allp; i++)
        {
            states_array[i] = WAITING;
        }

        pcb_array = malloc(sizeof(struct Process_Control_Block) * allp);
        pcb_array_currentSize = 0;

        pthread_t generator_tid, scheduler_tid;

        struct generator_params params;
        strcpy(params.distPL, distPL); strcpy(params.distIAT, distIAT);
        params.avgPL = avgPL; params.avgIAT = avgIAT;
        params.minPL = minPL; params.minIAT = minIAT;
        params.maxPL = maxPL; params.maxIAT = maxIAT;
        params.minPrio = minPrio; params.maxPrio = maxPrio;
        params.allp = allp; params.outmode = outmode;

        if (strcmp(prog_mode, "C"))
            params.mode = 0;
        else
        {
            params.mode = 1;
            strcpy(params.infile, infile);
        }
            
        printf("Test3\n");
        pthread_create(&generator_tid, NULL, generator, (void *) &params);
        pthread_create(&scheduler_tid, NULL, scheduler, (void*) &outmode);

        pthread_join(generator_tid, NULL);
        pthread_join(scheduler_tid, NULL);

        printf("pid  arv dept  prio  cpu  waitr  turna  cs\n");
        
        int sum = 0;
        for(int i = 0; i < pcb_array_currentSize; i++)
        {
            struct Process_Control_Block tmp = pcb_array[i];
            int turna = tmp.finish_time - tmp.arrival_time;
            int waitr = turna - tmp.pLength;
            sum += waitr;
            printf("%d  %d %d  %d  %d  %d  %d  %d\n", tmp.pid, tmp.arrival_time, tmp.finish_time, tmp.priority, tmp.pLength, waitr, turna, tmp.context_switch);
        }

        double avg_wait = (double) sum / pcb_array_currentSize;
        printf("avg waiting time : %f", avg_wait);
    }

    return 0;
}

void *generator(void *args)
{
    struct generator_params *params = (struct generator_params*) args;
    int numOfProcesses = params->allp;
    int mode = params->mode;
    int outmode = params->outmode;

    int process_length, interarrival_time, priority;
    FILE *fp;

    if (mode == 1)
        fp = fopen(params->infile, "r");

    pthread_t *thread_id_array = malloc(sizeof(pthread_t) * numOfProcesses);

    printf("Test4\n");
    for (int i = 0; i < numOfProcesses; i++)
    {
        if (mode == 0)
        {
            printf("Test5\n");
            
            process_length = generate_process_length(params->distPL, params->avgPL, params->minPL, params->maxPL);
            interarrival_time = generate_interarrival_time(params->distIAT, params->avgIAT, params->minIAT, params->maxIAT);
            priority = generate_priority(params->minPrio, params->maxPrio);
        }
        else
        {
            int value = 0;
            char buffer[10];

            fscanf(fp, "%s", buffer); // PL
            fscanf(fp, "%d", &value);
            process_length = value;
            fscanf(fp, "%d", &value);
            priority = value;
            fscanf(fp, "%s", buffer); // IAT
            fscanf(fp, "%d", &value);
            interarrival_time = value;
        }
        

        struct process_params p_params;
        p_params.priority = priority;
        p_params.process_length = process_length;
        p_params.pid = i + 1;
        p_params.outmode = outmode;

        while ( isFull(&runqueue) )
            usleep( interarrival_time * 1000 );

        
        // Create thread
        pthread_create(&thread_id_array[i], NULL, process, (void *) &p_params);

        if (outmode == 3)
        {
            printf("New Process with pid %d created", p_params.pid);
        }

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
    struct process_params *params = (struct process_params*) args;

    struct Process_Control_Block pcb;
    pcb.pid = params->pid;
    pcb.priority = params->priority;
    pcb.pLength = params->process_length;
    pcb.remaining_pLength = params->process_length;
    pcb.context_switch = 0;

    pcb.virtual_runtime = 0.0;
    pcb.total_time_spent = 0; //?

    int outmode = params->outmode;

    pthread_mutex_lock(&lock_runqueue);

    // Critical Section
    insert_pcb(&runqueue, pcb);

    if ( outmode == 3 )
    {
        printf("The process with pid %d added to the runqueue\n", pcb.pid);
    }

    gettimeofday(&arrival, NULL);
    pcb.arrival_time = (arrival.tv_usec - start.tv_usec) / 1000;

    pthread_mutex_unlock(&lock_runqueue);

    while ( states_array[pcb.pid - 1] == WAITING )
    {
        pthread_cond_signal(&scheduler_cond_var);
        pthread_cond_wait(&(cond_var_array[pcb.pid - 1]), &lock_runqueue);

        // Running
        pthread_mutex_lock(&lock_cpu);

        if (outmode == 2)
        {
            gettimeofday(&running, NULL);
            int time = (running.tv_usec - start.tv_usec) / 1000;
            printf("%d %d RUNNING\n", time, pcb.pid);
        }
        else if (outmode == 3)
        {
            printf("Process with pid %d is running in CPU\n", pcb.pid);
        }

        int timeslice = calculate_timeslice(&runqueue, pcb.priority);

        int actualruntime;

        if (pcb.remaining_pLength < timeslice)
        {
            actualruntime = pcb.remaining_pLength;
        }
        else
        {
            actualruntime = timeslice;
            if (outmode == 3)
            {
                printf("Process with pid %d expired timeslice\n", pcb.pid);
            }
        }

        usleep(actualruntime * 1000);
        pcb.virtual_runtime = update_virtual_runtime(pcb.virtual_runtime, pcb.priority, actualruntime);
        pcb.remaining_pLength -= actualruntime;

        pthread_mutex_lock(&lock_runqueue);

        delete_pcb(&runqueue);

        if (pcb.pLength > 0)
        {
            states_array[pcb.pid - 1] = WAITING;
            pcb.context_switch++;
            insert_pcb(&runqueue, pcb);
        }
        else
        {
            states_array[pcb.pid - 1] = FINISHED;
            if (outmode == 3)
            {
                printf("Process with pid %d finished\n", pcb.pid);
            }
            gettimeofday(&finish, NULL);
            pcb_array[pcb_array_currentSize] = pcb;
            pcb_array_currentSize++;
            pcb.finish_time = (finish.tv_usec - start.tv_usec) / 1000; // dept
        }

        pthread_mutex_unlock(&lock_runqueue);

        pthread_mutex_unlock(&lock_cpu);
    }

    pthread_exit(0);    

}

void *scheduler(void *args)
{
    int *outmode = (int *) args;
    while ( isAllpFinished() == 0 )
    {
        pthread_cond_wait(&scheduler_cond_var, &lock_runqueue);

        // Sceduler Woken Up
        pthread_mutex_lock(&lock_runqueue);

        struct Process_Control_Block pcb = get_min_pcb(&runqueue);
        states_array[pcb.pid - 1] = RUNNING;

        pthread_mutex_unlock(&lock_runqueue);

        if (*outmode == 3)
        {
            printf("Process with pid %d is selected for CPU\n", pcb.pid);
        }

        pthread_cond_signal(&(cond_var_array[pcb.pid - 1]));

    }

    pthread_exit(0);
}

int isAllpFinished()
{
    for (int i = 0; i < runqueue.currentSize; i++)
    {
        if ( states_array[i] != FINISHED )
        {
            return 0;
        }
    }
    return 1;
}