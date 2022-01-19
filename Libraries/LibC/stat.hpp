#ifndef STAT_HPP
#define STAT_HPP

struct stat {
    int st_uid;
    int st_gid;
    int st_size;
};

inline int fstat(int fd, struct stat* __restrict statbuffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(28), "b"(fd), "c"(statbuffer));
    return result;
}

inline int stat(char* filename, struct stat* __restrict statbuffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(18), "b"(filename), "c"(statbuffer));
    return result;
}

#endif
