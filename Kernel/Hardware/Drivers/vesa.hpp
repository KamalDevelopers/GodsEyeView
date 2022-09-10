#ifndef VESA_HPP
#define VESA_HPP

#include "../../Mem/mm.hpp"
#include "../interrupts.hpp"
#include <LibC/stdlib.hpp>
#include <LibC/types.hpp>

#define VESA Vesa::active

class Vesa {
private:
    uint64_t framebuffer;
    uint32_t screen_width = 0;
    uint32_t screen_height = 0;
    uint32_t screen_pitch = 0;
    uint32_t screen_bpp = 0;
    uint32_t screen_bpp_bytes = 0;

public:
    Vesa(uint64_t framebuffer_address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp);
    ~Vesa();

    static Vesa* active;
    uint32_t get_screen_width() { return screen_width; }
    uint32_t get_screen_height() { return screen_height; }
    uint32_t get_framebuffer() { return framebuffer; }
};

#endif
