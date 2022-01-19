#ifndef UNISTD_HPP
#define UNISTD_HPP

void _exit(int status);
void _shutdown();
void _reboot();
int close(int descriptor);
int read(int descriptor, void* buffer, int length);
int write(int descriptor, char* buffer, int length);
int open(char* file_name);
int spawn(char* file_name, char* args);

#endif
