#ifndef MM_HPP
#define MM_HPP

#include "pmm.hpp"

void* kmalloc(size_t size);
void kfree(void* mem);

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr);

#endif
