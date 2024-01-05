#include "pmm.hpp"
#include "../panic.hpp"

static BitArray<MAX_SLABS0> slab0_bitmap;
static BitArray<MAX_SLABS1> slab1_bitmap;
static BitArray<MAX_SLABS2> slab2_bitmap;

PhysicalMemoryManager* PhysicalMemoryManager::active = 0;
PhysicalMemoryManager::PhysicalMemoryManager(uint32_t pages)
{
    active = this;
    used_pages = 0;
    available_pages = pages;

    slab0_size = available_pages * 70 / 100;
    slab1_size = (available_pages * 15 / 100) / 15;
    slab2_size = (available_pages * 15 / 100) / 250;

    slab0_bitmap.bit_clear_range(0, MAX_SLABS0);
    slab1_bitmap.bit_clear_range(0, MAX_SLABS1);
    slab2_bitmap.bit_clear_range(0, MAX_SLABS2);

    slab0_ptr = PHYSICAL_MEMORY_START;
    slab1_ptr = slab0_ptr + slab0_size * SLAB0;
    slab2_ptr = slab1_ptr + slab1_size * SLAB1;
}

PhysicalMemoryManager::~PhysicalMemoryManager()
{
}

void PhysicalMemoryManager::stat(uint32_t* free_p, uint32_t* used_p)
{
    if (debug) {
        klog("PMM: total memory: %d MB (%d pages)\n", (available_pages * PAGE_SIZE) / MB, available_pages);
        klog("PMM: used memory: %d MB (%d pages)\n", (used_pages * PAGE_SIZE) / MB, used_pages);
    }

    *free_p = available_pages - used_pages;
    *used_p = used_pages;
}

uint32_t PhysicalMemoryManager::allocate_slab0(size_t size)
{
    uint32_t slabs = size / SLAB0;
    if ((size % SLAB0) != 0)
        ++slabs;
    uint32_t index = slab0_bitmap.fast_find_unset32(slabs);
    slab0_bitmap.bit_modify_range(index, slabs, 1);
    uint32_t address = slab0_ptr + index * SLAB0;
    if (debug)
        klog("PMM: allocating (%d) SLABS0 at [0x%x i: %d] -> [0x%x i: %d]\n", slabs, address, index, address + size, index + slabs);
    if ((index + slabs) > slab0_bitmap.size())
        return 0;
    return address;
}

uint32_t PhysicalMemoryManager::allocate_slab1(size_t size)
{
    uint32_t slabs = size / SLAB1;
    if ((size % SLAB1) != 0)
        ++slabs;
    uint32_t index = slab1_bitmap.fast_find_unset32(slabs);
    slab1_bitmap.bit_modify_range(index, slabs, 1);
    uint32_t address = slab1_ptr + index * SLAB1;
    if (debug)
        klog("PMM: allocating (%d) SLABS1 at [0x%x i: %d] -> [0x%x i: %d]\n", slabs, address, index, address + size, index + slabs);
    if ((index + slabs) > slab1_bitmap.size())
        return 0;
    return address;
}

uint32_t PhysicalMemoryManager::allocate_slab2(size_t size)
{
    uint32_t slabs = size / SLAB2;
    if ((size % SLAB2) != 0)
        ++slabs;
    uint32_t index = slab2_bitmap.fast_find_unset32(slabs);
    slab2_bitmap.bit_modify_range(index, slabs, 1);
    uint32_t address = slab2_ptr + index * SLAB2;
    if (debug)
        klog("PMM: allocating size=%d (%d) SLABS2 at [0x%x i: %d] -> [0x%x i: %d]\n", size, slabs, address, index, address + size, index + slabs);
    if ((index + slabs) > slab2_bitmap.size())
        return 0;
    return address;
}

uint32_t PhysicalMemoryManager::allocate_pages(size_t size)
{
    bool use_slab0 = (size <= SLAB0 * 10);
    bool use_slab1 = (!use_slab0 && size <= SLAB1 * 20);
    bool use_slab2 = !use_slab1;

    if (!IS_ALIGN(size))
        size = PAGE_ALIGN(size);
    uint32_t pages = size / 4096;
    uint32_t address = 0;

    if (use_slab0)
        address = allocate_slab0(size);
    else if (use_slab1)
        address = allocate_slab1(size);
    else if (use_slab2)
        address = allocate_slab2(size);

    if (!address) {
        klog("PMM: out of memory! [%d]/[%d]", available_pages, used_pages);
        return 0;
    }

    uint32_t map_address = address;
    for (uint32_t i = 0; i < pages; i++) {
        Paging::map_page(map_address, map_address);
        map_address += PAGE_SIZE;
        used_pages++;
    }

    return address;
}

int PhysicalMemoryManager::free_pages(uint32_t address, size_t size)
{
    int is_valid = 0;
    if (!IS_ALIGN(size))
        size = PAGE_ALIGN(size);

    if (!IS_ALIGN(address))
        address = PAGE_ALIGN_DOWN(address);

    bool use_slab0 = (size <= SLAB0 * 10);
    bool use_slab1 = (!use_slab0 && size <= SLAB1 * 20);
    bool use_slab2 = !use_slab1;

    uint32_t virtual_address = address;
    address = Paging::virtual_to_physical(address);
    uint32_t pages = size / PAGE_SIZE;
    uint32_t slabs = 0;
    uint32_t index = 0;

    if (use_slab0) {
        index = (address - slab0_ptr) / SLAB0;
        slabs = size / SLAB0;
        if ((size % SLAB0) != 0)
            ++slabs;
        slab0_bitmap.bit_clear_range(index, slabs);
    }

    else if (use_slab1) {
        index = (address - slab1_ptr) / SLAB1;
        slabs = size / SLAB1;
        if ((size % SLAB1) != 0)
            ++slabs;
        slab1_bitmap.bit_clear_range(index, slabs);
    }

    else if (use_slab2) {
        index = (address - slab2_ptr) / SLAB2;
        slabs = size / SLAB2;
        if ((size % SLAB2) != 0)
            ++slabs;
        slab2_bitmap.bit_clear_range(index, slabs);
    }

    if (debug)
        klog("PMM: freeing (%d) pages at [0x%x i: %d] -> [0x%x i: %d]\n", pages, address, index, address + size, index + pages);

    uint32_t map_address = virtual_address;
    for (uint32_t i = 0; i < pages; i++) {
        used_pages--;
        if (Paging::unmap_page(map_address) < 0)
            is_valid = -1;
        map_address += PAGE_SIZE;
    }

    return is_valid;
}
