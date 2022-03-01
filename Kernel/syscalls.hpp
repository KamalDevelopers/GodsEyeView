#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vesa.hpp"
#include "Hardware/audio.hpp"
#include "Hardware/interrupts.hpp"
#include "Mem/mm.hpp"
#include "multitasking.hpp"
#include "tty.hpp"

#include <LibC/path.hpp>
#include <LibC/poll.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>
#include <LibC/utsname.hpp>

extern "C" int shutdown();

class Syscalls : public InterruptHandler {
private:
    void sys_exit();
    int sys_read(int file_handle, void* data, int length);
    int sys_write(int file_handle, void* data, int length);
    int sys_open(char* file, int flags);
    int sys_close(int file_handle);
    int sys_waitpid(int pid);
    int sys_chdir(char* dir);
    int sys_stat(char* file, struct stat* buffer);
    int sys_fstat(int file_handle, struct stat* buffer);
    int sys_getpid();
    int sys_setsid();
    void sys_reboot(int cmd);
    int8_t sys_kill(int pid, int sig);
    uint32_t sys_mmap(void* addr, size_t length);
    int sys_munmap(void* addr, size_t length);
    int sys_fchown(int fd, uint32_t owner, uint32_t group);
    int sys_uname(utsname* buffer);
    int sys_sleep(int time);
    int sys_spawn(char* file, char** args);
    int sys_poll(pollfd* fds, uint32_t nfds);
    int sys_listdir(char* dirname, char** entries);
    void sys_getcwd(char* buffer);
    uint32_t sys_display();

public:
    Syscalls(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~Syscalls();

    virtual uint32_t interrupt(uint32_t esp) override;
};

#endif
