#ifndef PMM_HPP
#define PMM_HPP

#include "LibC/liballoc.hpp"
#include "LibC/stdlib.hpp"
#include "paging.hpp"

namespace PMM {
#define MAX_PAGES 20000
#define PHYSICAL_MEMORY_START PAGE_ALIGN(15 * MB)

void init();
void info();
uint32_t allocate_pages(size_t size);
int free_pages(uint32_t address, size_t size);
}

#endif
