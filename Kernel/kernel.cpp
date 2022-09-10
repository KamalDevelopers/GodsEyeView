#include "../multiboot.hpp"
#include "Exec/elf.hpp"
#include "Exec/loader.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include "Mem/pmm.hpp"
#include "Net/arp.hpp"
#include "Net/ethernet.hpp"
#include "Net/icmp.hpp"
#include "Net/ipv4.hpp"

#include "Filesystem/tar.hpp"
#include "Filesystem/vfs.hpp"
#include "Hardware/Drivers/am79c973.hpp"
#include "Hardware/Drivers/ata.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/es1370.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/pcs.hpp"
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
    QemuSerial qemu_serial;
    klog("Kernel initialization started");

    GDT gdt;
    PCI pci;
    CMOS time;
    TaskManager task_manager(&gdt);
    VirtualFilesystem vfs;
    Audio audio;

    klog("Starting memory management and paging");
    uint32_t total_memory = detect_memory(multiboot_info_ptr);
    uint32_t memory_divide = (total_memory >= 100 * MB) ? (MB * 100) : (MB * 10);
    total_memory = (total_memory - (total_memory % (memory_divide))) - 15 * MB;
    uint32_t available_pages = PAGE_ALIGN(total_memory) / PAGE_SIZE;

    Paging::init();
    PhysicalMemoryManager pmm(available_pages);

    VGA vga;
    Vesa vesa(multiboot_info_ptr->framebuffer_addr,
        multiboot_info_ptr->framebuffer_width,
        multiboot_info_ptr->framebuffer_height,
        multiboot_info_ptr->framebuffer_pitch,
        multiboot_info_ptr->framebuffer_bpp);

    klog("Initializing drivers and syscalls");
    InterruptManager interrupts(0x20, &gdt, &task_manager);
    Syscalls syscalls(&interrupts, 0x80);
    SoundBlaster16 sb16(&interrupts);

    klog("Mounting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);
    ata1s.identify();

    Tar fs_tar(&ata1s);
    if (fs_tar.mount() != 0)
        PANIC("Could not mount the filesystem");
    vfs.mount(&fs_tar);

    klog("Starting PCI drivers and networking");
    MouseDriver mouse(&interrupts, multiboot_info_ptr->framebuffer_width,
        multiboot_info_ptr->framebuffer_height);
    KeyboardDriver keyboard(&interrupts);

    Ethernet ethernet;
    if (pci.find_driver(AM79C973::identifier())) {
        AM79C973* am79c973 = new AM79C973(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(am79c973);
    }

    if (pci.find_driver(RTL8139::identifier())) {
        RTL8139* rtl8139 = new RTL8139(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(rtl8139);
    }

    if (sb16.activated()) {
        AUDIO->set_audio_driver(&sb16);
    }

    if (pci.find_driver(ES1370::identifier())) {
        ES1370* es1370 = new ES1370(&interrupts, pci.get_descriptor());
        AUDIO->set_audio_driver(es1370);
    }

    klog("Setting up loaders and tasks");
    Elf elf_load("elf32");
    Loader loader;
    loader.add(&elf_load);

    Task idle("idle", (uint32_t)kernel_idle, 1);
    TM->append_tasks(1, &idle);
    TM->spawn("servers/display", 0);
    TM->spawn("servers/sound", 0);
    TM->spawn("bin/terminal", 0);
    TM->spawn("bin/launcher", 0);

    Mutex::enable();
    TM->activate();
    IRQ::activate();

    while (1)
        ;
}
