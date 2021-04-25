#ifndef MM_HPP
#define MM_HPP

#include "../tty.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "paging.hpp"

#define MM MemoryManager::active
#define MB 1000000
#define KiB 1024

typedef struct {
    uint8_t status;
    uint32_t size;
} alloc_t;

class MemoryManager {
protected:
    uint32_t last_alloc;
    uint32_t heap_end;
    uint32_t heap_begin;
    uint32_t pheap_begin;
    uint32_t pheap_end;
    uint8_t* pheap_desc;
    uint32_t max_page_aligned;
    uint32_t memory_used;

public:
    static MemoryManager* active;

    MemoryManager(uint32_t kernel_end, uint32_t phys);
    ~MemoryManager();

    void dump();
    char* pmalloc(size_t size);
    char* kmalloc(size_t size);
    void pfree(void* mem, size_t size);
    void kfree(void* mem);
};

inline char* kmalloc(size_t size)
{
    return MM->kmalloc(size);
}

inline char* pmalloc(size_t size)
{
    return MM->pmalloc(size);
}

inline void kfree(void* mem)
{
    return MM->kfree(mem);
}

inline void pfree(void* mem, size_t size)
{
    return MM->pfree(mem, size);
}

void* operator new(size_t size);
void* operator new[](size_t size);

void* operator new(size_t size, void* ptr);
void* operator new[](size_t size, void* ptr);

void operator delete(void* ptr);
void operator delete[](void* ptr);

#endif
