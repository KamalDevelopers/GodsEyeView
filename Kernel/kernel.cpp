#include "../multiboot.hpp"
#include "Exec/elf.hpp"
#include "Exec/loader.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include "Mem/pmm.hpp"
#include "Net/arp.hpp"
#include "Net/dhcp.hpp"
#include "Net/dns.hpp"
#include "Net/ethernet.hpp"
#include "Net/icmp.hpp"
#include "Net/ipv4.hpp"
#include "Net/tcp.hpp"
#include "Net/udp.hpp"

#include "Filesystem/husky.hpp"
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
#include "Hardware/cpuid.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/pci.hpp"

#include "multitasking.hpp"
#include "panic.hpp"
#include "syscalls.hpp"

#include <LibC/cmath.h>
#include <LibC/ctype.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>

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
    if (qemu_serial.is_supported())
        klog("Compile flag QEMU virtualization set");

    klog("CPU detection started");
    cpuid_detect();

    GDT gdt;
    PCI pci;
    CMOS time;
    TaskManager task_manager(&gdt);
    VirtualFilesystem vfs;
    Audio audio;

    klog("Memory management and paging started");
    uint32_t total_memory = detect_memory(multiboot_info_ptr);
    uint32_t memory_divide = (total_memory >= 100 * MB) ? (MB * 100) : (MB * 10);
    total_memory = (total_memory - (total_memory % (memory_divide))) - 15 * MB;
    uint32_t available_pages = PAGE_ALIGN(total_memory) / PAGE_SIZE;
    bool has_volatile_memory = 0;

    PhysicalMemoryManager pmm(available_pages);
    if (!has_volatile_memory)
        Paging::init();

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

    klog("Filesystem mounting started");
    if (!pci.find_driver(ATA::identifier()))
        klog("Could not find ATA device");

    ATA ata1s(&interrupts, pci.get_descriptor(), 0x1F0, true);
    ata1s.identify();
    ata1s.set_dma(false);
    Tar fs_tar(&ata1s);
    Husky fs_husky(&ata1s);
    uint8_t has_filesystem = 0;

    klog("Drivers and networking initialization");
    MouseDriver mouse(&interrupts, multiboot_info_ptr->framebuffer_width,
        multiboot_info_ptr->framebuffer_height);
    KeyboardDriver keyboard(&interrupts);

    if (sb16.activated()) {
        klog("PCI: audio driver sb16");
        AUDIO->set_audio_driver(&sb16);
    }

    if (pci.find_driver(ES1370::identifier())) {
        klog("PCI: audio driver es1370");
        ES1370* es1370 = new ES1370(&interrupts, pci.get_descriptor());
        AUDIO->set_audio_driver(es1370);
    }

    Ethernet ethernet;
    if (pci.find_driver(AM79C973::identifier())) {
        klog("PCI: ethernet driver am79c973");
        AM79C973* am79c973 = new AM79C973(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(am79c973);
    }

    if (pci.find_driver(RTL8139::identifier())) {
        klog("PCI: ethernet driver rtl8139");
        RTL8139* rtl8139 = new RTL8139(&interrupts, pci.get_descriptor());
        ethernet.set_network_driver(rtl8139);
    }

    if (has_volatile_memory)
        klog("Volatile memory mode");
    klog("ELF loader initialization");
    Elf elf_load("elf32");
    Loader loader;
    loader.add(&elf_load);

    Task idle("idle", (uint32_t)kernel_idle, 1);
    TM->append_tasks(1, &idle);

    klog("IRQ activate reached");
    Mutex::enable();
    ata1s.set_dma(true);
    IRQ::activate();

    if (!has_filesystem && (fs_husky.mount() == 0)) {
        klog("Husky filesystem mounted");
        vfs.mount(&fs_husky);
        has_filesystem = 1;
    }
    if (!has_filesystem && (fs_tar.mount() == 0)) {
        klog("TAR filesystem mounted");
        vfs.mount(&fs_tar);
        has_filesystem = 2;
    }
    if (!has_filesystem) {
        PANIC("Could not mount the filesystem");
    }

    if (ethernet.get_available_driver() != 0) {
        klog("Network initialization");
        DHCP::discover();
        ARP::broadcast_mac_address(DHCP::gateway());
    }

    TM->spawn("servers/display", 0, 0);
    TM->spawn("servers/sound", 0, 0);
    TM->spawn("bin/launcher", 0, 0);
    klog("TM activate reached");
    TM->activate();

    while (1)
        ;
}
