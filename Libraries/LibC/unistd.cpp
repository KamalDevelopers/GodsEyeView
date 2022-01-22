#include "unistd.hpp"

void _exit(int status)
{
    asm volatile("int $0x80"
                 :
                 : "a"(1), "b"(0));
}

int kill(int pid, int sig)
{
    int status;
    asm volatile("int $0x80"
                 : "=a"(status)
                 : "a"(37), "b"(pid), "c"(sig));
    return status;
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

int close(int fd)
{
    asm volatile("int $0x80"
                 :
                 : "a"(6), "b"(fd));
    return 0;
}

int read(int fd, void* buffer, int length)
{
    int size;
    asm volatile("int $0x80"
                 : "=a"(size)
                 : "a"(3), "b"(fd), "c"(buffer), "d"(length));
    return size;
}

int write(int fd, char* buffer, int length)
{
    asm volatile("int $0x80"
                 :
                 : "a"(4), "b"(fd), "c"(buffer), "d"(length));
    return 0;
}

int open(char* file_name)
{
    int fd;
    asm volatile("int $0x80"
                 : "=a"(fd)
                 : "a"(5), "b"(file_name));
    return fd;
}

void sleep(int sec)
{
    asm volatile("int $0x80"
                 :
                 : "a"(162), "b"(sec));
}

int spawn(char* file_name, char* args)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(401), "b"(file_name), "c"(args));

    if (pid >= 0)
        while (kill(pid, 0) != -1)
            ;

    return pid;
}
