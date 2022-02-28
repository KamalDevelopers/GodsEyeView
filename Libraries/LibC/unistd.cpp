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

int write(int fd, void* buffer, int length)
{
    asm volatile("int $0x80"
                 :
                 : "a"(4), "b"(fd), "c"(buffer), "d"(length));
    return 0;
}

int open(char* file_name, int flags)
{
    int fd;
    asm volatile("int $0x80"
                 : "=a"(fd)
                 : "a"(5), "b"(file_name), "c"(flags));
    return fd;
}

int fchown(int descriptor, uint32_t owner, uint32_t group)
{
    int status;
    asm volatile("int $0x80"
                 : "=a"(descriptor)
                 : "a"(95), "b"(descriptor), "c"(owner), "d"(group));
    return status;
}

void sleep(int sec)
{
    asm volatile("int $0x80"
                 :
                 : "a"(162), "b"(sec));
}

int spawn(char* file_name, char** args)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(401), "b"(file_name), "c"(args));
    return pid;
}

int waitpid(int pid)
{
    int status;
    asm volatile("int $0x80"
                 : "=a"(status)
                 : "a"(7), "b"(pid));
    return status;
}

int chdir(char* dir)
{
    int exists;
    asm volatile("int $0x80"
                 : "=a"(exists)
                 : "a"(12), "b"(dir));
    return exists;
}

void getcwd(char* buffer)
{
    asm volatile("int $0x80"
                 :
                 : "a"(183), "b"(buffer));
}

int listdir(char* dirname, char** entries)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(402), "b"(dirname), "c"(entries));
    return result;
}

int getpid()
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(20));
    return pid;
}
