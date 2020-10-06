#ifndef MM_HPP
#define MM_HPP

#include "stdlib.hpp"

#define MAX_ALLOC_PAGES 32

static uint32_t pheap_begin = 0;
static uint32_t pheap_end = 0;
static uint8_t* pheap_desc = 0;

extern void pfree(void* mem);
extern void* pmalloc(size_t size);
extern void mm_init();

#endif
