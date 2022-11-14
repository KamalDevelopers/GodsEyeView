#include "cpuid.hpp"
#include <LibC/string.h>

static cpuid_t cpu;

cpuid_t* get_cpu()
{
    return &cpu;
}

void cpuid(uint32_t reg, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    asm volatile("cpuid"
                 : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                 : "0"(reg));
}

void cpuid_detect()
{
    uint32_t eax = 0;
    uint32_t ebx = 0;
    uint32_t ecx = 0;
    uint32_t edx = 0;
    uint32_t fun = 0;

    cpuid(0, &fun, (uint32_t*)cpu.vendor, (uint32_t*)(cpu.vendor + 8), (uint32_t*)(cpu.vendor + 4));
    cpu.vendor[12] = 0;

    cpuid(0x80000000, &fun, &ebx, &ecx, &edx);
    if (fun >= 0x80000001) {
        cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
        cpu.is64 = (edx & EDX_64_BIT);
    }

    char name[48];
    memset(name, 0, 48);
    const char* name_ptr = name;

    if (fun >= 0x80000004) {
        cpuid(0x80000002, (uint32_t*)name, (uint32_t*)(name + 4), (uint32_t*)(name + 8), (uint32_t*)(name + 12));
        cpuid(0x80000003, (uint32_t*)(name + 16), (uint32_t*)(name + 20), (uint32_t*)(name + 24), (uint32_t*)(name + 28));
        cpuid(0x80000004, (uint32_t*)(name + 32), (uint32_t*)(name + 36), (uint32_t*)(name + 40), (uint32_t*)(name + 44));

        name_ptr = name;
        while (*name_ptr == ' ')
            name_ptr++;
    }

    uint32_t length = strlen(name_ptr);
    if (length >= 5) {
        if (strncmp(name_ptr, "QEMU", 4) == 0)
            length = 5;
    }
    strncpy(cpu.name, name_ptr, length);
}
