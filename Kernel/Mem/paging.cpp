#include "paging.hpp"

Paging::page_directory_t* kernel_page_directory;

void Paging::map_page(uint32_t virtual_address, uint32_t physical_address)
{
    uint32_t directory_index = PAGEDIR_INDEX(virtual_address);
    uint32_t table_index = PAGETBL_INDEX(virtual_address);

    page_table_t* table = (page_table_t*)((directory_index * sizeof(page_table_t)) + KERNEL_PAGE_DIR_START + sizeof(page_directory_t) + 0x2000);

    kernel_page_directory->tables[directory_index].frame = (uint32_t)table >> 12;
    kernel_page_directory->tables[directory_index].present = 1;
    kernel_page_directory->tables[directory_index].rw = 1;
    kernel_page_directory->tables[directory_index].user = 1;
    kernel_page_directory->tables[directory_index].page_size = 0;
    kernel_page_directory->reference_tables[directory_index] = table;

    table->pages[table_index].frame = physical_address >> 12;
    table->pages[table_index].present = 1;
    table->pages[table_index].rw = 1;
    table->pages[table_index].user = 1;
}

int Paging::unmap_page(uint32_t virtual_address)
{
    uint32_t directory_index = PAGEDIR_INDEX(virtual_address);
    uint32_t table_index = PAGETBL_INDEX(virtual_address);

    if (!kernel_page_directory->reference_tables[directory_index]) {
        klog("Could not unmap page! [0x%x] (no table entry)", virtual_address);
        return -1;
    }

    page_table_t* table = kernel_page_directory->reference_tables[directory_index];
    if (!table->pages[table_index].present) {
        klog("Could not unmap page! [0x%x] (not present)", virtual_address);
        return -1;
    }

    table->pages[table_index].present = 0;
    table->pages[table_index].frame = 0;
    return 0;
}

void Paging::enable()
{
    asm volatile("mov %%eax, %%cr3"
                 :
                 : "a"(KERNEL_PAGE_DIR_START));
    asm volatile("mov %cr0, %eax");
    asm volatile("orl $0x80000000, %eax");
    asm volatile("mov %eax, %cr0");
}

void Paging::init()
{
    kernel_page_directory = (page_directory_t*)KERNEL_PAGE_DIR_START;

    uint32_t page = 0;
    while (page < (PAGE_SIZE * 1024 * 3)) {
        Paging::map_page(page, page);
        page += PAGE_SIZE;
    }

    Paging::enable();
}
