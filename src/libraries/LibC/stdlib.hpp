#ifndef STDLIB_HPP
#define STDLIB_HPP
#include "stdio.hpp"

unsigned rand(unsigned int seed);
unsigned int random(unsigned int seed, unsigned int max);

void* malloc(int size);
void free(void* ptr);
void* sbrk(int incr);
void* memcpy(void* __restrict dst, const void* __restrict src, size_t count);
void* memset(void *b, char c, int len);

template <typename T>
static void deleteElement(int x, int size, T* arr[]) 
{  
    for (int j = x; j < size; j++) 
        arr[j] = arr[j + 1]; 
}
#endif