#ifndef LIBALLOC_HPP
#define LIBALLOC_HPP

#include "types.hpp"

extern void memory_hooks(uint32_t (*pmalloc)(size_t), int (*pfree)(uint32_t, size_t));
static uint32_t (*hpmalloc)(size_t) = 0;
static int (*hpfree)(uint32_t, size_t) = 0;

static int l_initialized;

struct boundary_tag {
    unsigned int magic;     //< It's a kind of ...
    unsigned int size;      //< Requested size.
    unsigned int real_size; //< Actual size.
    int index;              //< Location in the page table.

    struct boundary_tag* split_left;  //< Linked-list info for broken pages.
    struct boundary_tag* split_right; //< The same.

    struct boundary_tag* next; //< Linked list info.
    struct boundary_tag* prev; //< Linked list info.
};

void* malloc(size_t);         //< The standard function.
void* realloc(void*, size_t); //< The standard function.
void* calloc(size_t, size_t); //< The standard function.
void free(void*);             //< The standard function.

#endif
