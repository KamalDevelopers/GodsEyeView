#ifndef ERRNO_H
#define ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

#define E_UNKNOWN 255
#define DESC_E_UNKNOWN ("E_UNKONWN: Unknown")
#define E_PERMISSION 1
#define DESC_E_PERMISSION ("E_PERMISSION: Operation not permitted")
#define E_VFSENTRY 2
#define DESC_E_VSFENTRY ("E_VFSENTRY: No such vfs entry")
#define E_OVERFLOW 7
#define DESC_E_OVERFLOW ("E_OVERFLOW: Input too great or list too long")
#define E_INVALIDEXEC 8
#define DESC_E_INVALIDEXEC ("E_INVALIDEXEC: Invalid executable")
#define E_INVALIDFD 9
#define DESC_E_INVALIDFD ("E_INVALIDFD: Invalid file descriptor")
#define E_NOCHILD 10
#define DESC_E_NOCHILD ("E_NOCHILD: Child or pid does not exist")
#define E_NOMEM 12
#define DESC_E_NOMEM ("E_NOMEM: Cannot allocate memory")
#define E_ACCES 13
#define DESC_E_ACCES ("E_ACCES: Access denied")
#define E_FAULT 14
#define DESC_E_FAULT ("E_FAULT: Bad address")
#define E_INVAL 22
#define DESC_E_INVAL ("E_INVAL: Invalid argument")


const char* error_what(int e);

#ifdef __cplusplus
}
#endif

#endif
