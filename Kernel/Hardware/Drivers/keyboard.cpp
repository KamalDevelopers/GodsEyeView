#include "keyboard.hpp"

/* Mapping */
static char* u_l1 = "QWERTYUIOP";
static char* u_l2 = "ASDFGHJKL";
static char* u_l3 = "ZXCVBNM";
static char* u_nm = "!#¤%&/()=";
static char* l_l1 = "qwertyuiop";
static char* l_l2 = "asdfghjkl";
static char* l_l3 = "zxcvbnm";
static char* l_nm = "123456789";

KeyboardDriver* KeyboardDriver::active = 0;

KeyboardDriver::KeyboardDriver(InterruptManager* interrupt_manager)
    : InterruptHandler(interrupt_manager, 0x21)
    , dataport(0x60)
    , commandport(0x64)
{
    active = this;
    while (commandport.read() & 0x1)
        dataport.read();
    commandport.write(0xae); // activate interrupts
    commandport.write(0x20); // command 0x20 = read controller command byte
    uint8_t status = (dataport.read() | 1) & ~0x10;
    commandport.write(0x60); // command 0x60 = set controller command byte
    dataport.write(status);
    dataport.write(0xf4);
}

KeyboardDriver::~KeyboardDriver()
{
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
        if (is_shift)
            return ':';
        return '.';
    }

    if (key == COMMA_PRESSED) {
        if (is_shift)
            return ';';
        return ',';
    }

    if (key == SLASH_RELEASED) {
        if (is_shift)
            return '_';
        return '-';
    }

    if (key == ZERO_PRESSED)
        return '0';

    if (key == PLUS_PRESSED) {
        if (is_shift)
            return '?';
        return '+';
    }

    /* Row 1 */
    if (key >= ONE_PRESSED && key <= NINE_PRESSED) {
        if (is_shift)
            return u_nm[key - ONE_PRESSED];
        return l_nm[key - ONE_PRESSED];
    }

    /* Row 2 */
    if (key >= Q_PRESSED && key <= ENTER_PRESSED) {
        if (is_shift)
            return u_l1[key - Q_PRESSED];
        return l_l1[key - Q_PRESSED];
    }

    /* Row 3 */
    else if (key >= A_PRESSED && key <= L_PRESSED) {
        if (is_shift)
            return u_l2[key - A_PRESSED];
        return l_l2[key - A_PRESSED];
    }

    /* Row 4 */
    else if (key >= Y_PRESSED && key <= M_PRESSED) {
        if (is_shift)
            return u_l3[key - Y_PRESSED];
        return l_l3[key - Y_PRESSED];
    }
    return 0;
}

int KeyboardDriver::get_key_presses(int raw)
{
    if (raw == 1)
        return keys_pressed_raw;
    return keys_pressed;
}

uint8_t KeyboardDriver::read_key()
{
    uint8_t lastkey = 0;
    if (commandport.read() & 1)
        lastkey = dataport.read();
    return lastkey;
}

char KeyboardDriver::get_key()
{
    uint8_t c = 0;
    while (c == 0) {
        c = read_key();
        if (c == SHIFT_PRESSED)
            is_shift = 1;
        if (c == SHIFT_RELEASED)
            is_shift = 0;
    }
    if (key_a(c) != 0)
        return key_a(c);
    return 0;
}

char KeyboardDriver::get_last_key(int raw)
{
    if (raw == 1)
        return last_key_raw;
    return last_key;
}

void KeyboardDriver::read_keys(int len, char* data)
{
    /* Disable Mouse */
    outb(0x64, 0xD4);
    outb(0x60, 0xF5);

    char c = 0;
    int key_stroke = 0;
    char buffer[512];

    while (c != 10) {
        while (!(c = KeyboardDriver::active->get_key()))
            ;
        if (c == '\b') {
            if (key_stroke > 0) {
                key_stroke--;
                kprintf("%c", c);
            }
        } else {
            buffer[key_stroke] = c;
            key_stroke++;
            kprintf("%c", c);
        }
    }

    len -= len - key_stroke;
    buffer[key_stroke + 1] = '\0';

    strncpy(data, buffer, len);
    data[len - 1] = '\0';

    /* Enable Mouse */
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
}

void KeyboardDriver::on_key(uint8_t keypress)
{
    if (keypress == SHIFT_PRESSED)
        is_shift = 1;
    if (keypress == SHIFT_RELEASED)
        is_shift = 0;

    if (key_a(keypress) != 0) {
        last_key = key_a(keypress);

        if ((last_key == '\b' && keys_pressed - 1 >= 0) || last_key != '\b') {
            TM->append_stdin(last_key);
            keys_pressed += (last_key == '\b') ? -1 : 1;
        }

        if (last_key == 10)
            keys_pressed = 0;
    }

    last_key_raw = keypress;
    keys_pressed_raw++;
}

uint32_t KeyboardDriver::interrupt(uint32_t esp)
{
    uint8_t key = dataport.read();
    on_key(key);
    return esp;
}
