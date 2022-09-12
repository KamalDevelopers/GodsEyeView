#ifndef UTSNAME_H
#define UTSNAME_H

struct utsname {
    char sysname[6];
    char release[6];
};

#ifdef __cplusplus
extern "C" {
#endif

inline int uname(struct utsname* buffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(109), "b"(buffer));
    return result;
}

#ifdef __cplusplus
}
#endif

#endif
