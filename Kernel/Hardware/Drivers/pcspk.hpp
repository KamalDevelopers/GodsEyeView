#ifndef PCSPK_HPP
#define PCSPK_HPP

#include "../port.hpp"
#include "LibC/stdio.hpp"
#include "LibC/types.hpp"

namespace PCS {
void play_sound(uint32_t frequency);
void nosound();
void beep(uint32_t ms_time, uint32_t frequency);
};

#endif
