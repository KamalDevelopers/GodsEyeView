#ifndef MM_HPP
#define MM_HPP

#include "pmm.hpp"
#include <LibC/types.h>

void* kmalloc_non_eternal(size_t size, const char* comment);
void* kmalloc(size_t size);
void kfree(void* mem);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr);

#endif
