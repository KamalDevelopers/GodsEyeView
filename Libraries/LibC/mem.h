#ifndef CMEM_H
#define CMEM_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FIXME: Kernel sse lock, see mem.c "TODO" */
bool has_sse();
void* sse2_memset8(void* s, char c, size_t n);
void* sse2_memset32(void* s, uint32_t c, size_t n);
void* sse2_memcpy(void* to, const void* from, size_t len);
void* sse_memcpy(void* to, const void* from, size_t len);
void* mmx_memcpy(void* to, const void* from, size_t len);

int memcmp(const void* buf1, const void* buf2, size_t count);
void* memmove(void* dest, const void* src, size_t n);
void* memchr(const void* s, int c, size_t n);
void* memcpy32(void* dst, const void* src, size_t cnt);
void* memcpy(void* dst, const void* src, uint32_t cnt);
void* memset(void* d, int c, size_t n);
void* memset16(void* d, uint16_t c, size_t n);
void* memset32(void* d, uint32_t c, size_t n);

void* malloc(size_t size);
void free(const void* addr);
void* calloc(size_t count, size_t size);
void* realloc(const void* address, size_t size);

#ifdef __cplusplus
}
#endif

#endif
