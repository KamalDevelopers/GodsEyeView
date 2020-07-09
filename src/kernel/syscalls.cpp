#include "syscalls.hpp"

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;

    switch(cpu->eax)
    {
        case 4:
            printf("%s", (char*)cpu->ebx);
            break;
        case 162:
            sleep((uint32_t)cpu->ebx);
            break;

        default:
            break;
    }
    return esp;
}