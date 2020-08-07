#ifndef MOUSE_HPP
#define MOUSE_HPP

#include "../interrupts.hpp"
#include "../port.hpp"
#include "stdio.hpp"
#include "types.hpp"
#include "vga.hpp"
#include "driver.hpp"

class MouseDriver : public InterruptHandler, public Driver {
    Port8Bit dataport;
    Port8Bit commandport;
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

    uint8_t x, y;
    int32_t w, h;
    uint32_t MouseX, MouseY, MousePress;

public:
    MouseDriver(InterruptManager* manager, int screenw, int screenh);
    ~MouseDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual void OnMouseMove(int x, int y);
    virtual void OnMouseUp(int b);
    virtual void OnMouseDown(int b);
    virtual int GetMouseY() { return MouseY; }
    virtual int GetMouseX() { return MouseX; }
    virtual int GetMousePress() { return MousePress; }
};

#endif
