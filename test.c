#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "distribute.h"

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 5; i++)
        printf("%d\n", generate_interarrival_time("fixed", 200, 100, 1000));
}