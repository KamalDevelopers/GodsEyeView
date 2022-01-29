#include "syscalls.hpp"

Syscalls::Syscalls(InterruptManager* interrupt_manager, uint8_t interrupt_number)
    : InterruptHandler(interrupt_manager, interrupt_number + interrupt_manager->get_hardware_interrupt_offset())
{
}

Syscalls::~Syscalls()
{
}

void Syscalls::sys_exit()
{
    TM->kill();
}

int Syscalls::sys_read(int fd, char* data, int length)
{
    if (length <= 0)
        return -1;

    char* buffer = new char[length];
    int size = 0;

    switch (fd) {
    case 0:
        size = TM->read_stdin(buffer, length);
        break;

    default:
        size = VFS->read(fd, (uint8_t*)buffer);
        break;
    }

    if (length < size)
        size = length;

    buffer[length] = '\0';
    memcpy(data, buffer, length);
    kfree(buffer);
    return size;
}

int Syscalls::sys_write(int fd, char* data, int length)
{
    if (length <= 0)
        return -1;

    switch (fd) {
    case 1:
        data[length] = '\0';
        write_string(data);
        break;

    default:
        /* FIXME: Cannot overwrite existing file */
        VFS->write(fd, (uint8_t*)&data, length);
        break;
    }

    return length;
}

int Syscalls::sys_open(char* file)
{
    return VFS->open(file);
}

int Syscalls::sys_close(int fd)
{
    return VFS->close(fd);
}

int Syscalls::sys_waitpid(int pid)
{
    return TM->waitpid(pid);
}

int Syscalls::sys_chdir(char* dir)
{
    return TM->task()->chdir(dir);
}

int Syscalls::sys_stat(char* file, struct stat* buffer)
{
    int fd = VFS->open(file);
    int status = sys_fstat(fd, buffer);
    VFS->close(fd);
    return status;
}

int Syscalls::sys_fstat(int fd, struct stat* buffer)
{
    buffer->st_uid = VFS->uid(fd);
    buffer->st_gid = VFS->gid(fd);
    buffer->st_size = VFS->size(fd);
    return (buffer->st_size == -1) ? -1 : 0;
}

int Syscalls::sys_getpid()
{
    return TM->task()->get_pid();
}

void Syscalls::sys_reboot(int cmd)
{
    switch (cmd) {
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

int8_t Syscalls::sys_kill(int pid, int sig)
{
    return TM->send_signal(pid, sig);
}

uint32_t Syscalls::sys_mmap(void* addr, size_t length)
{
    return PMM::allocate_pages(length);
}

int Syscalls::sys_munmap(void* addr, size_t length)
{
    return PMM::free_pages((uint32_t)addr, length);
}

int Syscalls::sys_uname(utsname* buffer)
{
    strcpy(buffer->sysname, "GevOS");
    strcpy(buffer->release, "0.1.0");
    return 0;
}

int Syscalls::sys_sleep(int sec)
{
    TM->sleep(sec * 18);
    TM->yield();
    return 0;
}

int Syscalls::sys_spawn(char* file, char** args)
{
    return TM->spawn(file, args);
}

int Syscalls::sys_listdir(char* dirname, char** entries)
{
    return VFS->listdir(dirname, entries);
}

void Syscalls::sys_getcwd(char* buffer)
{
    TM->task()->cwd(buffer);
}

uint32_t Syscalls::interrupt(uint32_t esp)
{
    cpu_state* cpu = (cpu_state*)esp;
    IRQ::activate();

    switch (cpu->eax) {
    case 1:
        sys_exit();
        break;

    case 3:
        cpu->eax = sys_read((int)cpu->ebx, (char*)cpu->ecx, (int)cpu->edx);
        break;

    case 4:
        cpu->eax = sys_write((int)cpu->ebx, (char*)cpu->ecx, (int)cpu->edx);
        break;

    case 5:
        cpu->eax = sys_open((char*)cpu->ebx);
        break;

    case 6:
        cpu->eax = sys_close((int)cpu->ebx);
        break;

    case 7:
        cpu->eax = sys_waitpid((int)cpu->ebx);
        break;

    case 12:
        cpu->eax = sys_chdir((char*)cpu->ebx);
        break;

    case 18:
        cpu->eax = sys_stat((char*)cpu->ebx, (struct stat*)cpu->ecx);
        break;

    case 20:
        cpu->eax = sys_getpid();
        break;

    case 28:
        cpu->eax = sys_fstat((int)cpu->ebx, (struct stat*)cpu->ecx);
        break;

    case 37:
        cpu->eax = sys_kill((int)cpu->ebx, (int)cpu->ecx);
        break;

    case 88:
        sys_reboot((int)cpu->ebx);
        break;

    case 90:
        cpu->eax = sys_mmap((void*)cpu->ebx, (size_t)cpu->ecx);
        break;

    case 91:
        cpu->eax = sys_munmap((void*)cpu->ebx, (size_t)cpu->ecx);
        break;

    case 109:
        cpu->eax = sys_uname((utsname*)cpu->ebx);
        break;

    case 162:
        cpu->eax = sys_sleep((uint32_t)cpu->ebx);
        break;

    case 183:
        sys_getcwd((char*)cpu->ebx);
        break;

    case 401:
        cpu->eax = sys_spawn((char*)cpu->ebx, (char**)cpu->ecx);
        break;

    case 402:
        cpu->eax = sys_listdir((char*)cpu->ebx, (char**)cpu->ecx);
        break;
    }

    return esp;
}
