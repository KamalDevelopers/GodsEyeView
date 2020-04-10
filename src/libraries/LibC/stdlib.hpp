#ifndef STDLIB_HPP
#define STDLIB_HPP
#include "stdio.hpp"

int atoi(char* array);

unsigned rand(unsigned int seed);
unsigned int random(unsigned int seed, unsigned int max);

void* malloc(int size);
void free(void* ptr);
void* sbrk(int incr);
void* memcpy(void* __restrict dst, const void* __restrict src, size_t count);
#endif