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
#include "Hardware/Drivers/USB/ehci.hpp"
#include "Hardware/Drivers/USB/scsi.hpp"
#include "Hardware/Drivers/USB/usb.hpp"
#include "Hardware/Drivers/ac97.hpp"
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
#include "Tasks/multitasking.hpp"

#include "panic.hpp"
#include "syscalls.hpp"

#include <LibC/ctype.h>
#include <LibC/math.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>

#define SYSTEM_INIT "bin/init"

typedef void (*constructor)();
extern "C" uint8_t kernel_end;
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
static char userspace_fxsave_region[512] __attribute__((aligned(16)));

extern "C" void call_constructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" {
multi_t* multiboot_info_ptr;
}

extern "C" [[noreturn]] void kernel_main(void* multiboot_structure, unsigned int magicnumber)
{
    gdt();
    tss(5, 0x10, 0);
    set_pit_hz();

    QemuSerial qemu_serial;
    if (qemu_serial.is_supported())
        klog("Compile flag QEMU virtualization set");

    klog("CPU detection started");
    cpuid_detect();

    PCI pci;
    CMOS time;
    TaskManager task_manager;
    VirtualFilesystem vfs;
    Audio audio;

    klog("Memory management and paging started");
    uint32_t total_memory = multiboot_info_ptr->upper_memory;
    uint32_t memory_divide = (total_memory >= 100 * MB) ? (MB * 100) : (MB * 10);
    total_memory = (total_memory - (total_memory % (memory_divide))) - 20 * MB;
    uint32_t available_pages = PAGE_ALIGN(total_memory) / PAGE_SIZE;
    bool has_volatile_storage = has_volatile_disk();
    bool has_volatile_memory = 0;

    if (has_volatile_storage)
        klog("RAM disk mode");
    if (has_volatile_memory)
        klog("Volatile memory mode");

    PhysicalMemoryManager pmm(available_pages);
    if (!has_volatile_memory)
        Paging::init();

    if (multiboot_info_ptr->magic != 0xCAFE)
        PANIC("Invalid multiboot magic");

    klog("Initializing drivers and syscalls");
    VGA vga;
    Vesa vesa(multiboot_info_ptr->vesa_framebuffer,
        multiboot_info_ptr->vesa_width,
        multiboot_info_ptr->vesa_height,
        multiboot_info_ptr->vesa_pitch,
        multiboot_info_ptr->vesa_bpp);

    InterruptManager interrupts(0x20, &task_manager);
    Syscalls syscalls(&interrupts, 0x80);
    SoundBlaster16 sb16(&interrupts);

    if (!pci.find_driver(ATA::identifier()))
        klog("Could not find ATA device");

    klog("Driver initialization");
    Storage storage;
    ATActrl atactrl(&interrupts, pci.get_descriptor());
    atactrl.identify_all();
    atactrl.register_all(&storage);
    atactrl.disable_dma();

    if (has_volatile_storage) {
        VolatileStorage volatile_storage(RAM_DISK);
        storage.register_storage_device(&volatile_storage);
    }

    MouseDriver mouse(&interrupts, multiboot_info_ptr->vesa_width,
        multiboot_info_ptr->vesa_height);
    KeyboardDriver keyboard(&interrupts);

    if (sb16.activated()) {
        klog("PCI: audio driver sb16");
        AUDIO->set_audio_driver(&sb16);
    }

    if (pci.find_driver(EHCI::identifier())) {
        klog("PCI: USB EHCI driver");
        EHCI* ehci = new EHCI(&interrupts, pci.get_descriptor());
    }

    if (pci.find_driver(ES1370::identifier())) {
        klog("PCI: audio driver es1370");
        ES1370* es1370 = new ES1370(&interrupts, pci.get_descriptor());
        AUDIO->set_audio_driver(es1370);
    }

    else if (pci.find_driver(AC97::identifier())) {
        klog("PCI: audio driver AC97");
        AC97* ac97 = new AC97(&interrupts, pci.get_descriptor());
        AUDIO->set_audio_driver(ac97);
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

    klog("USB: Enumerating devices");
    klog("USB: Devices connected %d", usb_device_count());
    for (uint32_t i = 0; i < usb_device_count(); ++i) {
        usb_device* device = usb_device_at(i);
        if (device->protocol != 2)
            continue;
        SCSI* scsi_device = new SCSI(device);
        storage.register_storage_device(scsi_device);
    }

    if (!storage.find_boot_storage())
        klog("Could not find boot storage device");

    klog("Filesystem mounting started");
    Tar fs_tar(storage.boot_device());
    Husky fs_husky(storage.boot_device());
    uint8_t has_filesystem = 0;

    klog("ELF loader initialization");
    Elf elf_load("elf32");
    Loader loader;
    loader.add(&elf_load);

    Task idle("idle", (uint32_t)kernel_idle, 1);
    TM->append_tasks(1, &idle);

    klog("IRQ activate reached");
    Mutex::enable();
    atactrl.enable_dma();
    IRQ::activate();

    klog("Filesystem detection started");
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
        PANIC("Could not mount any filesystem on boot drive");
    }

    if (ethernet.get_available_driver() != 0) {
        klog("Network initialization");
        DHCP::discover();
        ARP::broadcast_mac_address(DHCP::gateway());
    }

    asm volatile(" fxsave %0 " ::"m"(userspace_fxsave_region));

    klog("Spawning init system");
    TM->spawn(SYSTEM_INIT, 0, 0);

    klog("TM activate reached");
    TM->activate();

    while (1)
        ;
}
