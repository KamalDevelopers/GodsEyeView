#ifndef UNISTD_HPP
#define UNISTD_HPP


extern void _exit(int status);
extern int close(int descriptor);
extern int read(int descriptor, void* buffer, int length);
extern int open(char* file_name);

#endif
