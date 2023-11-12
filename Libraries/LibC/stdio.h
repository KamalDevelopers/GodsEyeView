#ifndef STDIO_H
#define STDIO_H

#define MAX_ROWS 25
#define MAX_COLS 80
#define BUFSIZ 512

#include "types.h"
#include "stdlib.h"
#include "unistd.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

void puts_hook(void (*t)(char*));
void flush();
void puts(char* str);
void puti(int n);
void putc(int c);
void putx(int c);
void vprintf(const char* format, va_list v);
int printf(const char* format, ...);
void snprintf(char* s, size_t n, const char* format, ...);
void clear();
void beep(uint32_t ms_time, uint32_t frequency);

#ifdef __cplusplus
}
#endif

#endif
