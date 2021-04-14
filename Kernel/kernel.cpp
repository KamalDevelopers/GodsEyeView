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

void poweroff()
{
    asm("int $0x80"
        :
        : "a"(88), "b"(1));
}

void reboot()
{
    asm("int $0x80"
        :
        : "a"(88), "b"(0));
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

    GUI::Window shutdownModal(200, 170, 235, 80, 0x7, 0);
    GUI::Label shutdown_label(30, 25, 10, 5, 0x0, 0x7, "Shutdown confirmation");
    GUI::Button shutdown_button(5, 55, 10, 20, "Shutdown", poweroff);
    shutdown_button.Color(0xC);
    GUI::Button reboot_button(85, 55, 20, 20, "Reboot", reboot);
    GUI::Button cancel_button(160, 55, 20, 20, "Cancel", close_shutdown);
    shutdownModal.Border(1, 0x8);
    shutdownModal.AddWidget(&shutdown_button);
    shutdownModal.AddWidget(&reboot_button);
    shutdownModal.AddWidget(&cancel_button);
    shutdownModal.AddWidget(&shutdown_label);
    desktop.AddWin(1, &shutdownModal);
    shutdownModal.SetHidden(1);

    GUI::Image wallpaper(640, 480, 0x0);
    wallpaper.ImageRenderer(wallpaper_data);
    desktop.SetWallpaper(&wallpaper);

    while (1) {
        if (open_term_hook == 1) {
            open_term_hook = 0;
            terminal.SetHidden(0);
        } else if (shutdown_hook == 1) {
            shutdown_hook = 0;
            shutdownModal.SetHidden(0);
        } else if (shutdown_cancel == 1) {
            shutdown_cancel = 0;
            shutdownModal.SetHidden(1);
        } else {
            clock_label.SetText(TimeDriver::time->GetFullTime());
        }
        desktop.Draw();
    }
}

extern "C" [[noreturn]] void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    clear_screen();
    init_serial();
    klog("Kernel started");

    GlobalDescriptorTable gdt;
    TaskManager tasksmgr(&gdt);
    TimeDriver time;
    VirtualFilesystem vfs;

    Paging::p_init();
    mm_init(kernel_end);

    klog("Initializing input drivers and syscalls");
    InterruptManager interrupts(0x20, &gdt, &tasksmgr);
    SyscallHandler syscalls(&interrupts, 0x80);
    MouseDriver m(&interrupts, 640, 480);
    KeyboardDriver k(&interrupts);

    drivers.mouse = &m;
    drivers.keyboard = &k;

    klog("Starting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);

    ata1s.Identify();
    Tar fs_tar(&ata1s);
    fs_tar.Mount();

    vfs.Mount(&fs_tar);

    uint8_t* fdata = new uint8_t[640 * 480];
    int d_wallpaper = VFS::open("root/wallpaper");
    VFS::read(d_wallpaper, fdata);
    VFS::close(d_wallpaper);
    memcpy((void*)wallpaper_data, (void*)fdata, 640 * 480);
    kfree(fdata);

    DriverManager drvManager;
    klog("Starting PCI and activating drivers");
    PCIcontroller PCI;
    drvManager.AddDriver(drivers.keyboard);
    drvManager.AddDriver(drivers.mouse);
    PCI.SelectDrivers(&drvManager, &interrupts);
    drvManager.ActivateAll();

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

    int desktop_exec = (int)&desktopEnvironment;

    Task Demo("Demo", demo_exec);
    Task Desktop("GUI", desktop_exec);
    tasksmgr.AppendTasks(2, &Desktop, &Demo);

    IRQ::activate();
    while (1)
        ;
}
