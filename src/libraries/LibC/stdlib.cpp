#include "stdlib.hpp"

int atoi(char *array)
{    
    int number = 0;
    int mult = 1;
    int n = str_len(array);

    n = (int)n < 0 ? -n : n;

    while (n--)
    {
        if ((array[n] < '0' || array[n] > '9') && array[n] != '-') {
            if (number)
                break;
            else
                continue;
        }

        if (array[n] == '-') {
            if (number) {
                number = -number;
                break;
            }
        }
        else {
            number += (array[n] - '0') * mult;
            mult *= 10;
        }
    }

    return number;
}

unsigned rand(unsigned int seed = 918, unsigned int max = 36000)
{
    seed = seed * 1103515245 + 12345;
    return ((unsigned)(seed / ((max + 1) * 2)) % (max + 1));
}

unsigned int random(unsigned int seed, unsigned int max)
{
    return rand(seed, max);
}