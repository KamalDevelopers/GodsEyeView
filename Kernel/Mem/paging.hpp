#ifndef PAGING_HPP
#define PAGING_HPP

#include "LibC/stdio.hpp"
#include "LibC/types.hpp"

namespace Paging {
extern void p_map_page(uint32_t virt, uint32_t phys);
extern void p_copy_page_directory(uint32_t* destination);
extern void p_switch_page_directory(uint32_t* page_dir);
extern void p_enable();
extern void p_init();
}

#endif
