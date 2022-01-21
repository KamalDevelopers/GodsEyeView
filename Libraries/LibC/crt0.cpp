#include "liballoc.hpp"
#include "stdio.hpp"
#include "string.hpp"
#include "unistd.hpp"

extern "C" {

int main(int argc, char** argv);

[[noreturn]] void _entry()
{
    /* Store in original pointer as 'args' to avoid manipulation before free() */
    char* arguments = (char*)malloc(100 * sizeof(char));
    char* args = arguments;
    int argc = 0;

    asm("movl %%ebx, %0;"
        : "=r"(arguments));

    if (strlen(arguments))
        argc = 1;

    int status = main(argc, &arguments);
    flush();
    free(args);
    _exit(status);

    while (1)
        ;
}
}
