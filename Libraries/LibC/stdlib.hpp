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
void* memcpy32(void* dst, const void* src, size_t cnt);
void* memcpy(void* dst, const void* src, unsigned int cnt);
void* memset(void* s, int c, size_t n);
int itoan(int x, char str[], int d);
void itoa(unsigned int num, char* number);
int atoi(char* str);
char* ftoa(float n, char* res, int afterpoint);
div_t div(int numerator, int denominator);
int bsearch(int elem, int arr[], int count, int start);

#endif
