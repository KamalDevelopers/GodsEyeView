#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/pcspk.hpp"
#include "Hardware/interrupts.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "Mem/mm.hpp"
#include "multitasking.hpp"
#include "tty.hpp"

extern "C" int shutdown();

struct utsname {
    char sysname[6];
    char release[6];
};

struct stat {
    int st_uid;
    int st_gid;
    int st_size;
};

class SyscallHandler : public InterruptHandler {
public:
    SyscallHandler(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~SyscallHandler();

    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif
