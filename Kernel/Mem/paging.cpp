#include "paging.hpp"
#include "../panic.hpp"
#include "pmm.hpp"

Paging::page_directory_t* kernel_page_directory;
static bool is_paging_enabled = false;

uint32_t Paging::virtual_to_physical(uint32_t virtual_address)
{
    if (!is_paging_enabled)
        return virtual_address;

    uint32_t page_directory_index = PAGEDIR_INDEX(virtual_address);
    uint32_t page_table_index = PAGETBL_INDEX(virtual_address);
    uint32_t page_frame_offset = PAGEFRAME_INDEX(virtual_address);
    page_table_t* table = kernel_page_directory->reference_tables[page_directory_index];
    uint32_t address = table->pages[page_table_index].frame;
    address = (address << 12) + page_frame_offset;
    return address;
}

inline void flush_tlb_single(unsigned long addr)
{
    asm volatile("invlpg (%0)" ::"r"(addr)
                 : "memory");
}

void Paging::map_page(uint32_t virtual_address, uint32_t physical_address)
{
    if (!is_paging_enabled)
        return;

    uint32_t directory_index = PAGEDIR_INDEX(virtual_address);
    uint32_t table_index = PAGETBL_INDEX(virtual_address);

    page_table_t* table = (page_table_t*)((directory_index * sizeof(page_table_t)) + (uint32_t)kernel_page_directory + sizeof(page_directory_t) + 0x2000);

    kernel_page_directory->tables[directory_index].present = 1;
    kernel_page_directory->tables[directory_index].rw = 1;
    kernel_page_directory->tables[directory_index].user = 1;
    kernel_page_directory->tables[directory_index].w_through = 0;
    kernel_page_directory->tables[directory_index].cache = 0;
    kernel_page_directory->tables[directory_index].access = 0;
    kernel_page_directory->tables[directory_index].reserved = 0;
    kernel_page_directory->tables[directory_index].page_size = 0;
    kernel_page_directory->tables[directory_index].global = 0;
    kernel_page_directory->tables[directory_index].available = 0;
    kernel_page_directory->tables[directory_index].frame = (uint32_t)table >> 12;
    kernel_page_directory->reference_tables[directory_index] = table;

    table->pages[table_index].present = 1;
    table->pages[table_index].rw = 1;
    table->pages[table_index].user = 1;
    table->pages[table_index].reserved = 0;
    table->pages[table_index].accessed = 0;
    table->pages[table_index].dirty = 0;
    table->pages[table_index].reserved2 = 0;
    table->pages[table_index].available = 0;
    table->pages[table_index].frame = physical_address >> 12;
    flush_tlb_single(virtual_address);
}

int Paging::unmap_page(uint32_t virtual_address)
{
    if (!is_paging_enabled)
        return 0;

    uint32_t directory_index = PAGEDIR_INDEX(virtual_address);
    uint32_t table_index = PAGETBL_INDEX(virtual_address);
    page_table_t* table = kernel_page_directory->reference_tables[directory_index];

    if (!table) {
        klog("Could not unmap page! [0x%x] (no table entry)", virtual_address);
        return -1;
    }

    if (!table->pages[table_index].present) {
        klog("Could not unmap page! [0x%x] (not present)", virtual_address);
        return -1;
    }

    table->pages[table_index].rw = 0;
    table->pages[table_index].present = 0;
    table->pages[table_index].frame = 0;
    return 0;
}

void Paging::enable()
{
    asm volatile("mov %%eax, %%cr3"
                 :
                 : "a"((uint32_t)kernel_page_directory));
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}

void Paging::init()
{
    /* kernel_page_directory = (page_directory_t*)KERNEL_PAGE_DIR_START; */
    kernel_page_directory = (page_directory_t*)PMM->allocate_pages(PAGE_ALIGN(5 * MB));
    memset(kernel_page_directory, 0, PAGE_ALIGN(5 * MB));
    is_paging_enabled = true;

    uint32_t page = 0;
    uint32_t page_max = PAGE_SIZE * 1024 * 500;
    while (page < (page_max)) {
        Paging::map_page(page, page);
        page += PAGE_SIZE;
    }

    Paging::enable();
}
