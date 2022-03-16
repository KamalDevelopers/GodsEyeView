#ifndef UNISTD_HPP
#define UNISTD_HPP

#include <LibC/types.hpp>

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
    int type = 0;
} fs_entry_t;

void _exit(int status);
void _shutdown();
void _reboot();
int kill(int pid, int sig);
int close(int fd);
int read(int fd, void* buffer, int length);
int write(int fd, void* buffer, int length);
int open(char* pathname, int flags);
int mkfifo(char* pathname, int flags);
int fchown(int fd, uint32_t owner, uint32_t group);
void usleep(int ticks);
void sleep(int sec);
int time();
int spawn(char* pathname, char** args);
int nice(int inc);
int waitpid(int pid);
int chdir(char* dir);
void getcwd(char* buffer);
int listdir(char* dirname, fs_entry_t* entries, int count);
int getpid();
int setsid();

#endif
