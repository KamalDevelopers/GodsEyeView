#ifndef ATOI_HPP
#define ATOI_HPP

static int atoi(char* array)
{
    int number = 0;
    int mult = 1;
    int n = str_len(array);

    n = (int)n < 0 ? -n : n;

    while (n--) {
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
        } else {
            number += (array[n] - '0') * mult;
            mult *= 10;
        }
    }

    return number;
}

#endif