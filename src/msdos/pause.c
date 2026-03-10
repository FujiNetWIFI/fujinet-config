#include <time.h>
#include "../pause.h"

#define TICKS_PER_SEC 60

void pause(unsigned char delay)
{
    clock_t start = clock();
    clock_t ticks = (clock_t)delay * CLOCKS_PER_SEC / TICKS_PER_SEC;

    while ((clock() - start) < ticks)
        ;
}
