#ifndef PANIC_HPP
#define PANIC_HPP

#include "Hardware/interrupts.hpp"
#include "tty.hpp"

#define PANIC(msg) panic(msg, __FILE__, __LINE__)

[[noreturn]] void panic(char* error, const char* file, uint32_t line);

#endif
