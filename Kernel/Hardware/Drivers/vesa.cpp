#include "vesa.hpp"

Vesa* Vesa::active = 0;
Vesa::Vesa(uint64_t framebuffer_address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp)
{
    active = this;
    framebuffer = framebuffer_address;
    screen_width = width;
    screen_height = height;
    screen_bpp = bpp;
    screen_bpp_bytes = bpp / 8;
    screen_pitch = pitch;

    for (uint32_t i = 0; i < width * height * screen_bpp_bytes; i += PAGE_SIZE)
        Paging::map_page(framebuffer + i, framebuffer + i);
}

Vesa::~Vesa()
{
}
