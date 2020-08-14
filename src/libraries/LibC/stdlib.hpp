#ifndef STDLIB_HPP
#define STDLIB_HPP
#include "stdio.hpp"

typedef struct {
  int quot, rem;
} div_t;

extern unsigned rand(unsigned int seed);
extern unsigned int random(unsigned int seed, unsigned int max);

extern void* memcpy(void* dst, const void* src, unsigned int cnt);
extern void* memset(void *b, char c, int len);
extern int atoi(char* str);
extern unsigned int abs(int num);
extern div_t div(int numerator, int denominator);

template <typename T>
static void deleteElement(int x, int size, T* arr[])
{
    for (int j = x; j < size; j++)
        arr[j] = arr[j + 1];
}

template <typename S>
static int bsearch(S elem, S* arr, int size)
{
    for (int i = 0; i < size; i++)
        if (*arr++ == elem) return i;
}
#endif
