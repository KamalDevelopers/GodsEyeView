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
int atoi(char* str);
char* ftoa(float n, char* res, int afterpoint);
div_t div(int numerator, int denominator);
int bsearch(int elem, int arr[], int count, int start);

/* TODO: Move network functions */
uint8_t htonb(uint8_t byte, int num_bits);
uint8_t ntohb(uint8_t byte, int num_bits);
uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);
uint16_t ntohs(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);

#ifdef __cplusplus
}
#endif

#endif
