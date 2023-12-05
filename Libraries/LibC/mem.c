#include "mem.h"

void* memmove(void* dst, const void* src, size_t cnt)
{
    void* temp = dst;
    if ((uintptr_t)dst <= (uintptr_t)src || (uintptr_t)dst >= (uintptr_t)src + cnt) {
        asm volatile (
            "rep movsb"
            :"=D"(dst),"=S"(src),"=c"(cnt)
            :"0"(dst),"1"(src),"2"(cnt)
            :"memory"
        );
    }
    else {
        asm volatile (
            "std\n\t"
            "rep movsb\n\t"
            "cld"
            :"=D"(dst),"=S"(src),"=c"(cnt)
            :"0"((uintptr_t)dst + cnt - 1),"1"((uintptr_t)src + cnt - 1),"2"(cnt)
            :"memory"
        );
    }
    return temp;
}


int memcmp(const void* buf1, const void* buf2, size_t count)
{
    if (!count)
        return(0);

    while (--count && *(char*)buf1 == *(char*)buf2 ) {
        buf1 = (char*)buf1 + 1;
        buf2 = (char*)buf2 + 1;
    }

    return (*((unsigned char*)buf1) - *((unsigned char*)buf2));
}

void* memchr(const void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    while (n--)
        if (*p != (unsigned char)c)
            p++;
        else
            return p;
    return 0;
}

void* memcpy(void* d, const void* s, uint32_t c)
{
    void* temp = d;
    asm volatile (
        "rep movsb"
        :"=D"(d),"=S"(s),"=c"(c)
        :"0"(d),"1"(s),"2"(c)
        :"memory"
    );
    return temp;
}

void* memcpy32(void* dst, const void* src, size_t cnt)
{
    uint32_t num_dwords = cnt / 4;
    uint32_t num_bytes = cnt % 4;
    uint32_t* dest32 = (uint32_t*)dst;
    uint32_t* src32 = (uint32_t*)src;
    uint8_t* dest8 = ((uint8_t*)dst) + num_dwords * 4;
    uint8_t* src8 = ((uint8_t*)src) + num_dwords * 4;
    uint32_t i;

    for (i = 0; i < num_dwords; i++)
        dest32[i] = src32[i];
    for (i = 0; i < num_bytes; i++)
        dest8[i] = src8[i];
    return dst;
}

void* memset(void* d, int s, size_t c)
{
    void * temp = d;
    asm volatile (
        "rep stosb"
        :"=D"(d),"=c"(c)
        :"0"(d),"a"(s),"1"(c)
        :"memory"
    );
    return temp;
}

inline void* palloc(int size)
{
    char* p2;

    asm("int $0x80"
        : "=a"(p2)
        : "a"(90), "b"(0), "c"(size));

    return p2;
}

inline int pfree(void* ptr, int size)
{
    asm("int $0x80"
        :
        : "a"(91), "b"(ptr), "c"(size));
    return 0;
}


/* STMP8_MAX = PAGE_SIZE * 126 */
#define STMP8_MAX 516096
#define PAGE_SIZE 4096
#define MAX_BUFFER_POOLS 10

typedef struct pool {
    void* address;
    uint8_t is_open;
} pool_t;

typedef struct major {
    uint8_t open_pools[11];
    int16_t free_pool[11];
    uint8_t cached_open_pools[11];
    pool_t pools[11][126];
} major_t;

static major_t major;

void* sh_malloc_stamp(uint32_t size)
{
    uint8_t is_big = (size > STMP8_MAX);
    uint32_t total_size = size + is_big * is_big + 1;

    if ((total_size % PAGE_SIZE) != 0)
        total_size = (total_size & ~(PAGE_SIZE - 1)) + 4096;

    uint8_t* base = (uint8_t*)palloc(total_size);
    if (is_big) {
        uint16_t pages = (total_size >> 12) - 127;
        ((uint8_t*)base)[2] = 255;
        ((uint16_t*)base)[0] = pages;
    } else {
        uint8_t stamp = (total_size >> 12) << 1;
        stamp |= 1;
        ((uint8_t*)base)[0] = stamp;
    }

    return base + 1 + is_big + is_big;
}

void sh_free_stamp(const void* addr)
{
    uint8_t* address = (uint8_t*)addr;
    address--;
    uint8_t small_stamp = ((uint8_t*)address)[0];
    uint16_t big_stamp = 0;
    small_stamp = small_stamp >> 1;

    if (small_stamp == 127) {
        big_stamp = ((uint16_t*)(address - 2))[0] + 127;
        address -= 2;
    }

    uint16_t stamp = (!big_stamp) ? small_stamp : big_stamp;
    pfree(address, stamp * 4096);
}

int8_t find_open_pool(uint32_t chunk_size_index, uint8_t* pool_index)
{
    /* With cache */
    uint8_t cached_open = major.cached_open_pools[chunk_size_index];
    if (major.pools[chunk_size_index][cached_open].is_open) {
        *pool_index = cached_open;
        return 0;
    }

    /* Without cache */
    for (*pool_index = 0; *pool_index < 126; ++(*pool_index)) {
        if (major.pools[chunk_size_index][*pool_index].is_open) {
            major.cached_open_pools[chunk_size_index] = *pool_index;
            return 0;
        }
    }

    /* Not found */
    *pool_index = 0;
    return 1;
}

