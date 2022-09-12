#include "mem.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

#ifdef __cplusplus
void* operator new(long unsigned int size)
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void* ptr, size_t size)
{
    free(ptr);
}

void operator delete[](void* ptr)
{
    free(ptr);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char** argv);

void _entry()
{
    uint32_t argument_pointer;
    char** arguments;
    int argc = 0;

    asm("movl %%edx, %0;"
        : "=r"(argument_pointer));

    memory_hooks(0, 0);
    flush();

    arguments = (char**)malloc(sizeof(char*) * 10);
    for (uint32_t i = 0; i < 10; ++i)
        arguments[i] = (char*)malloc(50);

    for (uint32_t i = 0; i < 10; i++)
        memset(arguments[i], 0, 50);

    char* arg = strtok((char*)argument_pointer, (char*)" ");
    while (arg) {
        if (arg) {
            strcpy(arguments[argc], arg);
            argc++;
        }
        arg = strtok(NULL, (char*)" ");
    }

    int status = main(argc, (char**)arguments);

    for (uint32_t i = 0; i < 10; i++)
        free(arguments[i]);
    free(arguments);

    flush();
    _exit(status);

    while (1)
        ;
}

#ifdef __cplusplus
}
#endif
