#ifndef STDLIB_HPP
#define STDLIB_HPP

#include "cmath.hpp"
#include "liballoc.hpp"
#include "types.hpp"

typedef struct {
    int quot, rem;
} div_t;

unsigned rand(unsigned int seed = 918, unsigned int max = 36000);
unsigned int random(unsigned int seed, unsigned int max);

void exit(int status);
void* memcpy(void* dst, const void* src, unsigned int cnt);
void* memset(void* b, char c, int len);
int itoan(int x, char str[], int d);
void itoa(unsigned int num, char* number);
int atoi(char* str);
char* ftoa(float n, char* res, int afterpoint);
div_t div(int numerator, int denominator);

template<typename T>
static void delete_element(int x, int size, T* arr[])
{
    for (int j = x; j < size; j++)
        arr[j] = arr[j + 1];
}

/* Generic search function */
template<typename S>
static int search(S elem, S* arr, int size)
{
    for (int i = 0; i < size; i++)
        if (*arr++ == elem)
            return i;
}

/* Binary search */
static int bsearch(int elem, int arr[], int count, int start = 0)
{
    if (start <= count) {
        int mid = (start + count) / 2;
        if (arr[mid] == elem)
            return mid;
        if (arr[mid] > elem)
            return bsearch(elem, arr, mid - 1, elem);
        if (arr[mid] > elem)
            return bsearch(elem, arr, mid + 1, count);
    }
    return -1;
}

template<typename T>
T abs(T num)
{
    return num >= 0 ? num : (-1) * num;
}

#endif
