#ifndef MOUSE_HPP
#define MOUSE_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include "vga.hpp"
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>

class MouseDriver : public InterruptHandler {
    Port8Bit data_port;
    Port8Bit command_port;
    uint8_t buffer[3];
    uint8_t offset = 0;
    uint8_t buttons = 0;
    uint8_t active = 1;

    uint8_t x = 0;
    uint8_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    uint32_t mouse_x = 0;
    uint32_t mouse_y = 0;
    uint32_t mouse_press = 0;

public:
    MouseDriver(InterruptManager* manager, int screenw, int screenh);
    ~MouseDriver();
    void set_res(int width, int height)
    {
        w = width;
        h = height;
    }

    uint32_t interrupt(uint32_t esp) override;
    void start() { active = 1; }
    void stop() { active = 0; }

    void on_mouse_move(int x, int y);
    void on_mouse_up();
    void on_mouse_down(int b);
    int get_mouse_y() const { return mouse_y; }
    int get_mouse_x() const { return mouse_x; }
    int get_mouse_press() const { return mouse_press; }
};

#endif
