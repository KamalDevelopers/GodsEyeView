#include "../multiboot.hpp"
#include "LibC/ctype.hpp"
#include "LibC/stdio.hpp"
#include "LibC/stdlib.hpp"
#include "LibGUI/font.hpp"
#include "LibGUI/gui.hpp"

#include "Exec/elf.hpp"
#include "Exec/loader.hpp"
#include "GDT/gdt.hpp"
#include "Mem/mm.hpp"
#include "Mem/paging.hpp"
#include "Net/arp.hpp"
#include "Net/etherframe.hpp"
#include "Net/ipv4.hpp"

#include "Filesystem/fat.hpp"
#include "Filesystem/part.hpp"
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

extern "C" void callConstructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" {
multiboot_info_t* multiboot_info_ptr;
}

struct DriverObjects {
    MouseDriver* mouse;
    KeyboardDriver* keyboard;
} static drivers;
static uint8_t* wallpaper_data;

void desktopEnvironment()
{
    Graphics vga;

    GUI::Desktop desktop(640, 480, &vga, drivers.mouse, drivers.keyboard);
    vga.Init(640, 480, 16, 0x0);

    GUI::Window window(0, 0, 640, 22, 0x7, 0);
    GUI::Window* win = &window;

    static int shutdown_hook = 0;
    static int shutdown_cancel = 0;
    auto open_shutdown = []() { shutdown_hook = 1; };
    auto close_shutdown = []() { shutdown_cancel = 1; };

    GUI::Image power_image(11, 10, powerbutton);
    GUI::Button power_button(3, 3, 16, 15, "", open_shutdown);

    static uint8_t open_term_hook = 0;
    auto open_term = []() { open_term_hook = 1; };
    GUI::Button terminal_button(25, 3, 14, 15, "Terminal", open_term);

    char* user_name = "Terry";

    GUI::Label clock_label(630 - (str_len(user_name) * 8) - 80, 7, 0, 10, 0x0, 0x7, "clock");
    GUI::Label user_label(630 - (str_len(user_name) * 8), 7, 0, 10, 0x0, 0x7, user_name);

    power_button.AddImage(&power_image);
    win->AddWidget(&power_button);
    win->AddWidget(&terminal_button);
    win->AddWidget(&clock_label);
    win->AddWidget(&user_label);
    desktop.AddWin(1, win);

    GUI::Window terminal(22, 42, 250, 150, 0x0, 1);
    terminal.Border(1, 0x7);
    terminal.SetTitle("Terminal");

    GUI::Input TerminalInput(5, 15, 240, 20, 0xF, 0x0, "Terry@Gev>");
    GUI::Label TerminalOutput(5, 35, 100, 100, 0xF, 0x0, "");
    TerminalInput.HitboxExpand(1);

    terminal.AddWidget(&TerminalOutput);
    terminal.AddWidget(&TerminalInput);
    terminal.SetHidden(1);
    desktop.AddWin(1, &terminal);

    GUI::Window shutdown_modal(200, 170, 235, 80, 0x7, 0);
    GUI::Label shutdown_label(30, 25, 10, 5, 0x0, 0x7, "Shutdown confirmation");
    GUI::Button shutdown_button(5, 55, 10, 20, "Shutdown", _shutdown);
    shutdown_button.Color(0xC);
    GUI::Button reboot_button(85, 55, 20, 20, "Reboot", _reboot);
    GUI::Button cancel_button(160, 55, 20, 20, "Cancel", close_shutdown);
    shutdown_modal.Border(1, 0x8);
    shutdown_modal.AddWidget(&shutdown_button);
    shutdown_modal.AddWidget(&reboot_button);
    shutdown_modal.AddWidget(&cancel_button);
    shutdown_modal.AddWidget(&shutdown_label);
    desktop.AddWin(1, &shutdown_modal);
    shutdown_modal.SetHidden(1);

    GUI::Image wallpaper(640, 480, 0x0);
    wallpaper.ImageRenderer(wallpaper_data);
    desktop.SetWallpaper(&wallpaper);

    while (1) {
        if (open_term_hook == 1) {
            open_term_hook = 0;
            terminal.SetHidden(0);
        } else if (shutdown_hook == 1) {
            shutdown_hook = 0;
            shutdown_modal.SetHidden(0);
        } else if (shutdown_cancel == 1) {
            shutdown_cancel = 0;
            shutdown_modal.SetHidden(1);
        } else {
            clock_label.SetText(TimeDriver::time->GetFullTime());
        }
        desktop.Draw();
    }
}

extern "C" [[noreturn]] void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    init_serial();
    klog("Kernel started");
    clear_screen(VGA16::WHITE, VGA16::BLACK);

    GlobalDescriptorTable gdt;
    TaskManager task_manager(&gdt);
    TimeDriver time;
    VirtualFilesystem vfs;

    Paging::p_init();
    kernel_end = 10 * 1024 * 2;
    uint32_t* memupper = (uint32_t*)(&multiboot_info_ptr->mem_upper);
    MemoryManager memory_manager(kernel_end, (*memupper) * 1024);
    MM->dump();

    klog("Initializing input drivers and syscalls");
    InterruptManager interrupts(0x20, &gdt, &task_manager);
    SyscallHandler syscalls(&interrupts, 0x80);

    drivers.mouse = new MouseDriver(&interrupts, 640, 480);
    drivers.keyboard = new KeyboardDriver(&interrupts);

    klog("Starting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);

    ata1s.Identify();
    Tar fs_tar(&ata1s);
    fs_tar.Mount();
    if (fs_tar.Exists("root/") == 1)
        panic("Could not mount the filesystem");
    vfs.Mount(&fs_tar);

    uint8_t* fdata = new uint8_t[640 * 480];
    int d_wallpaper = VFS::open("root/wallpaper");
    VFS::read(d_wallpaper, fdata);
    VFS::close(d_wallpaper);
    wallpaper_data = fdata;
    kfree(fdata);

    klog("Starting PCI and activating drivers");
    DriverManager driver_manager;
    PCIcontroller PCI;
    driver_manager.AddDriver(drivers.keyboard);
    driver_manager.AddDriver(drivers.mouse);
    PCI.SelectDrivers(&driver_manager, &interrupts);
    driver_manager.ActivateAll();

    klog("Setting up loaders and tasks");
    Elf elf_load("elf32");
    Loader mloader;
    mloader.Add(&elf_load);

    int d_demo = VFS::open("root/demo");
    int size = VFS::size(d_demo);
    uint8_t* elfdata = new uint8_t[size];
    VFS::read(d_demo, elfdata);
    VFS::close(d_demo);
    int demo_exec = Loader::load->Exec(elfdata);
    kfree(elfdata);
    Task Demo("Demo", demo_exec);

    int desktop_exec = (int)&desktopEnvironment;
    Task Desktop("GUI", desktop_exec, 1);

    task_manager.AppendTasks(2, &Desktop, &Demo);
    IRQ::activate();

    while (1)
        ;
}
