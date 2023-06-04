#ifndef STDLIB_H
#define STDLIB_H

#include "cmath.h"
#include "mem.h"
#include "types.h"

typedef struct {
    int quot;
    int rem;
} div_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned rand(unsigned int seed, unsigned int max);
unsigned int random(unsigned int seed, unsigned int max);

void exit(int status);
int itoan(int x, char str[], int d);
void itoa(unsigned int num, char* number);
double atof(const char* arr);
int atoi(const char* str);
char* ftoa(float n, char* res, int afterpoint);
div_t div(int numerator, int denominator);
int bsearch(int elem, int arr[], int count, int start);

#ifdef __cplusplus
}
#endif

#endif
