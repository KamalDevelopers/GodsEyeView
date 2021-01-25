#ifndef PCSPK_HPP
#define PCSPK_HPP

#include "LibC/types.hpp"
#include "LibC/stdio.hpp"

namespace PCS {
void play_sound(uint32_t nFrequence);
void nosound();
void beep(uint32_t ms_time, uint32_t frequency);
};


#endif
