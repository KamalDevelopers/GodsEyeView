#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define rep_memcpy(to, from, n)                    \
    {                                              \
        unsigned long int dummy;                   \
        __asm__ __volatile__(                      \
            "rep; movsb"                           \
            : "=&D"(to), "=&S"(from), "=&c"(dummy) \
            : "0"(to), "1"(from), "2"(n)           \
            : "memory");                           \
    }

#define MIN_LEN 0x40
#define SSE_MMREG_SIZE 16
#define MMX_MMREG_SIZE 8
#define MMX1_MIN_LEN 0x800

/*  Currently a single thread can lock the sse at a time,
 *  thus not allowing threads to simultaneously use sse.
 *  However, it is possible for two threads to grab the sse lock,
 *  which will cause a fault. As it is now, only the display server
 *  has the userspace sse lock, since it is the most memcpy
 *  heavy userspace application. */

/* TODO: In the future we could move the sse lock to the kernel.
 * With this solution a syscall would be required to grab the sse lock.
 * Another solution, which might be more common, is to change the sse
 * stack at every task switch. I find this solution to0 expensive currently.
 * */

bool has_sse()
{
    return true;
}

void* sse2_memset8(void* s, char c, size_t n)
{
    size_t i = 0;
    if ((size_t)s & (SSE_MMREG_SIZE - 1)) {
        while ((((size_t)s + i) & (SSE_MMREG_SIZE - 1)) && i < n) {
            ((char*)s + i)[0] = c;
            i++;
        }
    }

    asm volatile("movd %%eax, %%xmm0\n"
                 "pshufd $(0x00), %%xmm0, %%xmm0"
                 :
                 : "a"((uint32_t)c));

    for (; i + 64 <= n; i += 64) {
        asm volatile(" movdqa %%xmm0, 0(%0);	"
                     " movdqa %%xmm0, 16(%0);	"
                     " movdqa %%xmm0, 32(%0);	"
                     " movdqa %%xmm0, 48(%0);	" ::"r"((size_t)s + i));
    }

    asm(" rep stosb; " ::"a"((size_t)(c)), "D"(((size_t)s) + i), "c"(n - i));
    i += n - i;
    return (void*)(((size_t)s) + i);
}

void* sse2_memset32(void* s, uint32_t c, size_t n)
{
    size_t i = 0;
    n *= 4;
    if ((size_t)s & (SSE_MMREG_SIZE - 1)) {
        while ((((size_t)s + i) & (SSE_MMREG_SIZE - 1)) && i < n) {
            memset32((void*)((size_t)s + i), c, 1);
            i += 4;
        }
    }

    asm volatile("movd %%eax, %%xmm0\n"
                 "pshufd $(0x00), %%xmm0, %%xmm0"
                 :
                 : "a"((uint32_t)c));

    for (; i + 64 <= n; i += 64) {
        asm volatile(" movdqa %%xmm0, 0(%0);	"
                     " movdqa %%xmm0, 16(%0);	"
                     " movdqa %%xmm0, 32(%0);	"
                     " movdqa %%xmm0, 48(%0);	" ::"r"((size_t)s + i));
    }

    memset((void*)((size_t)s + i), c, n - i);
    i += n - i;
    return (void*)(((size_t)s) + i);
}

