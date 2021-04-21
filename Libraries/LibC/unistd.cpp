#include "unistd.hpp"

void _exit(int status)
{
    asm volatile("int $0x80"
                 :
                 : "a"(1), "b"(0));
}

int close(int descriptor)
{
    asm volatile("int $0x80"
                 :
                 : "a"(descriptor));
    return 0;
}

int read(int descriptor, void* buffer, int length)
{
    /* syscall 3: read() */
    asm volatile("int $0x80"
                 :
                 : "a"(3), "b"(descriptor), "c"(buffer), "d"(length));
    return 0;
}

int open(char* file_name)
{
    int descriptor;
    asm volatile("int $0x80"
                 : "=a"(descriptor)
                 : "a"(5), "b"(file_name));
    return descriptor;
}
