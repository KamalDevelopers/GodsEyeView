#include "mem.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "exit.h"

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

__attribute__((noreturn)) void __stack_chk_fail(void)
{
    uint32_t stk = 0;
    asm ("movl %%ebp,%0" : "=r"(stk) ::);
    asm volatile("int $0x80"
                 :
                 : "a"(444), "b"(stk), "c"(1));
    for (;;)
        ;
}

__attribute__((noreturn)) void __stack_chk_fail_local(void)
{
    __stack_chk_fail();
}

uintptr_t __stack_chk_guard = 0xe2dee396;

void _entry()
{
    uint32_t argument_pointer;
    asm("movl %%edx, %0;"
        : "=r"(argument_pointer));

    int argc;
    asm("movl %%ecx, %0;"
          : "=r"(argc));

    /* NOTE: No need to deallocate
     * arguments_dynamic as it will live
     * through entire program execution. */
    static char* arguments_static[100];
    char** arguments_dynamic = 0;
    char** args = arguments_static;
    char* arg = strtok((char*)argument_pointer, " ");
    if (argc >= 100) {
        arguments_dynamic = (char**)calloc(argc, sizeof(char*));
        args = arguments_dynamic;
    }

    /* Arguments are given to us
     * as a single string. */
    int count = 0;
    while (arg && (count < argc)) {
        /* Ignore 'disown' argument */
        if (arg[0] == '\0' || !(arg[0] == '&' && arg[1] == '\0')) {
            args[count] = arg;
            count++;
        } else {
            argc--;
        }
        arg = strtok(NULL, " ");
    }

    flush();
    int status = main(argc, (char**)args);
    uint32_t exits = atexit_count();
    void (*exit_func)(void);

    for (uint32_t i = 0; i < exits; i++) {
        exit_func = (void(*)(void))(atexit_func(i));
        exit_func();
    }

    flush();
    _exit(status);

    while (1)
        ;
}

#ifdef __cplusplus
}
#endif