void* sse_memcpy(void* to, const void* from, size_t len)
{
    void* retval;
    size_t i;
    retval = to;

    __asm__ __volatile__(
        "   prefetchnta (%0)\n"
        "   prefetchnta 64(%0)\n"
        "   prefetchnta 128(%0)\n"
        "   prefetchnta 192(%0)\n"
        "   prefetchnta 256(%0)\n"
        :
        : "r"(from));

    if (len >= MIN_LEN) {
        unsigned long int delta;
        /* Align destinition to MMREG_SIZE -boundary */
        delta = ((unsigned long int)to) & (SSE_MMREG_SIZE - 1);
        if (delta) {
            delta = SSE_MMREG_SIZE - delta;
            len -= delta;
            rep_memcpy(to, from, delta);
        }
        i = len >> 6; /* len/64 */
        len &= 63;
        if (((unsigned long)from) & 15)
            /* if SRC is misaligned */
            for (; i > 0; i--) {
                __asm__ __volatile__(
                    "prefetchnta 320(%0)\n"
                    "movups (%0), %%xmm0\n"
                    "movups 16(%0), %%xmm1\n"
                    "movups 32(%0), %%xmm2\n"
                    "movups 48(%0), %%xmm3\n"
                    "movntps %%xmm0, (%1)\n"
                    "movntps %%xmm1, 16(%1)\n"
                    "movntps %%xmm2, 32(%1)\n"
                    "movntps %%xmm3, 48(%1)\n" ::"r"(from),
                    "r"(to)
                    : "memory");
                from = ((const unsigned char*)from) + 64;
                to = ((unsigned char*)to) + 64;
            }
        else
            /*
               Only if SRC is aligned on 16-byte boundary.
               It allows to use movaps instead of movups, which required
               data to be aligned or a general-protection exception (#GP)
               is generated.
            */
            for (; i > 0; i--) {
                __asm__ __volatile__(
                    "prefetchnta 320(%0)\n"
                    "movaps (%0), %%xmm0\n"
                    "movaps 16(%0), %%xmm1\n"
                    "movaps 32(%0), %%xmm2\n"
                    "movaps 48(%0), %%xmm3\n"
                    "movntps %%xmm0, (%1)\n"
                    "movntps %%xmm1, 16(%1)\n"
                    "movntps %%xmm2, 32(%1)\n"
                    "movntps %%xmm3, 48(%1)\n" ::"r"(from),
                    "r"(to)
                    : "memory");
                from = ((const unsigned char*)from) + 64;
                to = ((unsigned char*)to) + 64;
            }
        /* since movntq is weakly-ordered, a "sfence"
         * is needed to become ordered again. */
        __asm__ __volatile__("sfence" ::
                                 : "memory");
        /* enables to use FPU */
        __asm__ __volatile__("emms" ::
                                 : "memory");
    }
    /*
     * Now do the tail of the block
     */
    if (len)
        memcpy(to, from, len);
    return retval;
}

void* mmx_memcpy(void* to, const void* from, size_t len)
{
    void* retval;
    size_t i;
    retval = to;

    if (len >= MMX1_MIN_LEN) {
        unsigned long int delta;
        /* Align destinition to MMREG_SIZE -boundary */
        delta = ((unsigned long int)to) & (MMX_MMREG_SIZE - 1);
        if (delta) {
            delta = MMX_MMREG_SIZE - delta;
            len -= delta;
            rep_memcpy(to, from, delta);
        }
        i = len >> 6; /* len/64 */
        len &= 63;
        for (; i > 0; i--) {
            __asm__ __volatile__(
                "movq (%0), %%mm0\n"
                "movq 8(%0), %%mm1\n"
                "movq 16(%0), %%mm2\n"
                "movq 24(%0), %%mm3\n"
                "movq 32(%0), %%mm4\n"
                "movq 40(%0), %%mm5\n"
                "movq 48(%0), %%mm6\n"
                "movq 56(%0), %%mm7\n"
                "movq %%mm0, (%1)\n"
                "movq %%mm1, 8(%1)\n"
                "movq %%mm2, 16(%1)\n"
                "movq %%mm3, 24(%1)\n"
                "movq %%mm4, 32(%1)\n"
                "movq %%mm5, 40(%1)\n"
                "movq %%mm6, 48(%1)\n"
                "movq %%mm7, 56(%1)\n" ::"r"(from),
                "r"(to)
                : "memory");
            from = ((const unsigned char*)from) + 64;
            to = ((unsigned char*)to) + 64;
        }
        __asm__ __volatile__("emms" ::
                                 : "memory");
    }
    /*
     * Now do the tail of the block
     */
    if (len)
        memcpy(to, from, len);
    return retval;
}

