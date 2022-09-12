#ifndef PCS_HPP
#define PCS_HPP

#include "../port.hpp"
#include <LibC/types.h>

#define BASE_FREQUENCY 1193180
#define LSB(x) ((x)&0xFF)
#define MSB(x) (((x) >> 8) & 0xFF)

namespace PCS {
void beep_on(uint32_t hz);
void beep_off();
};

#endif
