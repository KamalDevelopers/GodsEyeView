#include <LibC/exit.h>

static uint8_t counter = 0;
static void (*atexits[10])();

int atexit(void (*func)(void))
{
    if (counter >= 10)
        return 1;

    atexits[counter] = func;
    counter++;
    return 0;
}

int atexit_count()
{
    return counter;
}

uint32_t atexit_func(uint8_t i)
{
    return (uint32_t)atexits[i];
}
