#ifndef __KEYBOARD_HPP
#define __KEYBOARD_HPP

#include "types.hpp"
#include "../interrupts.hpp"
#include "../port.hpp"
#include "stdio.hpp"
#include "string.hpp"
#include "../../../libraries/LibGUI/font.hpp"
#include "vga.hpp"

class KeyboardDriver : public InterruptHandler
{
    Port8Bit dataport;
    Port8Bit commandport;
public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual char* GetKeys(char* arr);
    virtual char GetLastKey();
    virtual void ScreenOutput(int i, uint8_t color_index, Graphics *g);
private:
	int key_press_index = 0;
    char keys[100];
    void on_key(char keypress, int out_screen);
    int outp = 0;
    Graphics *vga;
    uint8_t color = 0x1;
};

#endif