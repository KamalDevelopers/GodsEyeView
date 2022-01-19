#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/pcspk.hpp"
#include "Hardware/interrupts.hpp"
#include "LibC/stat.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "LibC/utsname.hpp"
#include "Mem/mm.hpp"
#include "multitasking.hpp"
#include "tty.hpp"

extern "C" int shutdown();

class Syscalls : public InterruptHandler {
private:
    void sys_exit();
    int sys_read(int file_handle, char* data, int length);
    int sys_write(int file_handle, char* data, int length);
    int sys_open(char* file_name);
    int sys_close(int file_handle);
    int sys_stat(char* file_name, struct stat* buffer);
    int sys_fstat(int file_handle, struct stat* buffer);
    int sys_getpid();
    void sys_reboot(int cmd);
    int8_t sys_kill(int pid, int sig);
    uint32_t sys_mmap(void* addr, size_t length);
    int sys_munmap(void* addr, size_t length);
    int sys_uname(utsname* buffer);
    int sys_nanosleep(int time);
    int sys_beep(int time, uint32_t frequency);
    int sys_spawn(char* file, char* args);

public:
    Syscalls(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~Syscalls();

    virtual uint32_t interrupt(uint32_t esp) override;
};

#endif
