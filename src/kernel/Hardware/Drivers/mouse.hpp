#ifndef __MOUSE_HPP
#define __MOUSE_HPP

#include "LibC/stdio.hpp"
#include "LibC/types.hpp"
#include "../interrupts.hpp"
#include "../port.hpp"
#include "vga.hpp"

class MouseDriver : public InterruptHandler
{
    Port8Bit dataport;
    Port8Bit commandport;
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

    uint8_t x, y;
    int32_t w, h;
    uint32_t MouseX, MouseY;
public:
    MouseDriver(InterruptManager* manager, Graphics* v);
    ~MouseDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
    virtual void OnMouseMove(int x, int y);
private:
	Graphics* vga;
};

#endif