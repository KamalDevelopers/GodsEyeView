#include "paging.hpp"

static uint32_t* page_directory = 0;
static uint32_t page_dir_loc = 0;
static uint32_t* last_page = 0;

/* Map page, virtual to physical */
void Paging::map_page(uint32_t virt, uint32_t phys)
{
    uint16_t id = virt >> 22;
    for (int i = 0; i < 1024; i++) {
        last_page[i] = phys | 3;
        phys += 4096;
    }
    page_directory[id] = ((uint32_t)last_page) | 3;
    last_page = (uint32_t*)(((uint32_t)last_page) + 4096);
}

void Paging::copy_page_directory(uint32_t* destination)
{
    // FIXME
    for (int i = 0; i < 1024; i++) {
        destination[i] = page_directory[i];
    }
}

void Paging::switch_page_directory(uint32_t* page_dir)
{
    page_directory = (uint32_t*)0x400000;
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = page_dir[i];
    }
    page_dir_loc = (uint32_t)page_directory;
    enable();
}

void Paging::enable()
{
    /* Put page directory into CR3 */
    asm volatile("mov %%eax, %%cr3"
                 :
                 : "a"(page_dir_loc));
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}

void Paging::init()
{
    page_directory = (uint32_t*)0x400000;
    page_dir_loc = (uint32_t)page_directory;
    last_page = (uint32_t*)0x404000;
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | 2;
    }
    Paging::map_page(0, 0);
    Paging::map_page(0x400000, 0x400000);
    Paging::enable();
}
