#ifndef UNISTD_HPP
#define UNISTD_HPP

void _exit(int status);
void _shutdown();
void _reboot();
int close(int descriptor);
int read(int descriptor, void* buffer, int length);
int write(int descriptor, void* buffer, int length);
int open(char* file_name, int flags = 0);
void sleep(int sec);
int spawn(char* file_name, char** args);
int waitpid(int pid);
int chdir(char* dir);
void getcwd(char* buffer);
int listdir(char* dirname, char** entries);
int getpid();

#endif
