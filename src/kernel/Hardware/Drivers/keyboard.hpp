#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include "driver.hpp"
#include "stdio.hpp"
#include "string.hpp"
#include "types.hpp"
#include "vga.hpp"

class KeyboardDriver : public InterruptHandler
    , public Driver {
    Port8Bit dataport;
    Port8Bit commandport;

public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual char* GetKeys();
    virtual char GetLastKey();
    virtual uint8_t GetIndex() { return key_press_index; };
    virtual void ScreenOutput(int i, uint8_t color_index, int x_offset, int y_offset, Graphics* g);

private:
    int x_offset;
    int y_offset;

    Graphics* vga;
    uint8_t is_changed = 0;
    uint16_t key_press_index = 0;
    char keys[100];
    void on_key(char keypress, int out_screen);
    int outp = 0;
    uint8_t color = 0x1;
};

#endif