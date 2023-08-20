#ifndef STAT_H
#define STAT_H

#include "types.h"

struct stat {
    int st_uid;
    int st_gid;
    int st_size;
    uint32_t st_atime;
    uint32_t st_mtime;
    uint32_t st_ctime;
};

#ifdef __cplusplus
extern "C" {
#endif

inline int fstat(int fd, struct stat* __restrict statbuffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(28), "b"(fd), "c"(statbuffer));
    return result;
}

inline int stat(const char* filename, struct stat* __restrict statbuffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(18), "b"(filename), "c"(statbuffer));
    return result;
}

#ifdef __cplusplus
}
#endif

#endif
