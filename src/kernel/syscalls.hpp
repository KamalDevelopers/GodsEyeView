#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "Hardware/interrupts.hpp"
#include "multitasking.hpp"
#include "stdio.hpp"
#include "tty.hpp"
#include "types.hpp"

class SyscallHandler : public InterruptHandler {
public:
    SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber);
    ~SyscallHandler();

    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif
