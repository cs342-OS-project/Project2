#include <stdio.h>
#include <string.h>

#include "distribute.h"

#define MIN_ARGS_C 16
#define MIN_ARGS_F 7
#define MIN_ARGS 2

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
        int avgPL, minPL, maxPL, distPL;
        int avgIAT, minIAT, maxIAT, distIAT;
        int rqLen;

        char prog_mode[2];
        strcpy(prog_mode, argv[1]);

        if ( strcmp(prog_mode, "C") == 0)
        {
            if ( argc != MIN_ARGS_C )
            {
                fprintf(stderr, "Argument number is not sufficient");
                exit(-1);
            }
            // Take command line parameters
            minPrio = atoi(argv[2]); maxPrio = atoi(argv[3]);
            distPL = argv[4]; avgPL = atoi(argv[5]); minPL = atoi(argv[6]); maxPL = atoi(argv[7]);
            distIAT = argv[8]; avgIAT = atoi(argv[9]); minIAT = atoi(argv[10]); maxIAT = atoi(argv[11]);
            rqLen = atoi(argv[12]);



        }
        else if ( strcmp(prog_mode, "F") == 0 )
        {
            if ( argc != MIN_ARGS_F )
            {
                fprintf(stderr, "Argument number is not sufficient");
                exit(-1);
            }
            // Take command line parameters and params in the file
        }

        // Continue execution
    }

    return 0;
}
