#include "../libraries/LibGUI/font.hpp"
#include "../libraries/LibGUI/gui.hpp"
#include "stdio.hpp"
#include "stdlib.hpp"

#include "GDT/gdt.hpp"
#include "Hardware/Drivers/driver.hpp"
#include "Hardware/Drivers/cmos.hpp"
#include "Hardware/Drivers/keyboard.hpp"
#include "Hardware/Drivers/mouse.hpp"
#include "Hardware/Drivers/vga.hpp"
#include "Hardware/Drivers/ata.hpp"
#include "Hardware/Drivers/amd79.hpp"
#include "Hardware/interrupts.hpp"
#include "Hardware/pci.hpp"   
#include "FileSystem/fs.hpp"

#include "multitasking.hpp"
#include "syscalls.hpp"

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for (constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

void poweroff()
{
    Port16Bit qemu_power(0x604);
    qemu_power.Write(0x2000);

    Port16Bit vbox_power(0x4004);
    vbox_power.Write(0x3400);
}

static uint8_t open_term_hook = 0;
void open_term() { open_term_hook = 1; }

extern "C" void kernelMain(void* multiboot_structure, unsigned int magicnumber)
{
    AdvancedTechnologyAttachment ata0s(true, 0x1F0);
    ata0s.Identify();
    char *fserror;
    FileSystem fs(&ata0s, &fserror);

    Graphics vga;
    TaskManager t;
    TimeDriver time;
    DriverManager drvManager;

    vga.Init(640, 480, 16, 0x0);
    GlobalDescriptorTable gdt;

    InterruptManager interrupts(0x20, &gdt, &t);
    PCIcontroller PCI;

    MouseDriver mouse(&interrupts, &vga);
    KeyboardDriver keyboard(&interrupts);

    drvManager.AddDriver(&keyboard);
    drvManager.AddDriver(&mouse);
    PCI.SelectDrivers(&drvManager);

    drvManager.ActivateAll();
    interrupts.Activate();

    GUI::Desktop desktop(640, 480, &vga, &mouse, &keyboard);
    GUI::Window window(0, 0, 640, 21, 0x8, 0);
    GUI::Window *win = &window;

    GUI::Window terminal(22, 42, 250, 150, 0x0, 1);
    GUI::Window *term = &terminal;
    term->SetTitle("Terminal");

    GUI::Image image(10, 10, powerbutton);
    GUI::Button power_button(3, 3, 16, 15, "", poweroff);
    GUI::Button terminal_button(24, 3, 14, 15, "Terminal", open_term);

    char* user_name = "Terry";

    GUI::Label clock_label(630 - (str_len(user_name) * 8) - 69, 7, 0, 10, 0x0, 0x8, "clock");
    GUI::Label user_label(630 - (str_len(user_name) * 8), 7, 0, 10, 0x0, 0x8, user_name);

    power_button.AddImage(&image);
    win->AddWidget("lblb", &user_label, &power_button, &clock_label, &terminal_button);

    desktop.AddWin(1, win);
    while (1)
    {
        clock_label.SetText(time.GetFullTime());
        desktop.Draw();

        if (open_term_hook == 1){ open_term_hook = 0; term->Revive(); desktop.AppendWin(term); }
    }
}