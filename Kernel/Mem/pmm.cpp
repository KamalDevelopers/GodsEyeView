#include "pmm.hpp"

constexpr bool debug = false;
bool pages_bitmap[MAX_PAGES];
uint32_t available_pages;
uint32_t used_pages;

void PMM::init(uint32_t pages)
{
    used_pages = 0;
    available_pages = pages;
    if (available_pages > MAX_PAGES)
        available_pages = MAX_PAGES;
    memset(pages_bitmap, 0, MAX_PAGES);
}

void PMM::info()
{
    kprintf("total memory: %d MB (%d pages)\n", (available_pages * PAGE_SIZE) / MB, available_pages);
    kprintf("used memory: %d MB (%d pages)\n", (used_pages * PAGE_SIZE) / MB, used_pages);
}

uint32_t PMM::allocate_pages(size_t size)
{
    if (!IS_ALIGN(size))
        size = PAGE_ALIGN(size);

    uint32_t pages = size / PAGE_SIZE;
    uint32_t index = 0;

    while (index + pages < available_pages) {
        bool can_allocate = true;
        for (int i = 0; i < pages; i++) {
            if (pages_bitmap[index + i]) {
                can_allocate = false;
                index++;
            }
        }

        if (!can_allocate)
            continue;

        uint32_t address = PHYSICAL_MEMORY_START + index * PAGE_SIZE;
        if (debug)
            kprintf("allocated (%d) pages at [0x%x i: %d] -> [0x%x i: %d]\n", pages, address, index, address + size, index + pages);

        for (int i = 0; i < pages; i++) {
            used_pages++;
            if (pages_bitmap[index + i])
                klog("PMM: [0x%x i: %d], is already allocated!", (address + (i * PAGE_SIZE)), index + i);
            pages_bitmap[index + i] = true;
            Paging::map_page(address + (i * PAGE_SIZE), address + (i * PAGE_SIZE));
        }

        return address;
    }

    klog("PMM out of memory!");
    info();
    return 0;
}

int PMM::free_pages(uint32_t address, size_t size)
{
    int is_valid = 0;
    if (!IS_ALIGN(size))
        size = PAGE_ALIGN(size);

    uint32_t pages = size / PAGE_SIZE;
    address = PAGE_ALIGN_DOWN(address);
    uint32_t index = (address - PHYSICAL_MEMORY_START) / PAGE_SIZE;

    if (debug)
        kprintf("freeing (%d) pages at [0x%x i: %d] -> [0x%x i: %d]\n", pages, address, index, address + size, index + pages);

    for (int i = 0; i < pages; i++) {
        if (pages_bitmap[index + i] == false) {
            klog("PMM: [0x%x i: %d], is already freed!", (address + (i * PAGE_SIZE)), index + i);
            is_valid = -1;
            continue;
        }

        used_pages--;
        pages_bitmap[index + i] = false;
        Paging::unmap_page(address + (i * PAGE_SIZE));
    }

    return is_valid;
}
