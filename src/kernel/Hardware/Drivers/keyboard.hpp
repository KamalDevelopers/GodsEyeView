#ifndef __KEYBOARD_HPP
#define __KEYBOARD_HPP

#include "LibC/types.hpp"
#include "../interrupts.hpp"
#include "../port.hpp"
#include "LibC/stdio.hpp"

class KeyboardDriver : public InterruptHandler
{
    Port8Bit dataport;
    Port8Bit commandport;
public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    virtual uint32_t HandleInterrupt(uint32_t esp);
};

#endif