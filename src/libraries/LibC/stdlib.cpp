#include "stdlib.hpp"

int atoi(char* array)
{
    int number = 0;
    int mult = 1;
    int n = str_len(array);

    n = (int)n < 0 ? -n : n;

    while (n--) {
        if ((array[n] < '0' || array[n] > '9') && array[n] != '-') {
            if (number)
                break;
            else
                continue;
        }

        if (array[n] == '-') {
            if (number) {
                number = -number;
                break;
            }
        } else {
            number += (array[n] - '0') * mult;
            mult *= 10;
        }
    }

    return number;
}

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

void* memcpy(void* __restrict dst, const void* __restrict src, size_t count)
{
    char* __restrict s = (char*)src;
    char* __restrict d = (char*)dst;

    while (count-- > 0)
        *s++ = *d++;

    return dst;
}