#ifndef CPUID_HPP
#define CPUID_HPP

#include <LibC/types.h>

#define EDX_64_BIT (1 << 29)

typedef struct cpuid {
    char name[50];
    char vendor[13];
    bool is64;
} cpuid_t;

void cpuid_detect();
cpuid_t* get_cpu();

#endif
