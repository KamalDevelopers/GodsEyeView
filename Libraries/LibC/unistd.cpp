#include "unistd.hpp"

void _exit(int status)
{
    asm volatile("int $0x80"
                 :
                 : "a"(1), "b"(0));
}

int kill(int pid, int sig)
{
    int res;
    asm volatile("int $0x80"
                 : "=a"(res)
                 : "a"(37), "b"(pid), "c"(sig));
    return res;
}

void _shutdown()
{
    asm volatile("int $0x80"
                 :
                 : "a"(88), "b"(1));
}

void _reboot()
{
    asm volatile("int $0x80"
                 :
                 : "a"(88), "b"(0));
}

int close(int descriptor)
{
    asm volatile("int $0x80"
                 :
                 : "a"(6), "b"(descriptor));
    return 0;
}

int read(int descriptor, void* buffer, int length)
{
    asm volatile("int $0x80"
                 :
                 : "a"(3), "b"(descriptor), "c"(buffer), "d"(length));
    return 0;
}

int write(int descriptor, char* buffer, int length)
{
    asm volatile("int $0x80"
                 :
                 : "a"(4), "b"(descriptor), "c"(buffer), "d"(length));
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

int spawn(char* file_name)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(401), "b"(file_name));

    if (pid != -1)
        while (kill(pid, 0) != -1)
            ;

    return pid;
}
