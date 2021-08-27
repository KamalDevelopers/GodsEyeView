#include "string.hpp"
#include "unistd.hpp"

extern "C" {

int main(int argc, char** argv);

[[noreturn]] void _entry()
{
    char* arguments;
    int argc = 0;

    asm("movl %%ebx, %0;"
        : "=r"(arguments));

    if (strlen(arguments))
        argc = 1;

    int status = main(argc, &arguments);
    _exit(status);

    while (1)
        ;
}
}
