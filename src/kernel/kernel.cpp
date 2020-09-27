#include "../libraries/LibGUI/font.hpp"
#include "../libraries/LibGUI/gui.hpp"
#include "../multiboot.hpp"
#include "stdio.hpp"
#include "stdlib.hpp"

#include "GDT/gdt.hpp"
#include "Net/arp.hpp"
#include "Net/etherframe.hpp"
#include "Net/ipv4.hpp"

#include "Filesystem/fat.hpp"
#include "Filesystem/part.hpp"
#include "Filesystem/tar.hpp"
#include "Hardware/Drivers/amd79.hpp"
#include "Hardware/Drivers/ata.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/driver.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/pci.hpp"

#include "memory.hpp"
#include "multitasking.hpp"
#include "syscalls.hpp"

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" int shutdown();
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
    shutdown();
    Port16Bit qemu_power(0x604);
    qemu_power.Write(0x2000);

    Port16Bit vbox_power(0x4004);
    vbox_power.Write(0x3400);
}

void reboot()
{
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
}

struct DriverObjects {
    MouseDriver* mouse;
    KeyboardDriver* keyboard;
} static drivers;

static uint8_t* wallpaper_data;
void desktopEnvironment()
{
    TimeDriver time;
    Graphics vga;

    GUI::Desktop desktop(640, 480, &vga, drivers.mouse, drivers.keyboard);
    vga.Init(640, 480, 16, 0x0);

    GUI::Window window(0, 0, 640, 21, 0x8, 0);
    GUI::Window* win = &window;

    GUI::Image power_image(11, 10, powerbutton);
    GUI::Image reboot_image(11, 10, rebootbutton);
    GUI::Button power_button(3, 3, 16, 15, "", poweroff);
    GUI::Button reboot_button(25, 3, 16, 15, "", reboot);

    static uint8_t open_term_hook = 0;
    auto open_term = []() { open_term_hook = 1; };
    //GUI::Button terminal_button(36, 3, 14, 15, "Terminal", open_term);

    char* user_name = "Terry";

    GUI::Label clock_label(630 - (str_len(user_name) * 8) - 80, 7, 0, 10, 0x0, 0x8, "clock");
    GUI::Label user_label(630 - (str_len(user_name) * 8), 7, 0, 10, 0x0, 0x8, user_name);

    power_button.AddImage(&power_image);
    reboot_button.AddImage(&reboot_image);
    win->AddWidget(&power_button);
    win->AddWidget(&reboot_button);
    //win->AddWidget(&terminal_button);
    win->AddWidget(&clock_label);
    win->AddWidget(&user_label);
    desktop.AddWin(1, win);

    GUI::Window terminal(22, 42, 250, 150, 0x0, 1);
    terminal.SetTitle("Terminal");

    GUI::Input TerminalInput(5, 15, 240, 20, 0xF, 0x0, "Terry@Gev>");
    GUI::Label TerminalOutput(5, 35, 100, 100, 0xF, 0x0, "");
    TerminalInput.HitboxExpand(1);

    terminal.AddWidget(&TerminalOutput);
    terminal.AddWidget(&TerminalInput);
    terminal.SetHidden(1);
    desktop.AddWin(1, &terminal);

    GUI::Image wallpaper(640, 480, 0x0);
    wallpaper.ImageRenderer(wallpaper_data);
    desktop.SetWallpaper(&wallpaper);

    while (1) {
        desktop.Draw();
        /*Launch Application*/
        if (open_term_hook == 1) {
            open_term_hook = 0;
            terminal.SetHidden(0);
        } else {
            clock_label.SetText(time.GetFullTime());
        }
    }
}

extern "C" [[noreturn]] void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    clear_screen();
    init_serial();
    klog("Kernel started");

    GlobalDescriptorTable gdt;
    TaskManager tasksmgr;

    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10 * 1024 * 1024;
    kheap_init(heap, (*memupper) * 1024 - heap - 10 * 1024);

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

    uint8_t* data;
    fs_tar.ReadFile("root/wallpaper", data);
    wallpaper_data = data;

    DriverManager drvManager;
    klog("Starting PCI and activating drivers");
    PCIcontroller PCI;
    drvManager.AddDriver(drivers.keyboard);
    drvManager.AddDriver(drivers.mouse);
    PCI.SelectDrivers(&drvManager, &interrupts);
    drvManager.ActivateAll();

    klog("Setting up tasks");
    Task DesktopTask(&gdt, desktopEnvironment);
    tasksmgr.AppendTasks(1, &DesktopTask);

    interrupts.Activate();
    while (1)
        ;
}
