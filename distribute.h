#ifndef DISTRIBUTE
#define DISTRIBUTE

#include <stdlib.h>
#include <string.h>
#include <math.h>
#define STR_SIZE 20

// Declarations
int generate_priority(int minPrio, int maxPrio);

int generate_process_length(char *distPL, int avgPL, int minPL, int maxPL);

int generate_interarrival_time(char *distIAT, int avgIAT, int minIAT, int maxIAT);


// Implementation
int generate_priority(int minPrio, int maxPrio)
{
    int num = (rand() % (maxPrio - minPrio + 1)) + minPrio;
    return num;
}

int generate_process_length(char distPL[STR_SIZE], int avgPL, int minPL, int maxPL)
{
    if ( strcmp(distPL, "fixed") == 0 )
    {
        return avgPL;
    }
    else if ( strcmp(distPL, "uniform") == 0 )
    {
        int length = (rand() % (maxPL - minPL + 1)) + minPL;
        return length;
    }
    else if ( strcmp(distPL, "exponential") == 0 )
    {
        double lambda = (double) 1 / avgPL;
        double x;

        do
        {
            double u = (double)rand() / (double)RAND_MAX;
            x = ((-1) * log(1 - u)) / lambda;
        } while ( minPL > x || x > maxPL );

        return (int) x;
    }

    return -1;
}

int generate_interarrival_time(char distIAT[STR_SIZE], int avgIAT, int minIAT, int maxIAT)
{
    if ( strcmp(distIAT, "fixed") == 0 )
    {
        return avgIAT;
    }
    else if ( strcmp(distIAT, "uniform") == 0 )
    {
        int length = (rand() % (maxIAT - minIAT + 1)) + minIAT;
        return length;
    }
    else if ( strcmp(distIAT, "exponential") == 0 )
    {
        double lambda = (double) 1 / avgIAT;
        double x;

        do
        {
            double u = (double)rand() / (double)RAND_MAX;
            x = ((-1) * log(1 - u)) / lambda;
        } while ( minIAT > x || x > maxIAT );

        return (int) x;
    }

    return -1;
}

#endif
