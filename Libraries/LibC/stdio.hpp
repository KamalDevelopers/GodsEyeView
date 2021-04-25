#ifndef STDIO_HPP
#define STDIO_HPP

#define MAX_ROWS 25
#define MAX_COLS 80

#include "LibC/stdlib.hpp"
#include "LibC/string.hpp"
#include "LibC/unistd.hpp"
#include <stdarg.h>

static void (*hwrite)(char*) = 0;
extern void puts_hook(void (*t)(char*));

extern void puts(char* str);
extern void puti(int num);
extern void putc(int c);
extern void putx(int c);
extern void vprintf(const char* format, va_list v);
extern void printf(const char* format, ...);
extern void clear();
extern void sleep(uint32_t timer_count);
extern void usleep(uint32_t ms);
extern void beep(uint32_t ms_time, uint32_t frequency);
static void newline()
{
    putc(10);
}

#endif
