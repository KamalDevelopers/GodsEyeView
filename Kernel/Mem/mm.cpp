#include "mm.hpp"

static uint32_t allocate_pages_hook(size_t size)
{
    return PMM->allocate_pages(size);
}

static int free_pages_hook(uint32_t address, size_t size)
{
    return PMM->free_pages(address, size);
}

void* kmalloc(size_t size)
{
    memory_hooks(allocate_pages_hook, free_pages_hook);
    void* address = malloc(size);
    memory_hooks(0, 0);
    return address;
}

void* krealloc(void* address, size_t new_size)
{
    memory_hooks(allocate_pages_hook, free_pages_hook);
    void* new_address = krealloc(address, new_size);
    memory_hooks(0, 0);
    return new_address;
}

void* kcalloc(size_t nitems, size_t size)
{
    memory_hooks(allocate_pages_hook, free_pages_hook);
    void* address = kcalloc(nitems, size);
    memory_hooks(0, 0);
    return address;
}

void kfree(void* mem)
{
    memory_hooks(allocate_pages_hook, free_pages_hook);
    free(mem);
    memory_hooks(0, 0);
}

void* operator new(size_t size)
{
    return kmalloc(size);
}

void* operator new[](size_t size)
{
    return kmalloc(size);
}

void* operator new(size_t size, void* ptr)
{
    return ptr;
}

void* operator new[](size_t size, void* ptr)
{
    return ptr;
}

void operator delete(void* ptr)
{
    kfree(ptr);
}

void operator delete[](void* ptr)
{
    kfree(ptr);
}
