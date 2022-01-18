#include "mouse.hpp"

MouseDriver::MouseDriver(InterruptManager* manager, int screenw, int screenh)
    : InterruptHandler(manager, 0x2C)
    , dataport(0x60)
    , commandport(0x64)
{
    offset = 0;
    buttons = 0;
    w = screenw;
    h = screenh;
    mouse_x = w / 2;
    mouse_y = h / 2;

    commandport.write(0xA8);
    commandport.write(0x20);
    uint8_t status = dataport.read() | 2;
    commandport.write(0x60);
    dataport.write(status);

    commandport.write(0xD4);
    dataport.write(0xF4);
    dataport.read();
}

MouseDriver::~MouseDriver()
{
}

void MouseDriver::on_mouse_move(int x, int y)
{
    int32_t new_mouse_x = mouse_x + x;
    int32_t new_mouse_y = mouse_y + y;

    if (new_mouse_x < 0)
        new_mouse_x = 0;
    if (new_mouse_x >= w - 2)
        new_mouse_x = w - 3;
    if (new_mouse_y < 0)
        new_mouse_y = 0;
    if (new_mouse_y >= h - 20)
        new_mouse_y = h - 21;

    mouse_x = new_mouse_x;
    mouse_y = new_mouse_y;
}

void MouseDriver::on_mouse_up()
{
    mouse_press = 0;
}

void MouseDriver::on_mouse_down(int b)
{
    if (b == 9) {
        mouse_press = 1; // Left Click
        return;
    }
    if (b == 10) {
        mouse_press = 2; // Right Click
        return;
    }
    if (b == 12) {
        mouse_press = 3; // Middle Click
        return;
    }
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp)
{
    uint8_t status = commandport.read();
    if (!(status & 0x20) || (active == 0))
        return esp;

    buffer[offset] = dataport.read();
    offset = (offset + 1) % 3;

    if (offset == 0) {
        if (buffer[1] != 0 || buffer[2] != 0) {
            on_mouse_move((int8_t)buffer[1], -((int8_t)buffer[2]));
        }

        for (uint8_t i = 0; i < 3; i++) {
            if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i))) {
                if (buttons & (0x1 << i))
                    on_mouse_up();
                else
                    on_mouse_down(buffer[0]);
            }
        }

        buttons = buffer[0];
    }
    return esp;
}
