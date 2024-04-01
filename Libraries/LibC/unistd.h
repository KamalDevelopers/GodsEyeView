#ifndef UNISTD_H
#define UNISTD_H

#include "types.h"

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_APPEND 0x0008
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
#define O_EXCL 0x0800
#define O_SYNC 0x2000
#define O_CLOEXEC 0x40000
#define O_NOCTTY 0x8000

typedef struct fs_entry {
    char name[100];
    int type;
} fs_entry_t;

struct osinfo {
    uint32_t used_pages;
    uint32_t free_pages;
    int procs;
    int procs_sleeping;
    int procs_zombie;
    int procs_polling;
    uint32_t uptime;
    uint32_t uptime_ms;
    char cpu_string[50];
    uint32_t cpu_task_running_time;
    bool cpu_is64;
};

#ifdef __cplusplus
extern "C" {
#endif

void _exit(int status);
void _shutdown();
void _reboot();
int kill(int pid, int sig);
int close(int fd);
int read(int fd, void* buffer, int length);
int write(int fd, void* buffer, int length);
int open(const char* pathname, int flags);
int mkfifo(const char* pathname, int flags);
int fchown(int fd, uint32_t owner, uint32_t group);
int socketcall(int call, uint32_t* args);
void usleep(int ticks);
void sched_yield();
void sleep(int sec);
uint32_t time();
int spawn(const char* pathname, char** args, uint8_t argc);
int spawn_orphan(const char* pathname, char** args, uint8_t argc);
int nice(int inc);
int waitpid(int pid);
int unlink(const char* pathname);
int chdir(const char* dir);
void getcwd(char* buffer);
int listdir(const char* dirname, fs_entry_t* entries, int count);
int getpid();
int setsid();
int sys_osinfo(struct osinfo* buffer);
int getchar(int* character);
void sys_yield();

#ifdef __cplusplus
}
#endif

#endif
