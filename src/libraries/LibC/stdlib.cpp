#include "stdlib.hpp"

unsigned rand(unsigned int seed = 918, unsigned int max = 36000)
{
    seed = seed * 1103515245 + 12345;
    return ((unsigned)(seed / ((max + 1) * 2)) % (max + 1));
}

unsigned int random(unsigned int seed, unsigned int max)
{
    return rand(seed, max);
}

typedef struct free_block {
    int size;
    struct free_block* next;
} free_block;

static free_block free_block_list_head = { 0, 0 };
static const int align_to = 16;
static int current_break;
void* sbrk(int incr)
{
    int old_break = current_break;
    current_break += incr;
    return (void*)old_break;
}

void* malloc(int size)
{
    size = (size + sizeof(free_block) + (align_to - 1)) & ~(align_to - 1);
    free_block* block = free_block_list_head.next;
    free_block** head = &(free_block_list_head.next);
    while (block != 0) {
        if (block->size >= size) {
            *head = block->next;
            return ((char*)block) + sizeof(free_block);
        }
        head = &(block->next);
        block = block->next;
    }

    block = (free_block*)sbrk(size);
    block->size = size;

    return ((char*)block) + sizeof(free_block);
}

void free(void* ptr)
{
    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}

void* memcpy(void* dst, const void* src, unsigned int cnt)
{
    char *pszDest = (char*)dst;
    const char *pszSource = (const char*)src;
    if((pszDest != NULL) && (pszSource != NULL))
    {
        while(cnt)
        {
            *(pszDest++) = *(pszSource++);
            --cnt;
        }
    }
    return dst;
}

void* memset(void *b, char c, int len)
{
    char *b_char = (char *)b;
    while(*b_char && len > 0)
    {
        *b_char = c;
        b_char++;
        len--;
    }
    return b;
}