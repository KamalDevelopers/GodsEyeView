#include "vesa.hpp"

Vesa::Vesa(uint64_t framebuffer_address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp)
{
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

canvas_t* Vesa::create_canvas(uint32_t width, uint32_t height)
{
    canvas_t* canvas = (canvas_t*)kmalloc(sizeof(canvas_t));
    canvas->size = height * width * screen_bpp_bytes;
    canvas->framebuffer = (uint32_t*)kmalloc(canvas->size);
    canvas->x = 0;
    canvas->y = 0;
    canvas->width = width;
    canvas->height = height;
    return canvas;
}

void Vesa::destroy_canvas(canvas_t* canvas)
{
    kfree(canvas->framebuffer);
    kfree(canvas);
}

void Vesa::write_canvas(canvas_t* canvas)
{
    uint32_t frame_address = get_index(canvas->x, canvas->y) + framebuffer;
    uint32_t canvas_address = (uint32_t)(canvas->framebuffer);

    for (uint32_t y = 0; y < canvas->height; y++) {
        memcpy((void*)frame_address, (void*)(canvas_address), canvas->width * screen_bpp_bytes);
        canvas_address += canvas->width * screen_bpp_bytes;
        frame_address += screen_pitch;
    }
}
