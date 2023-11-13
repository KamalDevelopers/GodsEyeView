#include "stdio.h"

int fibonacci (int i) {
    if (i <= 1) return 1;
    return fibonacci(i - 1) + fibonacci(i - 2);
}

int main () {
    int count;
    int i;
    count = 15;
    i = 0;
    while (i <= count) {
        printf("fibonacci(%d) return %d\n", i, fibonacci(i));
        i++;
    }
    return 0;
}
