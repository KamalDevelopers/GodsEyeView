#ifndef MM_HPP
#define MM_HPP

#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "paging.hpp"

#define MAX_PAGE_ALIGNED_ALLOCS 32

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

extern void pfree(void* mem);
extern void mm_init(uint32_t kernel_end);
extern char* pmalloc(size_t size);
extern char* kmalloc(size_t size);
extern void kfree(void* mem);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif
