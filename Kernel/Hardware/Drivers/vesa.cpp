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

uint32_t Vesa::get_index(uint32_t x, uint32_t y)
{
    return y * screen_pitch + (x * screen_bpp_bytes);
}

void Vesa::write_canvas(canvas_t* canvas)
{
    uint32_t frame_address = get_index(canvas->x, canvas->y) + framebuffer;
    uint32_t canvas_address = (uint32_t)(canvas->framebuffer);

    for (uint32_t y = 0; y < canvas->height; y++) {
        memcpy32((uint32_t*)frame_address, (uint32_t*)canvas_address, canvas->width * screen_bpp_bytes);
        canvas_address += canvas->width * screen_bpp_bytes;
        frame_address += screen_pitch;
    }
}
