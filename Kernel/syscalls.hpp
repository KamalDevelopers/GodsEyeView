#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vesa.hpp"
#include "Hardware/audio.hpp"
#include "Hardware/cpuid.hpp"
#include "Hardware/interrupts.hpp"
#include "Mem/mm.hpp"
#include "Net/dhcp.hpp"
#include "Net/dns.hpp"
#include "Net/icmp.hpp"
#include "Tasks/multitasking.hpp"
#include "panic.hpp"
#include "tty.hpp"

#include <LibC/errno.h>
#include <LibC/network.h>
#include <LibC/path.h>
#include <LibC/poll.h>
#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/types.h>
#include <LibC/utsname.h>

extern "C" int shutdown();

class Syscalls : public InterruptHandler {
private:
    void sys_exit();
    int sys_read(int fd, void* data, int length);
    int sys_write(int fd, void* data, int length);
    int sys_open(char* file, int flags);
    int sys_close(int fd);
    int sys_waitpid(int pid);
    int sys_unlink(char* pathname);
    int sys_chdir(char* dir);
    uint32_t sys_time();
    int sys_stat(char* file, struct stat* buffer);
    int sys_fstat(int fd, struct stat* buffer);
    int sys_socketcall(int call, uint32_t* args);
    int sys_getpid();
    int sys_setsid();
    void sys_reboot(int cmd);
    int sys_nice(int inc);
    int8_t sys_kill(int pid, int sig);
    uint32_t sys_mmap(void* addr, size_t length);
    int sys_munmap(void* addr, size_t length);
    int sys_fchown(int fd, uint32_t owner, uint32_t group);
    int sys_uname(utsname* buffer);
    int sys_sleep(int time);
    int sys_mkfifo(char* pathname, int mode);
    int sys_spawn(char* pathname, char** args, uint8_t argc);
    int sys_poll(pollfd* fds, uint32_t nfds, int timeout);
    int sys_listdir(char* dirname, fs_entry_t* entries, uint32_t count);
    void sys_getcwd(char* buffer);
    int sys_osinfo(struct osinfo* buffer);
    int sys_getchar(int* character);
    void sys_guard(uintptr_t stk, uint8_t type);

public:
    Syscalls(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~Syscalls();

    virtual uint32_t interrupt(uint32_t esp) override;
};

#endif
