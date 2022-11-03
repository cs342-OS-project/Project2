#include <stdio.h>
#include <string.h>

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
