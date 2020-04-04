#ifndef STDIO_HPP
#define STDIO_HPP

#include "itoa.hpp"
#include "string.hpp"
#include <stdarg.h>

static unsigned short* VideoMemory = (unsigned short*)0xb8000;
static int VideoMemoryIndex = 0;
static int NewLineIndex = 0;

extern void indexmng();
extern void puts(char* str);
extern void puti(int num);
extern void putc(int c);
extern void putx(int c);
extern void vprintf(const char* format, va_list v);
extern void printf(const char* format, ...);
extern void clear();
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
extern void sleep(uint32_t timer_count);

#endif