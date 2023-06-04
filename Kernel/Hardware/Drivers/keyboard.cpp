#include "keyboard.hpp"

/* Mapping */
static char* altgr_l1 = "@ſ€®þ←↓→œþ";
static char* altgr_l2 = "ªßðđŋħ̉ĸł";
static char* altgr_l3 = "«»©„“”µ";
static char* altgr_l0 = "!@?$??{[]}";
static char* shift_l1 = "QWERTYUIOP";
static char* shift_l2 = "ASDFGHJKL";
static char* shift_l3 = "ZXCVBNM";
static char* shift_l0 = "!\"#?%&/()=";
static char* l1 = "qwertyuiop";
static char* l2 = "asdfghjkl";
static char* l3 = "zxcvbnm";
static char* l0 = "123456789";

KeyboardDriver* KeyboardDriver::active = 0;

KeyboardDriver::KeyboardDriver(InterruptManager* interrupt_manager)
    : InterruptHandler(interrupt_manager, 0x21)
    , data_port(0x60)
    , command_port(0x64)
{
    active = this;
    while (command_port.read() & 0x1)
        data_port.read();
    command_port.write(0xae);
    command_port.write(0x20);
    uint8_t status = (data_port.read() | 1) & ~0x10;
    command_port.write(0x60);
    data_port.write(status);
    data_port.write(0xf4);
}

KeyboardDriver::~KeyboardDriver()
{
}

bool KeyboardDriver::has_unread_event()
{
    if (current_event >= events_index)
        return false;
    return true;
}

int KeyboardDriver::keyboard_event(keyboard_event_t* event)
{
    if (current_event >= events_index)
        return 0;

    memcpy(event, &events[current_event], sizeof(keyboard_event_t));
    current_event++;

    if (current_event >= events_index) {
        current_event = 0;
        events_index = 0;
    }
    return sizeof(keyboard_event_t);
}

/* TODO: English layout & more character support */
uint8_t KeyboardDriver::key_a(uint8_t key)
{
    if (key == ENTER_PRESSED)
        return '\n';

    if (key == SPACE_PRESSED)
        return ' ';

    if (key == BACKSPACE_PRESSED)
        return '\b';

    if (key == POINT_RELEASED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return ':';
        return '.';
    }

    if (key == COMMA_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return ';';
        return ',';
    }

    if (key == SLASH_RELEASED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return '_';
        return '-';
    }

    if (key == ZERO_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return shift_l0[9];
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return altgr_l0[9];
        return '0';
    }

    if (key == PLUS_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return '?';
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return '\\';
        return '+';
    }

    if (key == CARET_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return '*';
        return '\'';
    }

    if (key == PIPE_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return '>';
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return '|';
        return '<';
    }

    /* Row 1 */
    if (key >= ONE_PRESSED && key <= NINE_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return altgr_l0[key - ONE_PRESSED];
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return shift_l0[key - ONE_PRESSED];
        return l0[key - ONE_PRESSED];
    }

    /* Row 2 */
    if (key >= Q_PRESSED && key <= ENTER_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return altgr_l1[key - Q_PRESSED];
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return shift_l1[key - Q_PRESSED];
        return l1[key - Q_PRESSED];
    }

    /* Row 3 */
    else if (key >= A_PRESSED && key <= L_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return altgr_l2[key - A_PRESSED];
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return shift_l2[key - A_PRESSED];
        return l2[key - A_PRESSED];
    }

    /* Row 4 */
    else if (key >= Y_PRESSED && key <= M_PRESSED) {
        if (modifier == KEYBOARD_MODIFIER_ALTGR)
            return altgr_l3[key - Y_PRESSED];
        if (modifier == KEYBOARD_MODIFIER_SHIFT)
            return shift_l3[key - Y_PRESSED];
        return l3[key - Y_PRESSED];
    }

    /* ESC */
    if (key == 1)
        return 27;

    if (key == TAB_PRESSED)
        return 9;

    return 0;
}

uint8_t KeyboardDriver::read_key()
{
    uint8_t lastkey = 0;
    if (command_port.read() & 1)
        lastkey = data_port.read();
    return lastkey;
}

void KeyboardDriver::on_key(uint8_t keypress)
{
    if (keypress == SHIFT_PRESSED)
        modifier = KEYBOARD_MODIFIER_SHIFT;
    if (keypress == SHIFT_RELEASED)
        modifier = 0;
    if (keypress == ALTGR_PRESSED)
        modifier = KEYBOARD_MODIFIER_ALTGR;
    if (keypress == ALTGR_RELEASED)
        modifier = 0;
    if (keypress == CTRL_PRESSED)
        modifier = KEYBOARD_MODIFIER_CTRL;
    if (keypress == CTRL_RELEASED)
        modifier = 0;

    last_key = key_a(keypress);
    if (last_key != 0) {
        events[events_index].key = last_key;
        events[events_index].modifier = modifier;
        events_index++;

        if (events_index >= MAX_KEYBOARD_EVENTS) {
            events_index = 0;
            current_event = 0;
        } else {
            TM->test_poll();
        }
    }

    keys_pressed += (last_key == '\b') ? -1 : 1;
    if (last_key == 10)
        keys_pressed = 0;
    last_key_raw = keypress;
    keys_pressed_raw++;
}

uint32_t KeyboardDriver::interrupt(uint32_t esp)
{
    uint8_t key = data_port.read();
    on_key(key);
    return esp;
}
