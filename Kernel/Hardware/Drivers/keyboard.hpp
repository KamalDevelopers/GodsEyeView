#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/types.h>
#include <LibDisplay/events.hpp>

#define MAX_KEYBOARD_EVENTS 100
#define KEYBOARD_MODIFIER_SHIFT 1
#define KEYBOARD_MODIFIER_CTRL 2
#define KEYBOARD_MODIFIER_ALTGR 3

class KeyboardDriver : public InterruptHandler {
    Port8Bit data_port;
    Port8Bit command_port;

private:
    int x_offset = 0;
    int y_offset = 0;

    int keys_pressed = 0;
    int keys_pressed_raw = 0;

    keyboard_event_t events[MAX_KEYBOARD_EVENTS];
    uint32_t events_index = 0;
    uint32_t current_event = 0;

    char last_key = 0;
    uint8_t last_key_raw = 0;
    uint32_t modifier = 0;

    void on_key(uint8_t keypress);
    uint8_t key_a(uint8_t key);

public:
    KeyboardDriver(InterruptManager* interrupt_manager);
    ~KeyboardDriver();

    static KeyboardDriver* active;

    virtual uint32_t interrupt(uint32_t esp) override;
    uint8_t read_key();

    bool has_unread_event();
    int keyboard_event(keyboard_event_t* event);
};

/* Currently Swedish Layout */

enum KEYCODE {
    NULL_KEY = 0,
    Q_PRESSED = 0x10,
    Q_RELEASED = 0x90,
    W_PRESSED = 0x11,
    W_RELEASED = 0x91,
    E_PRESSED = 0x12,
    E_RELEASED = 0x92,
    R_PRESSED = 0x13,
    R_RELEASED = 0x93,
    T_PRESSED = 0x14,
    T_RELEASED = 0x94,
    Z_PRESSED = 0x15,
    Z_RELEASED = 0x95,
    U_PRESSED = 0x16,
    U_RELEASED = 0x96,
    I_PRESSED = 0x17,
    I_RELEASED = 0x97,
    O_PRESSED = 0x18,
    O_RELEASED = 0x98,
    P_PRESSED = 0x19,
    P_RELEASED = 0x99,
    A_PRESSED = 0x1E,
    A_RELEASED = 0x9E,
    S_PRESSED = 0x1F,
    S_RELEASED = 0x9F,
    D_PRESSED = 0x20,
    D_RELEASED = 0xA0,
    F_PRESSED = 0x21,
    F_RELEASED = 0xA1,
    G_PRESSED = 0x22,
    G_RELEASED = 0xA2,
    H_PRESSED = 0x23,
    H_RELEASED = 0xA3,
    J_PRESSED = 0x24,
    J_RELEASED = 0xA4,
    K_PRESSED = 0x25,
    K_RELEASED = 0xA5,
    L_PRESSED = 0x26,
    L_RELEASED = 0xA6,
    Y_PRESSED = 0x2C,
    Y_RELEASED = 0xAC,
    X_PRESSED = 0x2D,
    X_RELEASED = 0xAD,
    C_PRESSED = 0x2E,
    C_RELEASED = 0xAE,
    V_PRESSED = 0x2F,
    V_RELEASED = 0xAF,
    B_PRESSED = 0x30,
    B_RELEASED = 0xB0,
    N_PRESSED = 0x31,
    N_RELEASED = 0xB1,
    M_PRESSED = 0x32,
    M_RELEASED = 0xB2,

    COMMA_PRESSED = 0xB3,
    ZERO_PRESSED = 0x0B,
    ONE_PRESSED = 0x2,
    NINE_PRESSED = 0xA,
    PLUS_PRESSED = 0x0C,
    PLUS_RELEASED = 0x8C,
    PIPE_PRESSED = 0x56,
    CARET_PRESSED = 0x2B,
    TAB_PRESSED = 0xF,
    TAB_RELEASED = 0x8F,

    POINT_PRESSED = 0x34,
    POINT_RELEASED = 0xB4,
    SLASH_RELEASED = 0xB5,

    BACKSPACE_PRESSED = 0xE,
    BACKSPACE_RELEASED = 0x8E,
    SPACE_PRESSED = 0x39,
    SPACE_RELEASED = 0xB9,
    ENTER_PRESSED = 0x1C,
    ENTER_RELEASED = 0x9C,

    SHIFT_PRESSED = 0x2A,
    SHIFT_RELEASED = 0xAA,
    CTRL_PRESSED = 0x1D,
    CTRL_RELEASED = 0x9D,
    ALTGR_PRESSED = 0x38,
    ALTGR_RELEASED = 0xB8
};

#endif
