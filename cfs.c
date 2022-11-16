#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <semaphore.h>

#include "distribute.h"
#include "priority_queue.h"
#include "scheduling.h"
#include "pcb.h"

#define MIN_ARGS_C 15
#define MIN_ARGS_F 6
#define MIN_ARGS 2
#define STR_SIZE 20

#define SCHEDULER_WAITING 0
#define SCHEDULER_RUNNING 1

// Global (Shared) Data

struct priority_queue runqueue;

pthread_mutex_t lock1;

pthread_mutex_t lock2;

pthread_cond_t scheduler_cond_var;

pthread_cond_t *cond_var_array;

int *states_array;

struct timeval start, arrival, finish, running;

struct Process_Control_Block *pcb_array;
int pcb_array_currentSize;

int scheduler_mode = SCHEDULER_WAITING;

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
struct scheduler_params
{
    int outmode;
    int allp;
};

int isAllpFinished(int size);

void printArray(int *arr, int size);

void broadcast(pthread_cond_t *arr, int size);

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

        // Start Simulation
        gettimeofday(&start, NULL);

        init_queue(&runqueue, rqLen);
        pthread_mutex_init(&lock1, NULL);
        pthread_mutex_init(&lock2, NULL);
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

        if (strcmp(prog_mode, "C") == 0)
            params.mode = 0;
        else
        {
            params.mode = 1;
            strcpy(params.infile, infile);
        }
        
        struct scheduler_params sParams;
        sParams.allp = allp;
        sParams.outmode = outmode;

        pthread_create(&generator_tid, NULL, generator, (void *) &params);
        pthread_create(&scheduler_tid, NULL, scheduler, (void*) &sParams);

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
        printf("avg waiting time : %f\n", avg_wait);
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

    for (int i = 0; i < numOfProcesses; i++)
    {
        if (mode == 0)
        {
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
            printf("New Process with pid %d created\n", p_params.pid);
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

    pthread_mutex_lock(&lock1);

    // Critical Section
    insert_pcb(&runqueue, pcb);

    pthread_mutex_unlock(&lock1);

    printQueue(&runqueue);

    if ( outmode == 3 )
    {
        printf("The process with pid %d added to the runqueue\n", pcb.pid);
    }

    gettimeofday(&arrival, NULL);
    pcb.arrival_time = (arrival.tv_usec - start.tv_usec) / 1000;

    while ( pcb.remaining_pLength > 0 )
    {
        pthread_mutex_lock(&lock2);

        scheduler_mode = SCHEDULER_RUNNING;
        pthread_cond_signal(&scheduler_cond_var);

        while ( states_array[pcb.pid - 1] != RUNNING )
        {
            pthread_cond_wait(&(cond_var_array[pcb.pid - 1]), &lock2);
        }

        printf("pid %d remaining %d virtual runtime %f\n", pcb.pid, pcb.remaining_pLength, pcb.virtual_runtime);
        
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

        if (pcb.remaining_pLength <= 0)
            break;

        pthread_mutex_lock(&lock1);

        int id = pcb.pid;

        for (int i = 0; i < runqueue.currentSize; i++)
        {
            if (runqueue.heap[i].pid == id)
            {
                runqueue.heap[i].virtual_runtime = update_virtual_runtime(pcb.virtual_runtime, pcb.priority, actualruntime);
                runqueue.heap[i].remaining_pLength -= actualruntime;
            }
        }

        //delete_pcb(&runqueue);

        heapRebuild(&runqueue, 0);
            
       
        //insert_pcb(&runqueue, pcb);

        printQueue(&runqueue);

        pthread_mutex_unlock(&lock1);

        states_array[pcb.pid - 1] = WAITING;
        pcb.context_switch++;

        pthread_mutex_unlock(&lock2);
    }

    printf("FINISH\n");

    pthread_mutex_lock(&lock1);

    delete_pcb(&runqueue);

    pthread_mutex_unlock(&lock1);

    // printQueue(&runqueue);

    states_array[pcb.pid - 1] = FINISHED;
    if (outmode == 3)
    {
        printf("Process with pid %d finished\n", pcb.pid);
        printQueue(&runqueue);
    }
    gettimeofday(&finish, NULL);
    pcb_array[pcb_array_currentSize] = pcb;
    pcb_array_currentSize++;
    pcb.finish_time = (finish.tv_usec - start.tv_usec) / 1000; // dept

    scheduler_mode = SCHEDULER_RUNNING;
    pthread_cond_signal(&scheduler_cond_var);

    pthread_mutex_unlock(&lock2);

    pthread_exit(0);

}

void *scheduler(void *args)
{
    struct scheduler_params *sParams = (struct scheduler_params *) args;
    int outmode = sParams->outmode;
    int allp = sParams->allp;
    //printf("%d\n", isAllpFinished(allp));
    while ( isAllpFinished(allp) == 0 )
    {
        printf("%d\n", isAllpFinished(allp));

        pthread_mutex_lock(&lock2);

        if (runqueue.currentSize == 0 )
            scheduler_mode = SCHEDULER_WAITING;

        if (isAllpFinished(allp) == 1)
            break;

        while (scheduler_mode == SCHEDULER_WAITING )
            pthread_cond_wait(&scheduler_cond_var, &lock2);

        printf("Entered\n");

        pthread_mutex_lock(&lock1);

        struct Process_Control_Block pcb = get_min_pcb(&runqueue);

        pthread_mutex_unlock(&lock1);

        states_array[pcb.pid - 1] = RUNNING;
        pthread_cond_signal(&(cond_var_array[pcb.pid - 1]));
        //broadcast(cond_var_array, allp);

        printArray(states_array, allp);

        if (outmode == 3)
        {
            printf("Process with pid %d is selected for CPU\n", pcb.pid);
        }

        pthread_mutex_unlock(&lock2);
    }

    printf("Scheduler exit\n");

    pthread_exit(0);
}

int isAllpFinished(int size)
{
    for (int i = 0; i < size; i++)
    {
        if ( states_array[i] != FINISHED )
        {
            return 0;
        }
    }
    return 1;
}

void printArray(int *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d:%d, ", i + 1, arr[i]);
    }
    printf("\n");
}

void broadcast(pthread_cond_t *arr, int size)
{
    for (int i = 0; i < size; i++)
    {
        pthread_cond_signal(&arr[i]);
    }
}