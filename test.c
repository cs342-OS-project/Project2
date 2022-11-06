#include <stdio.h>
#include <string.h>
#include "distribute.h"

int main(int argc, char const *argv[])
{
    printf("%d", generate_interarrival_time("exponential", 200, 10, 1000));
}