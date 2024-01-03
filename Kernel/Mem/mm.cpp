#include "mm.hpp"
#include "../Hardware/interrupts.hpp"
#include "paging.hpp"

/* FIXME: Faults when < 0x1000 */
#define KMALLOC_OFFSET 0x1000
#define KETERNAL_MAX 1024

typedef struct kmem_meta {
    uint32_t magic;
    uint32_t address;
    uint32_t pmm_address;
    uint32_t size;
} kmem_meta;

typedef struct keternal {
    uint32_t address;
    uint32_t head;
    uint32_t size;
} keternal;

static keternal keternal_page;

void kfree(void* mem)
{
    /* Eternal allocation */
    if (((uint32_t)mem >= keternal_page.address) && ((uint32_t)mem <= (keternal_page.address + PAGE_SIZE)))
        return;

    /* Kernel heap allocation */
    kmem_meta* meta = (kmem_meta*)((uint32_t)mem - KMALLOC_OFFSET);
    if (meta->address != (uint32_t)mem || meta->magic != 0xBEEF) {
        klog("invalid kfree() detected, magic=%x", meta->magic);
        return;
    }

    int total_size = meta->size + KMALLOC_OFFSET;
    if ((total_size % PAGE_SIZE) != 0)
        total_size = PAGE_ALIGN(total_size);

    meta->magic = 0;
    PMM->free_pages(meta->pmm_address, total_size);
}

void* kmalloc(size_t size)
{
    /* Eternal allocation */
    if ((size <= KETERNAL_MAX) && ((keternal_page.size + size) < PAGE_SIZE)) {
        if (!keternal_page.address) {
            keternal_page.address = PMM->allocate_pages(PAGE_SIZE);
            keternal_page.head = keternal_page.address;
        }

        uint32_t address = keternal_page.head;
        keternal_page.head += size;
        keternal_page.size += size;
        return (void*)keternal_page.head;
    }

    /* Kernel heap allocation */
    int total_size = size + KMALLOC_OFFSET;
    if ((total_size % PAGE_SIZE) != 0)
        total_size = PAGE_ALIGN(total_size);

    uint32_t address = PMM->allocate_pages(total_size);
    kmem_meta* meta = (kmem_meta*)address;
    meta->size = size;
    meta->address = address + KMALLOC_OFFSET;
    meta->pmm_address = address;
    meta->magic = 0xBEEF;
    return (void*)meta->address;
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
