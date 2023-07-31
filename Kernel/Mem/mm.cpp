#include "mm.hpp"
#include "../Hardware/interrupts.hpp"
#include "paging.hpp"

typedef struct kmem_meta {
    uint32_t address;
    uint32_t size;
    uint8_t allocated;
} kmem_meta;

void* kmalloc(size_t size)
{
    kmem_meta meta;
    uint32_t address = PMM->allocate_pages(PAGE_ALIGN(size) + PAGE_SIZE);
    meta.size = size;
    meta.address = address + PAGE_SIZE;
    meta.allocated = 0x10;
    memcpy((void*)address, (void*)&meta, sizeof(kmem_meta));
    return (void*)meta.address;
}

void kfree(void* mem)
{
    kmem_meta meta;
    memcpy(&meta, (void*)((uint32_t)mem - PAGE_SIZE), sizeof(kmem_meta));
    if (meta.address != (uint32_t)mem || meta.allocated != 0x10) {
        klog("invalid kfree() detected");
        return;
    }
    PMM->free_pages((uint32_t)mem - PAGE_SIZE, meta.size + PAGE_SIZE);
    meta.allocated = 0;
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

void operator delete(void* ptr, size_t size)
{
    kfree(ptr);
}

void operator delete[](void* ptr)
{
    kfree(ptr);
}
