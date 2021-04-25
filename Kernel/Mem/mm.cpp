#include "mm.hpp"

MemoryManager* MemoryManager::active = 0;
MemoryManager::MemoryManager(uint32_t kernel_end, uint32_t phys)
{
    active = this;
    memory_used = 0;

    heap_begin = 0x500000;
    pheap_end = phys;

    heap_end = heap_begin + 30 * MB;
    pheap_begin = heap_end;

    max_page_aligned = (pheap_end - pheap_begin) / 4096;
    last_alloc = heap_begin;

    memset((char*)heap_begin, 0, heap_end - heap_begin);
    pheap_desc = (uint8_t*)kmalloc(max_page_aligned);
}

MemoryManager::~MemoryManager()
{
    if (active == this)
        active = 0;
}

void MemoryManager::dump()
{
    kprintf("kheap start: 0x%x\n", heap_begin);
    kprintf("kheap end: 0x%x\n", heap_end);
    kprintf("pheap start: 0x%x\n", pheap_begin);
    kprintf("pheap end: 0x%x\n\n", pheap_end);

    kprintf("Kernel reserved space: %d MB\n", heap_begin / MB);
    kprintf("Available memory: %d MB\n\n", (pheap_end - heap_begin) / MB);

    kprintf("Kernel memory used: %d KiB\n", memory_used / KiB);
    kprintf("Free kernel memory: %d MB\n\n", (heap_end - heap_begin - memory_used) / MB);

    kprintf("pheap size: %d pages\n", max_page_aligned);
    kprintf("kheap size: %d pages\n\n", (heap_end - heap_begin) / 4096);
}

char* MemoryManager::pmalloc(size_t size)
{
    if (size % 4096 != 0) {
        klog("pmalloc: not page aligned");
        return 0;
    }

    uint32_t pages = size / 4096;
    for (int i = 0; i < max_page_aligned; i++) {
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

void MemoryManager::pfree(void* mem, size_t size)
{
    uint32_t pages = size / 4096;
    uint32_t ad = (uint32_t)mem;
    ad -= pheap_begin;
    ad /= 4096;

    if (ad < 0 || ad > max_page_aligned) {
        klog("pfree: failed to free memory");
        return;
    }

    for (int i = 0; i < pages; i++)
        pheap_desc[ad + i] = 0;
    return;
}

char* MemoryManager::kmalloc(size_t size)
{
    if ((!size) || (size > heap_end - heap_begin - memory_used)) {
        klog("kmalloc: invalid allocation amount");
        return 0;
    }

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
        klog("kmalloc: no more memory");
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

void MemoryManager::kfree(void* mem)
{
    alloc_t* alloc = (alloc_t*)((size_t)mem - sizeof(alloc_t));
    memory_used -= alloc->size + sizeof(alloc_t);
    alloc->status = 0;
}

void* operator new(size_t size)
{
    return MemoryManager::active->kmalloc(size);
}

void* operator new[](size_t size)
{
    return MemoryManager::active->kmalloc(size);
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
    MemoryManager::active->kfree(ptr);
}

void operator delete[](void* ptr)
{
    MemoryManager::active->kfree(ptr);
}
