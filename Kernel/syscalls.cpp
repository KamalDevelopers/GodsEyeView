#include "syscalls.hpp"

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
    : InterruptHandler(interruptManager, InterruptNumber + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}

void sys_write(int file_handle, char* data, int len)
{
    char* buffer = new char[len];
    if (file_handle == 1) {
        memcpy(buffer, data, len);
        buffer[len] = '\0';
        write_string(buffer);
    }
    kfree(buffer);
}

void sys_reboot(int arg)
{
    switch (arg) {
    case 1:
        outbw(0x604, 0x2000);
        outbw(0x4004, 0x3400);
        shutdown();
        return;

    default:
        uint8_t reboot = 0x02;
        while (reboot & 0x02)
            reboot = inb(0x64);
        outb(0x64, 0xFE);
        return;
    }
}

uint32_t SyscallHandler::HandleInterrupt(uint32_t esp)
{
    CPUState* cpu = (CPUState*)esp;

    switch (cpu->eax) {
    case 4:
        sys_write((int)cpu->ebx, (char*)cpu->ecx, (int)cpu->edx);
        break;

    case 88:
        sys_reboot((int)cpu->ebx);
        break;

    case 162:
        sleep((uint32_t)cpu->ebx);
        break;

    case 400:
        PCS::beep((uint32_t)cpu->ebx, (uint32_t)cpu->ecx);
        break;

    default:
        break;
    }
    return esp;
}