void* sse2_memcpy(void* to, const void* from, size_t len)
{
    void* retval;
    size_t i;
    retval = to;

    __asm__ __volatile__(
        "   prefetchnta (%0)\n"
        "   prefetchnta 64(%0)\n"
        "   prefetchnta 128(%0)\n"
        "   prefetchnta 192(%0)\n"
        "   prefetchnta 256(%0)\n"
        /*
        "   prefetchnta 320(%0)\n"
        "   prefetchnta 384(%0)\n"
        "   prefetchnta 448(%0)\n"
        "   prefetchnta 512(%0)\n"
        */
        :
        : "r"(from));

    if (len >= MIN_LEN) {
        unsigned long int delta;
        /* Align destinition to MMREG_SIZE -boundary */
        delta = ((unsigned long int)to) & (SSE_MMREG_SIZE - 1);
        if (delta) {
            delta = SSE_MMREG_SIZE - delta;
            len -= delta;
            rep_memcpy(to, from, delta);
        }
        i = len >> 7; /* len/128 */
        len &= 127;
        if (((unsigned long)from) & 15)
            /* if SRC is misaligned */
            for (; i > 0; i--) {
                __asm__ __volatile__(
                    "prefetchnta 640(%0)\n"

                    "movdqu (%0), %%xmm0\n"
                    "movdqu 16(%0), %%xmm1\n"
                    "movdqu 32(%0), %%xmm2\n"
                    "movdqu 48(%0), %%xmm3\n"

                    "movntdq %%xmm0, (%1)\n"
                    "movntdq %%xmm1, 16(%1)\n"
                    "movntdq %%xmm2, 32(%1)\n"
                    "movntdq %%xmm3, 48(%1)\n"

                    "movdqu 64(%0), %%xmm4\n"
                    "movdqu 80(%0), %%xmm5\n"
                    "movdqu 96(%0), %%xmm6\n"
                    "movdqu 112(%0), %%xmm7\n"

                    "movntdq %%xmm4, 64(%1)\n"
                    "movntdq %%xmm5, 80(%1)\n"
                    "movntdq %%xmm6, 96(%1)\n"
                    "movntdq %%xmm7, 112(%1)\n" ::"r"(from),
                    "r"(to)
                    : "memory");
                from = ((const unsigned char*)from) + 128;
                to = ((unsigned char*)to) + 128;
            }
        else
            /*
               Only if SRC is aligned on 16-byte boundary.
               It allows to use movaps instead of movups, which required
               data to be aligned or a general-protection exception (#GP)
               is generated.
            */
            for (; i > 0; i--) {
                __asm__ __volatile__(
                    "prefetchnta 640(%0)\n"

                    "movapd (%0), %%xmm0\n"
                    "movapd 16(%0), %%xmm1\n"
                    "movapd 32(%0), %%xmm2\n"
                    "movapd 48(%0), %%xmm3\n"

                    "movntdq %%xmm0, (%1)\n"
                    "movntdq %%xmm1, 16(%1)\n"
                    "movntdq %%xmm2, 32(%1)\n"
                    "movntdq %%xmm3, 48(%1)\n"

                    "movapd 64(%0), %%xmm4\n"
                    "movapd 80(%0), %%xmm5\n"
                    "movapd 96(%0), %%xmm6\n"
                    "movapd 112(%0), %%xmm7\n"

                    "movntdq %%xmm4, 64(%1)\n"
                    "movntdq %%xmm5, 80(%1)\n"
                    "movntdq %%xmm6, 96(%1)\n"
                    "movntdq %%xmm7, 112(%1)\n" ::"r"(from),
                    "r"(to)
                    : "memory");
                from = ((const unsigned char*)from) + 128;
                to = ((unsigned char*)to) + 128;
            }
        /* since movntq is weakly-ordered, a "sfence"
         * is needed to become ordered again. */
        __asm__ __volatile__("mfence" ::
                                 : "memory");
        /* enables to use FPU */
        __asm__ __volatile__("emms" ::
                                 : "memory");
    }
    /*
     * Now do the tail of the block
     */
    if (len)
        memcpy(to, from, len);
    return retval;
}

