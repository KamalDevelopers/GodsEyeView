#include "../multiboot.hpp"
#include "Exec/elf.hpp"
#include "Exec/loader.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include "Mem/pmm.hpp"
#include "Net/arp.hpp"
#include "Net/ethernet.hpp"
#include "Net/ipv4.hpp"

#include "Filesystem/tar.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/am79c973.hpp"
#include "Hardware/Drivers/ata.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/rtl8139.hpp"
#include "Hardware/Drivers/sb16.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/pci.hpp"

#include "multitasking.hpp"
#include "panic.hpp"
#include "syscalls.hpp"

#include <LibC/cmath.hpp>
#include <LibC/ctype.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>

typedef void (*constructor)();
extern "C" uint8_t kernel_end;
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

    set_color(VGA16::white, VGA16::black);
    clear_screen();

    GDT gdt;
    PCI pci;
    TaskManager task_manager(&gdt);
    TimeDriver time;
    VirtualFilesystem vfs;
    VGA vga;

    klog("Starting memory management and paging");
    uint32_t total_memory = detect_memory(multiboot_info_ptr);
    uint32_t memory_divide = (total_memory >= 100 * MB) ? (MB * 100) : (MB * 10);
    total_memory = (total_memory - (total_memory % (memory_divide))) - 15 * MB;
    uint32_t available_pages = PAGE_ALIGN(total_memory) / PAGE_SIZE;

    Paging::init();
    PhysicalMemoryManager pmm(available_pages);

    klog("Initializing drivers and syscalls");
    InterruptManager interrupts(0x20, &gdt, &task_manager);
    Syscalls syscalls(&interrupts, 0x80);

    MouseDriver mouse(&interrupts, 640, 480);
    KeyboardDriver keyboard(&interrupts);
    SoundBlaster16 sb16(&interrupts);

    klog("Starting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);
    ata1s.identify();

    Tar fs_tar(&ata1s);
    if (fs_tar.mount() != 0)
        PANIC("Could not mount the filesystem");
    vfs.mount(&fs_tar);

    klog("Starting PCI drivers and networking");
    Ethernet ethernet;
    if (pci.find_driver(AM79C973::identifier())) {
        AM79C973* am79c973 = new AM79C973(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(am79c973);
    }

    if (pci.find_driver(RTL8139::identifier())) {
        RTL8139* rtl8139 = new RTL8139(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(rtl8139);
    }

    klog("Setting up loaders and tasks");
    Elf elf_load("elf32");
    Loader loader;
    loader.add(&elf_load);

    int shell_file_descriptor = VFS->open("shell");
    int size = VFS->size(shell_file_descriptor);
    uint8_t* elfdata = new uint8_t[size];
    VFS->read(shell_file_descriptor, elfdata);
    VFS->close(shell_file_descriptor);

    executable_t execute = Loader::load->exec(elfdata);
    kprintf("Spawning interactive shell...\n\n");

    Task shell("shell", 0);
    shell.executable(execute);
    TM->append_tasks(1, &shell);

    TM->activate();
    IRQ::activate();

    while (1)
        ;
}
