#ifndef STDIO_HPP
#define STDIO_HPP

#define MAX_ROWS 25
#define MAX_COLS 80
#define BUFSIZ 512

#include "stdlib.hpp"
#include "unistd.hpp"
#include <stdarg.h>

void puts_hook(void (*t)(char*));
void flush();
void puts(char* str);
void puti(int num);
void putc(int c);
void putx(int c);
void vprintf(const char* format, va_list v);
void printf(const char* format, ...);
void clear();
void beep(uint32_t ms_time, uint32_t frequency);
inline void newline()
{
    putc(10);
}

#endif