void* memmove(void* dst, const void* src, size_t cnt)
{
    void* temp = dst;
    if ((uintptr_t)dst <= (uintptr_t)src || (uintptr_t)dst >= (uintptr_t)src + cnt) {
        asm volatile(
            "rep movsb"
            : "=D"(dst), "=S"(src), "=c"(cnt)
            : "0"(dst), "1"(src), "2"(cnt)
            : "memory");
    } else {
        asm volatile(
            "std\n\t"
            "rep movsb\n\t"
            "cld"
            : "=D"(dst), "=S"(src), "=c"(cnt)
            : "0"((uintptr_t)dst + cnt - 1), "1"((uintptr_t)src + cnt - 1), "2"(cnt)
            : "memory");
    }
    return temp;
}

int memcmp(const void* buf1, const void* buf2, size_t count)
{
    if (!count)
        return (0);

    while (--count && *(char*)buf1 == *(char*)buf2) {
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
    asm volatile(
        "rep movsb"
        : "=D"(d), "=S"(s), "=c"(c)
        : "0"(d), "1"(s), "2"(c)
        : "memory");
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

void* memset32(void* d, uint32_t c, size_t n)
{
    int temp0, temp1;
    asm volatile("rep\n\t"
                 "stosl"
                 : "=&c"(temp0), "=&D"(temp1)
                 : "a"(c), "1"(d), "0"(n)
                 : "memory");

    return d;
}

void* memset16(void* d, uint16_t c, size_t n)
{
    int temp0, temp1;
    asm volatile("rep\n\t"
                 "stosw"
                 : "=&c"(temp0), "=&D"(temp1)
                 : "a"(c), "1"(d), "0"(n)
                 : "memory");
    return d;
}

void* memset(void* d, int c, size_t n)
{
    void* temp = d;
    asm volatile(
        "rep stosb"
        : "=D"(d), "=c"(n)
        : "0"(d), "a"(c), "1"(n)
        : "memory");
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



#define POOL_SIZE(i) (4096 * ((i > 5) ? (i < 12) ? (i >> 1) : ((i >> 1) * (i >> 1)) : 1))
#define ALIGN_UP(addr, size) (((addr) + ((size) - 1)) & (~((typeof(addr))(size) - 1)))
#define MAX_CHUNKS_SIZES 16
#define STMP8_MAX 516096 /* PAGE_SIZE * 126 */

typedef struct __attribute__((packed)) pool {
    void* address;
    uint8_t is_open;
} pool_t;

typedef struct __attribute__((packed)) major {
    pool_t pools[MAX_CHUNKS_SIZES][126];
    uint8_t open_pools[MAX_CHUNKS_SIZES];
    int16_t free_pool[MAX_CHUNKS_SIZES];
    uint8_t cached_open_pools[MAX_CHUNKS_SIZES];
    uint32_t major_index;
    struct major* next;
} major_t;

static major_t first_major;

void* sh_malloc_stamp(uint32_t size)
{
    uint8_t is_big = (size > STMP8_MAX);
    uint32_t total_size = size + is_big * is_big + 1;

    if ((total_size % 4096) != 0)
        total_size = (total_size & ~(4096 - 1)) + 4096;

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

int8_t find_open_pool(uint32_t chunk_size_index, uint8_t* pool_index, major_t* major)
{
    /* With cache */
    uint8_t cached_open = major->cached_open_pools[chunk_size_index];
    if (major->pools[chunk_size_index][cached_open].is_open) {
        *pool_index = cached_open;
        return 0;
    }

    /* Without cache */
    for (*pool_index = 0; *pool_index < 126; ++(*pool_index)) {
        if (major->pools[chunk_size_index][*pool_index].is_open) {
            major->cached_open_pools[chunk_size_index] = *pool_index;
            return 0;
        }
    }

    /* Not found */
    *pool_index = 0;
    return 1;
}

int8_t find_open_pool_pos(pool_t* pool, uint32_t pool_size, uint32_t chunk_size, uint32_t* index_in_pool)
{
    uint32_t start = 0;
    uint32_t i = 0;

    /* Scan pool for free spaces */
    while (start < pool_size) {
        if (!((uint8_t*)(pool->address) + start)[0]) {
            *index_in_pool = i;
            return 0;
        }
        start += chunk_size + 1;
        i++;
    }
    return 1;
}

int16_t find_free_pool(uint32_t chunk_size_index, major_t* major)
{
    for (uint16_t i = 0; i < 126; ++i)
        if (!major->pools[chunk_size_index][i].address)
            return i;
    return -1;
}

major_t* sh_stamp_to_pool(const void* address, int* range, uint8_t* stamp,
    uint8_t* pool, int8_t* chunk_size_index, major_t* major)
{
    if (!major)
        return 0;
    
    /* Determine pool number & size */
    *stamp = ((uint8_t*)address - 1)[0];
    *pool = (*stamp >> 1) - 1;
    if (*stamp == 0) *pool = 0;

    /* Determine chunk size index */
    *chunk_size_index = -1;
    for (uint8_t i = 0; i < MAX_CHUNKS_SIZES; ++i) {
        uint32_t pool_size = POOL_SIZE(i);
        *range = (uint8_t*)address - (uint8_t*)major->pools[i][*pool].address;
        if (*range > 0 && *range < pool_size) {
            *chunk_size_index = i;
            return major;
        }
    }

    /* Not found, try next major */
    return sh_stamp_to_pool(address, range, stamp, pool,
        chunk_size_index, (major_t*)major->next);
}

void sh_free_pool(const void* address)
{
    int range;
    uint8_t stamp, pool;
    int8_t chunk_size_index;
    major_t* major = sh_stamp_to_pool(address, &range, &stamp, &pool, &chunk_size_index, &first_major);
    if (chunk_size_index < 0 || !major)
        return;

    *((uint8_t*)address - 1) = 0;

    /* Case 1: pool was full but is re-opened */
    if (!major->pools[chunk_size_index][pool].is_open) {
        major->pools[chunk_size_index][pool].is_open = 1;
        major->open_pools[chunk_size_index]++;
        return;
    }

    uint32_t pool_size = POOL_SIZE(chunk_size_index);
    if (chunk_size_index == 0)
        chunk_size_index = 1;
    uint32_t chunk_size = 2 << (chunk_size_index - 1);
    bool is_last = true;

    for (int32_t i = 0; i < pool_size; i += chunk_size + 1) {
        if (((uint8_t*)major->pools[chunk_size_index][pool].address)[i]) {
            is_last = false;
            break;
        }
    }

    /* Case 2: last chunk free'd, delete pool */
    if (is_last)  {
        pfree(major->pools[chunk_size_index][pool].address, pool_size);
        major->free_pool[chunk_size_index] = pool;
        major->pools[chunk_size_index][pool].is_open = 0;
        major->pools[chunk_size_index][pool].address = 0;
        major->open_pools[chunk_size_index] -= 1;
    }
}

void* sh_malloc_pool(uint32_t size, void* major_ptr)
{
    major_t* major = (major_t*)major_ptr;
    uint32_t chunk_size = size;
    chunk_size |= chunk_size >> 1;
    chunk_size |= chunk_size >> 2;
    chunk_size |= chunk_size >> 4;
    chunk_size |= chunk_size >> 8;
    chunk_size |= chunk_size >> 16;
    chunk_size++;
    uint8_t chunk_size_index = __builtin_ffs(chunk_size) - 1;
    uint32_t pool_size = POOL_SIZE(chunk_size_index);

    /* Try to find open pool */
    uint8_t pool;
    int8_t do_create_pool = 1;
    if (major->open_pools[chunk_size_index])
        do_create_pool = find_open_pool(chunk_size_index, &pool, major);

    /* Case 1: no pool found, create new pool */
    if (do_create_pool) {
        /* Case 1.1: no available pool in major,
         *           create new major */
        if (major->free_pool[chunk_size_index] < 0) {
            if (!major->next) {
                major_t* next = (major_t*)palloc(ALIGN_UP(sizeof(major_t), 4096));
                memset(next, 0, sizeof(major_t));
                next->major_index = major->major_index + 1;
                major->next = (struct major*)next;
            }
            return sh_malloc_pool(size, major->next);
        }

        /* Case 1.2: available pool in major, allocate it */
        uint8_t* address = (uint8_t*)palloc(pool_size);
        for (int32_t i = 0; i < pool_size; i += chunk_size + 1)
            ((uint8_t*)address)[i] = 0;

        major->pools[chunk_size_index][major->free_pool[chunk_size_index]].address = address;
        major->pools[chunk_size_index][major->free_pool[chunk_size_index]].is_open = 1;
        major->open_pools[chunk_size_index] += 1;
        uint8_t stamp = ((major->free_pool[chunk_size_index] + 1) << 1) & ~(1 << 0);
        ((uint8_t*)address)[0] = stamp;

        major->free_pool[chunk_size_index] = find_free_pool(chunk_size_index, major);
        return address + 1;
    }

    /* Case 2: has open pool */
    uint32_t index_in_pool;
    pool_t* p = &major->pools[chunk_size_index][pool];
    int8_t no_position = find_open_pool_pos(p, pool_size, chunk_size, &index_in_pool);
    if (no_position) return 0;

    uint8_t stamp = ((pool + 1) << 1) & ~(1 << 0);
    uint8_t* address = ((uint8_t*)p->address) + index_in_pool * (chunk_size + 1);
    ((uint8_t*)address)[0] = stamp;

    if ((address + 1 + chunk_size) > (((uint8_t*)p->address) + pool_size)) {
        p->is_open = 0;
        major->open_pools[chunk_size_index]--;
    }

    return address + 1;
}

void* sh_realloc_pool(const void* address, uint32_t size)
{
    int range;
    uint8_t stamp, pool;
    int8_t chunk_size_index;
    major_t* major = sh_stamp_to_pool(address, &range, &stamp, &pool, &chunk_size_index, &first_major);

    if (chunk_size_index < 0 || !major)
        return 0;

    if (chunk_size_index == 0)
        chunk_size_index = 1;

    uint32_t old_size = 2 << (chunk_size_index - 1);
    void* new_alloc = malloc(old_size);
    memcpy(new_alloc, address, old_size);
    sh_free_pool(address);
    return new_alloc;
}

void* sh_realloc_stamp(const void* address, uint32_t size)
{
    uint8_t* addr = (uint8_t*)address;
    addr--;
    uint8_t small_stamp = ((uint8_t*)addr)[0];
    uint16_t big_stamp = 0;
    small_stamp = small_stamp >> 1;

    if (small_stamp == 127) {
        big_stamp = ((uint16_t*)(addr - 2))[0] + 127;
        addr -= 2;
    }

    uint16_t stamp = (!big_stamp) ? small_stamp : big_stamp;
    uint32_t old_size = stamp * 4096;
    void* new_alloc = malloc(size);
    memcpy(new_alloc, address, old_size);
    pfree(addr, old_size);
    return new_alloc;
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

void* sh_realloc(const void* address, size_t size)
{
    if (!address)
        return malloc(size);
    if (size <= 1024)
        return sh_realloc_pool(address, size);
    return sh_realloc_stamp(address, size);
}

void* sh_malloc(size_t size)
{
    if (!size)
        return 0;
    if (size <= 32767)
        return sh_malloc_pool(size, &first_major);
    return sh_malloc_stamp(size);
}


void* malloc(size_t size)
{
    return sh_malloc(size);
}

void* realloc(const void* address, size_t size)
{
    return sh_realloc(address, size);
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

#ifdef __cplusplus
}
#endif
