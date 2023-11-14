#ifndef LIBALLOC_H
#define LIBALLOC_H

#include "types.h"

extern void*(malloc)(size_t);         ///< The standard function.
extern void*(realloc)(void*, size_t); ///< The standard function.
extern void*(calloc)(size_t, size_t); ///< The standard function.
extern void(free)(const void*);             ///< The standard function.

int memcmp(const void* buf1, const void* buf2, size_t count);
void* memmove(void *dest, const void *src, size_t n);
void* memchr(const void* s, int c, size_t n);
void* memcpy32(void* dst, const void* src, size_t cnt);
void* memcpy(void* dst, const void* src, unsigned int cnt);
void* memset(void* s, int c, size_t n);

#endif
