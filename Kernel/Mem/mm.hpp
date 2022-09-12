#ifndef MM_HPP
#define MM_HPP

#include "pmm.hpp"
#include <LibC/mem.h>

void* kmalloc(size_t size);
void* krealloc(void* address, size_t new_size);
void* kcalloc(size_t nitems, size_t size);
void kfree(void* mem);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr);

#endif
