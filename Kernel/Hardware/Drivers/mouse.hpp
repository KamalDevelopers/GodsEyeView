#ifndef MOUSE_HPP
#define MOUSE_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "driver.hpp"
#include "vga.hpp"

class MouseDriver : public InterruptHandler
    , public Driver {
    Port8Bit dataport;
    Port8Bit commandport;
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;
    uint8_t active = 1;

    uint8_t x, y;
    int32_t w, h;
    uint32_t mouse_x, mouse_y, mouse_press;

public:
    MouseDriver(InterruptManager* manager, int screenw, int screenh);
    ~MouseDriver();
    virtual void set_res(int width, int height)
    {
        w = width;
        h = height;
    }

    virtual uint32_t HandleInterrupt(uint32_t esp) override;
    virtual void start() { active = 1; }
    virtual void stop() { active = 0; }

    virtual void on_mouse_move(int x, int y);
    virtual void on_mouse_up();
    virtual void on_mouse_down(int b);
    virtual int get_mouse_y() const { return mouse_y; }
    virtual int get_mouse_x() const { return mouse_x; }
    virtual int get_mouse_press() const { return mouse_press; }
};

#endif
