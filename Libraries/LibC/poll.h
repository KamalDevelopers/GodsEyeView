#ifndef POLL_H
#define POLL_H

#include "types.h"

#define POLLIN 0x0001
#define POLLINS 0x0002
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#define POLLNVAL 0x0020
#define POLLRDNORM 0x0040
#define POLLRDBAND 0x0080

struct pollfd {
    int fd;
    int16_t events;
    int16_t revents;
};

#ifdef __cplusplus
extern "C" {
#endif

inline int poll(pollfd* fds, uint32_t nfds, int timeout)
{
    int status;
    asm volatile("int $0x80"
                 : "=a"(status)
                 : "a"(168), "b"(fds), "c"(nfds), "d"(timeout));
    return status;
}

#ifdef __cplusplus
}
#endif

#endif
