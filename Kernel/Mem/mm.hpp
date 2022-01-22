#ifndef MM_HPP
#define MM_HPP

#include "pmm.hpp"
#include <LibC/liballoc.hpp>

void* kmalloc(size_t size);
void* krealloc(void* address, size_t new_size);
void* kcalloc(size_t nitems, size_t size);
void kfree(void* mem);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif
