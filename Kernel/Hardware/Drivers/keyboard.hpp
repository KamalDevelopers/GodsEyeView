#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/string.hpp"
#include "LibC/types.hpp"
#include "driver.hpp"
#include "vga.hpp"

class KeyboardDriver : public InterruptHandler
    , public Driver {
    Port8Bit dataport;
    Port8Bit commandport;

private:
    int x_offset;
    int y_offset;

    int keys_pressed = 0;
    int keys_pressed_raw = 0;

    char last_key;
    uint8_t last_key_raw;
    bool is_shift = 0;

    //uint8_t key_cache[100];

    void OnKey(uint8_t keypress);
    uint8_t KeyA(uint8_t key);

public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual char GetLastKey(int raw = 0);
    virtual int GetKeyPresses(int raw = 0);
};

#endif
