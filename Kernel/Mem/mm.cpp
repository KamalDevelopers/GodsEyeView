#include "mm.hpp"

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t* pheap_desc = 0;
uint32_t memory_used = 0;

void mm_init(uint32_t kernel_end)
{
    last_alloc = kernel_end + 0x1000;
    heap_begin = last_alloc;
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096); // 0x400000 - 0x20000
    heap_end = pheap_begin;
    memset((char*)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t*)kmalloc(MAX_PAGE_ALIGNED_ALLOCS);
}

char* pmalloc(size_t size)
{
    if (size % 4096 != 0) {
        klog("pmalloc: not page aligned");
        return 0;
    }

    uint32_t pages = size / 4096;
    for (int i = 0; i < MAX_PAGE_ALIGNED_ALLOCS; i++) {
        if (pheap_desc[i] == 1)
            continue;

        for (int j = 0; j < pages; j++) {
            pheap_desc[i + j] = 1;
        }
        return (char*)(pheap_begin + i * size);
    }
    klog("pmalloc: no more memory");
    return 0;
}

void pfree(void* mem, size_t size)
{
    uint32_t pages = size / 4096;
    uint32_t ad = (uint32_t)mem;
    ad -= pheap_begin;
    ad /= 4096;

    if (ad < 0 || ad > MAX_PAGE_ALIGNED_ALLOCS) {
        klog("pfree: failed to free memory");
        return;
    }

    for (int i = 0; i < pages; i++)
        pheap_desc[ad + i] = 0;
    return;
}

char* kmalloc(size_t size)
{
    if (!size)
        return 0;

    uint8_t* mem = (uint8_t*)heap_begin;
    while ((uint32_t)mem < last_alloc) {
        alloc_t* a = (alloc_t*)mem;

        if (!a->size)
            break;

        if (a->status) {
            mem += a->size;
            mem += sizeof(alloc_t);
            mem += 4;
            continue;
        }

        if (a->size >= size) {
            a->status = 1;

            memset(mem + sizeof(alloc_t), 0, size);
            memory_used += size + sizeof(alloc_t);
            return (char*)(mem + sizeof(alloc_t));
        }

        mem += a->size;
        mem += sizeof(alloc_t);
        mem += 4;
    }

    if (last_alloc + size + sizeof(alloc_t) >= heap_end) {
        return 0;
    }
    alloc_t* alloc = (alloc_t*)last_alloc;
    alloc->status = 1;
    alloc->size = size;

    last_alloc += size;
    last_alloc += sizeof(alloc_t);
    last_alloc += 4;
    memory_used += size + 4 + sizeof(alloc_t);
    memset((char*)((uint32_t)alloc + sizeof(alloc_t)), 0, size);
    return (char*)((uint32_t)alloc + sizeof(alloc_t));
}

void kfree(void* mem)
{
    alloc_t* alloc = (alloc_t*)((size_t)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
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
