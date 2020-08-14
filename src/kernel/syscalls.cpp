#include "syscalls.hpp"

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}

void sys_printf(int file_handle, char* data)
{
    if (file_handle == 1)
        printf("%s", data);
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;

    switch(cpu->eax)
    {
        case 4:
            sys_printf((int)cpu->ebx, (char*)cpu->ecx);
            break;
        case 162:
            sleep((uint32_t)cpu->ebx);
            break;

        default:
            break;
    }
    return esp;
}
