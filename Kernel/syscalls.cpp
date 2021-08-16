#include "syscalls.hpp"

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
    : InterruptHandler(interruptManager, InterruptNumber + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}

void sys_read(int file_handle, char* data, int len)
{
    if (len <= 0)
        return;

    char* buffer = new char[len];

    switch (file_handle) {
    case 2:
        /* FIXME: This should not freeze every process */
        KeyboardDriver::active->ReadKeys(len, buffer);
        break;

    default:
        VFS::read(file_handle, (uint8_t*)buffer);
        break;
    }

    buffer[len] = '\0';
    memcpy(data, buffer, len);
    kfree(buffer);
}

void sys_write(int file_handle, char* data, int len)
{
    if (len <= 0)
        return;

    char* buffer = new char[len];
    memcpy(buffer, data, len);

    switch (file_handle) {
    case 1:
        buffer[len] = '\0';
        write_string(data);
        break;

    default:
        /* FIXME: Cannot overwrite existing file */
        VFS::write(file_handle, (uint8_t*)&buffer, len);
        break;
    }

    kfree(buffer);
}

int sys_open(char* file_name)
{
    int descriptor = VFS::open(file_name);
    return descriptor;
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
    case 1:
        TaskManager::active->Kill();
        break;

    case 3:
        sys_read((int)cpu->ebx, (char*)cpu->ecx, (int)cpu->edx);
        break;

    case 4:
        sys_write((int)cpu->ebx, (char*)cpu->ecx, (int)cpu->edx);
        break;

    case 5:
        /* TODO: Support mode and flags */
        int desc;
        desc = sys_open((char*)cpu->ebx);
        cpu->eax = desc;
        break;

    case 6:
        VFS::close((int)cpu->ebx);
        break;

    case 20:
        int pid;
        pid = TaskManager::active->GetPid();
        cpu->eax = pid;
        break;

    case 37:
        cpu->eax = TaskManager::active->SendSignal((int)cpu->ebx, (int)cpu->ecx);
        break;

    case 88:
        sys_reboot((int)cpu->ebx);
        break;

    case 90:
        /* Incomplete implementation */
        cpu->eax = (uint32_t)pmalloc((size_t)cpu->ecx);
        Paging::map_page(cpu->eax, cpu->eax);
        break;

    case 91:
        /* Incomplete implementation */
        pfree((void*)cpu->ebx, (size_t)cpu->ecx);
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
