#include "mouse.hpp"

MouseDriver* MouseDriver::active = 0;
MouseDriver::MouseDriver(InterruptManager* manager, int screenw, int screenh)
    : InterruptHandler(manager, 0x2C)
    , data_port(0x60)
    , command_port(0x64)
{
    active = this;
    offset = 0;
    buttons = 0;
    w = screenw;
    h = screenh;
    mouse_x = w / 2;
    mouse_y = h / 2;

    command_port.write(0xA8);
    command_port.write(0x20);
    uint8_t status = data_port.read() | 2;
    command_port.write(0x60);
    data_port.write(status);

    command_port.write(0xD4);
    data_port.write(0xF4);
    data_port.read();
}

MouseDriver::~MouseDriver()
{
}

mouse_event_t* MouseDriver::get_mouse_event()
{
    if (has_read_event)
        return 0;
    event.modifier = 0;
    if (!has_read_klick)
        event.modifier = mouse_press;
    event.x = mouse_x;
    event.y = mouse_y;
    has_read_event = true;
    has_read_klick = true;
    return &event;
}

void MouseDriver::on_mouse_move(int x, int y)
{
    int32_t new_mouse_x = mouse_x + x;
    int32_t new_mouse_y = mouse_y + y;

    if (new_mouse_x < 0)
        new_mouse_x = 0;
    if (new_mouse_x >= w)
        new_mouse_x = w;
    if (new_mouse_y < 0)
        new_mouse_y = 0;
    if (new_mouse_y >= h)
        new_mouse_y = h;

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
    }
    if (b == 10) {
        mouse_press = 2; // Right Click
    }
    if (b == 12) {
        mouse_press = 3; // Middle Click
    }
}

uint32_t MouseDriver::interrupt(uint32_t esp)
{
    uint8_t status = command_port.read();
    if (!(status & 0x20) || (is_active == 0))
        return esp;

    has_read_event = false;
    TM->test_poll();
    buffer[offset] = data_port.read();
    offset = (offset + 1) % 3;

    if (offset == 0) {
        if (buffer[1] != 0 || buffer[2] != 0)
            on_mouse_move((int8_t)buffer[1], -((int8_t)buffer[2]));

        for (uint8_t i = 0; i < 3; i++) {
            if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i))) {
                has_read_klick = false;
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
