#ifndef UNISTD_HPP
#define UNISTD_HPP

#include <LibC/types.hpp>

typedef struct fs_entry {
    char name[100];
    int type = 0;
} fs_entry_t;

void _exit(int status);
void _shutdown();
void _reboot();
int kill(int pid, int sig);
int close(int descriptor);
int read(int descriptor, void* buffer, int length);
int write(int descriptor, void* buffer, int length);
int open(char* file_name, int flags = 0);
int fchown(int descriptor, uint32_t owner, uint32_t group);
void sleep(int sec);
int spawn(char* file_name, char** args);
int waitpid(int pid);
int chdir(char* dir);
void getcwd(char* buffer);
int listdir(char* dirname, fs_entry_t* entries, int count);
int getpid();
int setsid();

#endif
