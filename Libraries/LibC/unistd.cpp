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

int open(char* pathname, int flags)
{
    int fd;
    asm volatile("int $0x80"
                 : "=a"(fd)
                 : "a"(5), "b"(pathname), "c"(flags));
    return fd;
}

int mkfifo(char* pathname, int flags)
{
    int fd;
    asm volatile("int $0x80"
                 : "=a"(fd)
                 : "a"(400), "b"(pathname), "c"(flags));
    return fd;
}

int fchown(int fd, uint32_t owner, uint32_t group)
{
    int error;
    asm volatile("int $0x80"
                 : "=a"(error)
                 : "a"(95), "b"(fd), "c"(owner), "d"(group));
    return error;
}

void usleep(int ticks)
{
    asm volatile("int $0x80"
                 :
                 : "a"(162), "b"(ticks));
}

void sleep(int sec)
{
    int start_time = time();
    while ((time() - start_time) <= sec)
        usleep(100);
}

int time()
{
    int timestamp;
    asm volatile("int $0x80"
                 : "=a"(timestamp)
                 : "a"(13));
    return timestamp;
}

int spawn(char* pathname, char** args)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(401), "b"(pathname), "c"(args));
    return pid;
}

int waitpid(int pid)
{
    int error;
    asm volatile("int $0x80"
                 : "=a"(error)
                 : "a"(7), "b"(pid));
    return error;
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

int nice(int inc)
{
    int error;
    asm volatile("int $0x80"
                 : "=a"(error)
                 : "a"(34), "b"(inc));
    return error;
}

int listdir(char* dirname, fs_entry_t* entries, int count)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(402), "b"(dirname), "c"(entries), "d"(count));
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

int setsid()
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(66));
    return pid;
}
