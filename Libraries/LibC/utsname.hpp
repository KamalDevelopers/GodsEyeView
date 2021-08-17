#ifndef UTSNAME_HPP
#define UTSNAME_HPP

struct utsname {
    char sysname[6];
    char release[6];
};

inline int uname(struct utsname* buffer)
{
    int result;
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(109), "b"(buffer));
    return result;
}

#endif
