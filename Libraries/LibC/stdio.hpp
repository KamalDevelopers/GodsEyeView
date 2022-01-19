#ifndef STDIO_HPP
#define STDIO_HPP

#define MAX_ROWS 25
#define MAX_COLS 80
#define BUFSIZ 512

#include "LibC/stdlib.hpp"
#include "LibC/unistd.hpp"
#include <stdarg.h>

static char write_buffer[512];
static int write_index = 0;
static void (*hwrite)(char*) = 0;
void puts_hook(void (*t)(char*));

void flush();
void puts(char* str);
void puti(int num);
void putc(int c);
void putx(int c);
void vprintf(const char* format, va_list v);
void printf(const char* format, ...);
void clear();
void sleep(uint32_t timer_count);
void usleep(uint32_t ms);
void beep(uint32_t ms_time, uint32_t frequency);
static void newline()
{
    putc(10);
}

#endif
