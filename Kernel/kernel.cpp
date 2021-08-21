#include "../multiboot.hpp"
#include "LibC/ctype.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"

#include "Exec/elf.hpp"
#include "Exec/loader.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include "Net/arp.hpp"
#include "Net/etherframe.hpp"
#include "Net/ipv4.hpp"

#include "Filesystem/tar.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/amd79.hpp"
#include "Hardware/Drivers/ata.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/driver.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/pci.hpp"

#include "multitasking.hpp"
#include "panic.hpp"
#include "syscalls.hpp"

typedef void (*constructor)();
extern "C" uint32_t kernel_end;
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" {
multiboot_info_t* multiboot_info_ptr;
}

extern "C" [[noreturn]] void kernel_main(void* multiboot_structure, unsigned int magicnumber)
{
    init_serial();
    klog("Kernel started");
    clear_screen(VGA16::white, VGA16::black);

    GDT gdt;
    TaskManager task_manager(&gdt);
    TimeDriver time;
    VirtualFilesystem vfs;

    Paging::init();
    kernel_end = 10 * 1024 * 2;
    uint32_t* memupper = (uint32_t*)(&multiboot_info_ptr->mem_upper);
    MemoryManager memory_manager(kernel_end, (*memupper) * 1024);
    MM->dump();

    klog("Initializing input drivers and syscalls");
    InterruptManager interrupts(0x20, &gdt, &task_manager);
    SyscallHandler syscalls(&interrupts, 0x80);

    MouseDriver* mouse = new MouseDriver(&interrupts, 640, 480);
    KeyboardDriver* keyboard = new KeyboardDriver(&interrupts);

    klog("Starting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);

    ata1s.identify();
    Tar fs_tar(&ata1s);
    if (fs_tar.mount() != 0)
        PANIC("Could not mount the filesystem");
    vfs.mount(&fs_tar);

    klog("Starting PCI and activating drivers");
    PCI pci;
    DriverManager driver_manager;

    driver_manager.add_driver(keyboard);
    driver_manager.add_driver(mouse);
    pci.select_drivers(&driver_manager, &interrupts);
    driver_manager.activate_all();

    klog("Setting up loaders and tasks");
    Elf elf_load("elf32");
    Loader loader;
    loader.add(&elf_load);

    int d_demo = VFS::open("demo");
    int size = VFS::size(d_demo);
    uint8_t* elfdata = new uint8_t[size];
    VFS::read(d_demo, elfdata);
    VFS::close(d_demo);
    int demo_exec = Loader::load->exec(elfdata);
    kfree(elfdata);
    Task demo("Demo", demo_exec);

    task_manager.append_tasks(1, &demo);
    IRQ::activate();

    while (1)
        ;
}
