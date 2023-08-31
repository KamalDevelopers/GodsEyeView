#ifndef PMM_HPP
#define PMM_HPP

#include "paging.hpp"
#include <LibC++/bitarray.hpp>
#include <LibC/mem.h>
#include <LibC/stdlib.h>

#define MAX_SLABS0 488280 /* 50% */
#define MAX_SLABS1 48828  /* 25% */
#define MAX_SLABS2 4882   /* 25% */

#define SLAB0 4096
#define SLAB1 61440
#define SLAB2 1024000

#define PHYSICAL_MEMORY_START PAGE_ALIGN(10 * MB)
#define PMM PhysicalMemoryManager::active

class PhysicalMemoryManager {
private:
    bool debug = false;
    uint32_t available_pages = 0;
    uint32_t used_pages = 0;

    uint32_t slab0_size;
    uint32_t slab1_size;
    uint32_t slab2_size;

    uint32_t slab0_ptr;
    uint32_t slab1_ptr;
    uint32_t slab2_ptr;

public:
    PhysicalMemoryManager(uint32_t pages);
    ~PhysicalMemoryManager();

    static PhysicalMemoryManager* active;

    void stat(uint32_t* free_pages, uint32_t* used_pages);
    uint32_t allocate_pages(size_t size);
    int free_pages(uint32_t address, size_t size);
    uint32_t allocate_slab0(size_t size);
    uint32_t allocate_slab1(size_t size);
    uint32_t allocate_slab2(size_t size);
};

#endif