int8_t find_open_pool_pos(pool_t* pool, uint32_t chunk_size, uint32_t* index_in_pool)
{
    uint32_t start = 0;
    uint32_t i = 0;

    /* Scan pool for free spaces */
    while (start < 4096) {
        if (!((uint8_t*)(pool->address) + start)[0]) {
            *index_in_pool = i;
            return 0;
        }
        start += chunk_size + 1;
        i++;
    }
    return 1;
}

int16_t find_free_pool(uint32_t chunk_size_index)
{
    for (uint16_t i = 0; i < 126; ++i)
        if (!major.pools[chunk_size_index][i].address)
            return i;
    return -1;
}

void sh_free_pool(const void* address)
{
    /* Determine pool number */
    uint8_t stamp = ((uint8_t*)address - 1)[0];
    uint8_t pool = (stamp >> 1) - 1;
    int range;

    /* Determine chunk size index */
    int8_t chunk_size_index = -1;
    for (uint8_t i = 0; i < 11; ++i) {
        range = (uint8_t*)address - (uint8_t*)major.pools[i][pool].address;
        if (range > 0 && range < 4096) {
            chunk_size_index = i;
            break;
        }
    }
    if (chunk_size_index < 0)
        return;

    *((uint8_t*)address - 1) = 0;

    /* Case 1: pool was full but is re-opened */
    if (!major.pools[chunk_size_index][pool].is_open) {
        major.pools[chunk_size_index][pool].is_open = 1;
        major.open_pools[chunk_size_index]++;
    }

    /* Case 2: when pool index > MAX_BUFFER_POOLS
     * last chunk free'd, delete pool */
    if (range == 1 && pool > MAX_BUFFER_POOLS)  {
        pfree(major.pools[chunk_size_index][pool].address, 4096);
        major.free_pool[chunk_size_index] = pool;
        major.pools[chunk_size_index][pool].is_open = 0;
        major.pools[chunk_size_index][pool].address = 0;
        major.open_pools[chunk_size_index] -= 1;
    }
}

void* sh_malloc_pool(uint32_t size)
{
    uint32_t chunk_size = size;
    chunk_size |= chunk_size >> 1;
    chunk_size |= chunk_size >> 2;
    chunk_size |= chunk_size >> 4;
    chunk_size |= chunk_size >> 8;
    chunk_size |= chunk_size >> 16;
    chunk_size++;
    uint8_t chunk_size_index = __builtin_ffs(chunk_size) - 1;

    /* Check for pool */
    uint8_t pool;
    int8_t do_create_pool = 1;
    if (major.open_pools[chunk_size_index])
        do_create_pool = find_open_pool(chunk_size_index, &pool);

    /* Create new pool, slow? */
    if (do_create_pool) {
        if (major.free_pool[chunk_size_index] < 0)
            return sh_malloc_stamp(size); // plan b

        uint8_t* address = (uint8_t*)palloc(4096);
        for (int16_t i = 0; i < 4096; i += chunk_size + 1)
            ((uint8_t*)address)[i] = 0;

        major.pools[chunk_size_index][major.free_pool[chunk_size_index]].address = address;
        major.pools[chunk_size_index][major.free_pool[chunk_size_index]].is_open = 1;
        major.open_pools[chunk_size_index] += 1;
        uint8_t stamp = ((major.free_pool[chunk_size_index] + 1) << 1) & ~(1 << 0);
        ((uint8_t*)address)[0] = stamp;

        major.free_pool[chunk_size_index] = find_free_pool(chunk_size_index);
        return address + 1;
    }

    uint32_t index_in_pool;
    pool_t* p = &major.pools[chunk_size_index][pool];
    int8_t no_position = find_open_pool_pos(p, chunk_size, &index_in_pool);

    if (!no_position) {
        uint8_t stamp = ((pool + 1) << 1) & ~(1 << 0);
        uint8_t* address = ((uint8_t*)p->address) + index_in_pool * (chunk_size + 1);
        ((uint8_t*)address)[0] = stamp;

        if ((address + 1 + chunk_size) > (((uint8_t*)p->address) + 4096)) {
            p->is_open = 0;
            major.open_pools[chunk_size_index]--;
        }
        return address + 1;
    }

    /* Assert not reached! */
    return 0;
}

void sh_free(const void* address)
{
    if (!address)
        return;
    uint8_t stamp = ((uint8_t*)address - 1)[0];
    if (stamp & (1 << 0))
        return sh_free_stamp(address);
    return sh_free_pool(address);
}

void* sh_malloc(size_t size)
{
    if (!size)
        return 0;
    if (size <= 1024)
        return sh_malloc_pool(size);
    return sh_malloc_stamp(size);
}

void* malloc(size_t size)
{
    return sh_malloc(size);
}

void free(const void* address)
{
    return sh_free(address);
}

void* calloc(size_t count, size_t size)
{
  if (count == 0 || size == 0)
      return 0;

    void* p = malloc(count * size);
    memset(p, 0, size * count);
    return p;
}
