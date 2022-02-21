#ifndef VESA_HPP
#define VESA_HPP

#include "../../Mem/mm.hpp"
#include <LibC/stdlib.hpp>
#include <LibC/types.hpp>

typedef struct canvas {
    uint32_t* framebuffer = 0;
    uint32_t size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t x = 0;
    uint32_t y = 0;
} canvas_t;

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

    uint32_t get_index(uint32_t x, uint32_t y);
    canvas_t* create_canvas(uint32_t width, uint32_t height);
    void destroy_canvas(canvas_t* canvas);
    void write_canvas(canvas_t* canvas);
};

#endif
