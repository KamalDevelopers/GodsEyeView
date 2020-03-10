#ifndef PRINTF_HPP
#define PRINTF_HPP

#include <stdarg.h>
#include "itoa.hpp"
#include "string.hpp"

static unsigned short* VideoMemory = (unsigned short*)0xb8000;
static int VideoMemoryIndex = 0;
static int NewLineIndex = 0;

extern void indexmng();
extern void puts(char* str);
extern void puti(int num);
extern void putc(int c);
extern void vprintf(const char *format, va_list v);
extern void printf (const char *format, ...);
extern void clear();

#endif