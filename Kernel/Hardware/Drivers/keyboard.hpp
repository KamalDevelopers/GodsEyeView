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

    void on_key(uint8_t keypress);
    uint8_t key_a(uint8_t key);

public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();

    static KeyboardDriver* active;

    virtual uint32_t interrupt(uint32_t esp) override;
    char get_last_key(int raw = 0);
    int get_key_presses(int raw = 0);
    char get_key();
    uint8_t read_key();
    void read_keys(int len, char* data);
};

#endif
