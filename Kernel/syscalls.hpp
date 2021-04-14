#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/pcspk.hpp"
#include "Hardware/interrupts.hpp"
#include "Exec/loader.hpp"
#include "Filesystem/vfs.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "Mem/mm.hpp"
#include "multitasking.hpp"
#include "tty.hpp"

extern "C" int shutdown();

class SyscallHandler : public InterruptHandler {
public:
    SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber);
    ~SyscallHandler();

    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif
