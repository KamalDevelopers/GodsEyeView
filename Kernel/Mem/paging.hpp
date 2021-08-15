#ifndef PAGING_HPP
#define PAGING_HPP

#include "LibC/stdio.hpp"
#include "LibC/types.hpp"

namespace Paging {
extern void map_page(uint32_t virt, uint32_t phys);
extern void copy_page_directory(uint32_t* destination);
extern void switch_page_directory(uint32_t* page_dir);
extern void enable();
extern void init();
}

#endif
