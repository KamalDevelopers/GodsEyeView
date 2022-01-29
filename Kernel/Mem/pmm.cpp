#include "pmm.hpp"
#include "../panic.hpp"

PhysicalMemoryManager* PhysicalMemoryManager::active = 0;
PhysicalMemoryManager::PhysicalMemoryManager(uint32_t pages)
{
    active = this;
    used_pages = 0;
    available_pages = pages;
    if (available_pages >= MAX_PAGES)
        available_pages = MAX_PAGES - 1;
}

PhysicalMemoryManager::~PhysicalMemoryManager()
{
}

void PhysicalMemoryManager::info()
{
    kprintf("total memory: %d MB (%d pages)\n", (available_pages * PAGE_SIZE) / MB, available_pages);
    kprintf("used memory: %d MB (%d pages)\n", (used_pages * PAGE_SIZE) / MB, used_pages);
}

uint32_t PhysicalMemoryManager::allocate_pages(size_t size)
{
    if (!IS_ALIGN(size))
        size = PAGE_ALIGN(size);

    uint32_t pages = size / PAGE_SIZE;
    uint32_t index = page_bitmap.find_unset(pages);
    uint32_t address = PHYSICAL_MEMORY_START + index * PAGE_SIZE;

    if (debug)
        kprintf("allocating (%d) pages at [0x%x i: %d] -> [0x%x i: %d]\n", pages, address, index, address + size, index + pages);

    for (uint32_t i = 0; i < pages; i++) {
        uint32_t map_address = address + (i * PAGE_SIZE);
        if (page_bitmap.bit_get(index + i)) {
            klog("PMM: [0x%x i: %d], is already allocated!", map_address, index + i);
            PANIC("Out of memory");
        }

        used_pages++;
        Paging::map_page(map_address, map_address);
        page_bitmap.bit_set(index + i);
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

    uint32_t virtual_address = address;
    address = Paging::virtual_to_physical(address);
    uint32_t pages = size / PAGE_SIZE;
    uint32_t index = (address - PHYSICAL_MEMORY_START) / PAGE_SIZE;

    if (debug)
        kprintf("freeing (%d) pages at [0x%x i: %d] -> [0x%x i: %d]\n", pages, address, index, address + size, index + pages);

    for (uint32_t i = 0; i < pages; i++) {
        uint32_t map_address = virtual_address + (i * PAGE_SIZE);
        if (!page_bitmap.bit_get(index + i)) {
            klog("PMM: [0x%x i: %d], is already freed!", map_address, index + i);
            is_valid = -1;
        }

        used_pages--;
        Paging::unmap_page(map_address);
        page_bitmap.bit_clear(index + i);
    }

    return is_valid;
}
