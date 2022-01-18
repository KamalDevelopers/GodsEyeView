#ifndef PAGING_HPP
#define PAGING_HPP

#include "../tty.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "mm.hpp"
#include "pmm.hpp"

namespace Paging {
#define PAGE_SIZE 4096

#define IS_ALIGN(addr) ((((uint32_t)(addr)) % PAGE_SIZE) == 0)
#define PAGE_ALIGN(addr) ((((uint32_t)(addr)) & 0xFFFFF000) + 0x1000)
#define PAGE_ALIGN_DOWN(addr) ((((uint32_t)(addr)) & 0xFFFFF000))

#define PAGEDIR_INDEX(vaddr) (((uint32_t)vaddr) >> 22)
#define PAGETBL_INDEX(vaddr) ((((uint32_t)vaddr) >> 12) & 0x3ff)
#define PAGEFRAME_INDEX(vaddr) (((uint32_t)vaddr) & 0xfff)

#define KERNEL_PAGE_DIR_START 0x400000
#define ERR_PRESENT 0x1
#define ERR_RW 0x2
#define ERR_USER 0x4
#define ERR_RESERVED 0x8
#define ERR_INST 0x10
#define MB 1000000

typedef struct __attribute__((packed)) page_dir_entry {
    unsigned int present : 1;
    unsigned int rw : 1;
    unsigned int user : 1;
    unsigned int w_through : 1;
    unsigned int cache : 1;
    unsigned int access : 1;
    unsigned int reserved : 1;
    unsigned int page_size : 1;
    unsigned int global : 1;
    unsigned int available : 3;
    unsigned int frame : 20;
} page_dir_entry_t;

typedef struct page_table_entry {
    unsigned int present : 1;
    unsigned int rw : 1;
    unsigned int user : 1;
    unsigned int reserved : 2;
    unsigned int accessed : 1;
    unsigned int dirty : 1;
    unsigned int reserved2 : 2;
    unsigned int available : 3;
    unsigned int frame : 20;
} page_table_entry_t;

typedef struct __attribute__((packed)) page_table {
    page_table_entry_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_dir_entry_t tables[1024];
    page_table_t* reference_tables[1024];
} page_directory_t;

void map_page(uint32_t virtual_address, uint32_t physical_address);
int unmap_page(uint32_t virtual_address);
void enable();
void init();
}

#endif
