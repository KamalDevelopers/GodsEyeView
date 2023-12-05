#ifndef LIBALLOC_H
#define LIBALLOC_H

#include "types.h"

int memcmp(const void* buf1, const void* buf2, size_t count);
void* memmove(void *dest, const void *src, size_t n);
void* memchr(const void* s, int c, size_t n);
void* memcpy32(void* dst, const void* src, size_t cnt);
void* memcpy(void* dst, const void* src, uint32_t cnt);
void* memset(void* s, int c, size_t n);

void* malloc(size_t size);
void free(const void* addr);
void* calloc(size_t count, size_t size);

#endif
