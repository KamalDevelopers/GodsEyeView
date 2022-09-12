#ifndef LIBALLOC_H
#define LIBALLOC_H

#include "types.h"

/** \defgroup ALLOCHOOKS liballoc hooks
 *
 * These are the OS specific functions which need to
 * be implemented on any platform that the library
 * is expected to work on.
 */

/** @{ */

// If we are told to not define our own size_t, then we skip the define.
//#define _HAVE_UINTPTR_T
// typedef	unsigned long	uintptr_t;

// This lets you prefix malloc and friends

#ifdef __cplusplus
extern "C" {
#endif

extern void*(malloc)(size_t);         ///< The standard function.
extern void*(realloc)(void*, size_t); ///< The standard function.
extern void*(calloc)(size_t, size_t); ///< The standard function.
extern void(free)(void*);             ///< The standard function.

extern void memory_hooks(uint32_t (*pmalloc)(size_t), int (*pfree)(uint32_t, size_t));

void* memchr(const void* str, int c, size_t n);
void* memcpy32(void* dst, const void* src, size_t cnt);
void* memcpy(void* dst, const void* src, unsigned int cnt);
void* memset(void* s, int c, size_t n);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
