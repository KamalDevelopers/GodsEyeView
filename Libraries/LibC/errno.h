#ifndef ERRNO_H
#define ERRNO_H

#define E_UNKNOWN 255
#define DESC_E_UNKNOWN ("E_UNKONWN: Unknown")
#define E_PERMISSION 1
#define DESC_E_PERMISSION ("E_PERMISSION: Operation not permitted")
#define E_VFSENTRY 2
#define DESC_E_VSFENTRY ("E_VFSENTRY: No such vfs entry")
#define E_INVALIDEXEC 8
#define DESC_E_INVALIDEXEC ("E_INVALIDEXEC: Invalid executable")
#define E_INVALIDFD 9
#define DESC_E_INVALIDFD ("E_INVALIDFD: Invalid file descriptor")

const char* error_what(int e);

#endif
