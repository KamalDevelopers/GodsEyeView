#include "mm.hpp"

void mm_init()
{
    pheap_end = 0x400000;
    pheap_begin = pheap_end - (MAX_ALLOC_PAGES * 4096);
}

/* Allocate a page */
void* pmalloc(size_t size)
{
    if (size > 4096)
        return 0;

    /* Page alloc */
    for (int i = 0; i < MAX_ALLOC_PAGES; i++) {
        if (pheap_desc[i])
            continue;
        pheap_desc[i] = 1;
        return (void*)(pheap_begin + i * 4096);
    }
    klog("pmalloc Error");
    return 0;
}

void pfree(void* mem)
{
    /* Determine which page */
    uint32_t ad = (uint32_t)mem;
    ad -= pheap_begin;
    ad /= 4096;
    /* Check if ad is out of range */
    if (ad < 0 || ad > MAX_ALLOC_PAGES)
        return;
    /* Now, ad has the id of the page */
    pheap_desc[ad] = 0;
    return;
}
