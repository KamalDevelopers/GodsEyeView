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

struct drivers_t {
    MouseDriver* mouse;
    KeyboardDriver* keyboard;
} static drivers;
static uint8_t* p_wallpaper_data;

void gui()
{
    Graphics vga;

    GUI::Desktop desktop(640, 480, &vga, drivers.mouse, drivers.keyboard);
    vga.init(640, 480, 16, 0x0);

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

    power_button.add_image(&power_image);
    win->add_widget(&power_button);
    win->add_widget(&terminal_button);
    win->add_widget(&clock_label);
    win->add_widget(&user_label);
    desktop.add_win(1, win);

    GUI::Window terminal(22, 42, 250, 150, 0x0, 1);
    terminal.border(1, 0x7);
    terminal.set_title("Terminal");

    GUI::Input terminal_input(5, 15, 240, 20, 0xF, 0x0, "Terry@Gev>");
    GUI::Label terminal_output(5, 35, 100, 100, 0xF, 0x0, "");
    terminal_input.set_hitbox_expand(1);

    terminal.add_widget(&terminal_output);
    terminal.add_widget(&terminal_input);
    terminal.set_hidden(1);
    desktop.add_win(1, &terminal);

    GUI::Window shutdown_modal(200, 170, 235, 80, 0x7, 0);
    GUI::Label shutdown_label(30, 25, 10, 5, 0x0, 0x7, "Shutdown confirmation");
    GUI::Button shutdown_button(5, 55, 10, 20, "Shutdown", _shutdown);
    shutdown_button.color(0xC);
    GUI::Button reboot_button(85, 55, 20, 20, "Reboot", _reboot);
    GUI::Button cancel_button(160, 55, 20, 20, "Cancel", close_shutdown);
    shutdown_modal.border(1, 0x8);
    shutdown_modal.add_widget(&shutdown_button);
    shutdown_modal.add_widget(&reboot_button);
    shutdown_modal.add_widget(&cancel_button);
    shutdown_modal.add_widget(&shutdown_label);
    desktop.add_win(1, &shutdown_modal);
    shutdown_modal.set_hidden(1);

    GUI::Image wallpaper(640, 480, 0x0);
    wallpaper.image_renderer(p_wallpaper_data);
    desktop.set_wallpaper(&wallpaper);

    while (1) {
        if (open_term_hook == 1) {
            open_term_hook = 0;
            terminal.set_hidden(0);
        } else if (shutdown_hook == 1) {
            shutdown_hook = 0;
            shutdown_modal.set_hidden(0);
        } else if (shutdown_cancel == 1) {
            shutdown_cancel = 0;
            shutdown_modal.set_hidden(1);
        } else {
            char* time;
            TimeDriver::time->get_full_time(':', time);
            clock_label.set_text(time);
        }
        desktop.draw();
    }
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

    drivers.mouse = new MouseDriver(&interrupts, 640, 480);
    drivers.keyboard = new KeyboardDriver(&interrupts);

    klog("Starting filesystem");
    AdvancedTechnologyAttachment ata1s(true, 0x1F0);

    ata1s.identify();
    Tar fs_tar(&ata1s);
    fs_tar.mount();
    if (fs_tar.exists("welcome") == 1)
        PANIC("Could not mount the filesystem");
    vfs.mount(&fs_tar);

    uint8_t* wallpaper_data = new uint8_t[640 * 480];
    int d_wallpaper = VFS::open("wallpaper");
    VFS::read(d_wallpaper, wallpaper_data);
    VFS::close(d_wallpaper);
    p_wallpaper_data = wallpaper_data;

    klog("Starting PCI and activating drivers");
    PCI pci;
    DriverManager driver_manager;

    driver_manager.add_driver(drivers.keyboard);
    driver_manager.add_driver(drivers.mouse);
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

    int desktop_exec = (int)&gui;
    Task desktop("GUI", desktop_exec, 1);

    task_manager.append_tasks(2, &desktop, &demo);
    IRQ::activate();

    while (1)
        ;
}
