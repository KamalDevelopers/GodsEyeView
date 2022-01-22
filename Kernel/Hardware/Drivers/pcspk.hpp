#ifndef PCSPK_HPP
#define PCSPK_HPP

#include "../port.hpp"
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>

namespace PCS {
void beep_start(uint32_t frequency);
void beep_stop();
};

#endif
