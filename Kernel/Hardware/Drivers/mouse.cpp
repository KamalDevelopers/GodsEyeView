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

bool MouseDriver::has_unread_event()
{
    if (unread_move_event || unread_click_event)
        return true;
    return false;
}

int MouseDriver::mouse_event(mouse_event_t* event)
{
    if (unread_click_event) {
        memcpy(event, &click_event, sizeof(mouse_event_t));
        unread_click_event = false;
        return sizeof(mouse_event_t);
    }

    if (unread_move_event) {
        memcpy(event, &move_event, sizeof(mouse_event_t));
        unread_move_event = false;
        return sizeof(mouse_event_t);
    }

    return 0;
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

    move_event.x = mouse_x;
    move_event.y = mouse_y;
    move_event.modifier = 0;
    unread_move_event = true;

    TM->test_poll();
    /* yield() = performance increase? */
    TM->yield();
}

void MouseDriver::on_mouse_up()
{
    mouse_press = 0;
}

void MouseDriver::on_mouse_down(int button)
{
    if (button & MOUSE_BUTTON_L)
        mouse_press = MOUSE_MODIFIER_L;
    if (button & MOUSE_BUTTON_R)
        mouse_press = MOUSE_MODIFIER_R;
    if (button & MOUSE_BUTTON_M)
        mouse_press = MOUSE_MODIFIER_M;

    click_event.x = mouse_x;
    click_event.y = mouse_y;
    click_event.modifier = mouse_press;
    unread_click_event = true;
    TM->test_poll();
}

uint32_t MouseDriver::interrupt(uint32_t esp)
{
    uint8_t status = command_port.read();
    if (!(status & 0x20) || (is_active == 0))
        return esp;

    buffer[offset] = data_port.read();
    offset = (offset + 1) % 3;

    if (offset != 0)
        return esp;

    if (buffer[1] != 0 || buffer[2] != 0)
        on_mouse_move((int8_t)buffer[1], -((int8_t)buffer[2]));

    for (uint8_t i = 0; i < 3; i++) {
        if ((buffer[0] & (0x1 << i)) != (buttons & (0x1 << i))) {
            if (buttons & (0x1 << i))
                on_mouse_up();
            else
                on_mouse_down(buffer[0]);
        }
    }

    buttons = buffer[0];
    return esp;
}
