#include "unistd.h"

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

int open(const char* pathname, int flags)
{
    int fd;
    asm volatile("int $0x80"
                 : "=a"(fd)
                 : "a"(5), "b"(pathname), "c"(flags));
    return fd;
}

int mkfifo(const char* pathname, int flags)
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

int socketcall(int call, uint32_t* args)
{
    int ret;
    asm volatile("int $0x80"
                 : "=a"(ret)
                 : "a"(102), "b"(call), "c"(args));
    return ret;
}

void sched_yield()
{
    asm volatile("int $0x80"
                 :
                 : "a"(158));
}

void usleep(uint32_t ticks)
{
    asm volatile("int $0x80"
                 :
                 : "a"(162), "b"(ticks));
}

void sleep(uint32_t sec)
{
    asm volatile("int $0x80"
                 :
                 : "a"(161), "b"(sec));
}

uint32_t time()
{
    uint32_t timestamp;
    asm volatile("int $0x80"
                 : "=a"(timestamp)
                 : "a"(13));
    return timestamp;
}

int spawn(const char* pathname, char** args, uint8_t argc)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(390), "b"(pathname), "c"(args), "d"(argc));
    return pid;
}

int spawn_orphan(const char* pathname, char** args, uint8_t argc)
{
    int pid;
    asm volatile("int $0x80"
                 : "=a"(pid)
                 : "a"(391), "b"(pathname), "c"(args), "d"(argc));
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

int unlink(const char* pathname)
{
    int error;
    asm volatile("int $0x80"
                 : "=a"(error)
                 : "a"(10), "b"(pathname));
    return error;
}

int chdir(const char* dir)
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

int listdir(const char* dirname, fs_entry_t* entries, int count)
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

int sys_osinfo(struct osinfo* buffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(403), "b"(buffer));
    return result;
}

int getchar(int* character)
{
    int size;
    asm volatile("int $0x80"
                 : "=a"(size)
                 : "a"(404), "b"(character));
    return size;
}

void sys_yield()
{
    sched_yield();
}
