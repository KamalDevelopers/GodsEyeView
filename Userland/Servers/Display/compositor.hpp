#ifndef COMPOSITOR_HPP
#define COMPOSITOR_HPP

#include "window.hpp"
#include <LibC++/vector.hpp>
#include <LibC/types.h>
#include <LibDisplay/canvas.hpp>
#include <LibDisplay/connection.hpp>
#include <LibDisplay/events.hpp>

#define MAX_LAYERS 50
#define MOUSE_WIDTH 12
#define MOUSE_HEIGHT 18

class Compositor {
private:
    uint32_t* mouse_bitmap = 0;
    uint32_t display_framebuffer = 0;
    uint32_t display_width = 0;
    uint32_t display_height = 0;
    bool needs_update = false;

    canvas_t display_layer;
    canvas_t* final_layer = 0;
    canvas_t* root_layer = 0;
    canvas_t* mouse_layer = 0;
    canvas_t* mouse_ghost_layer = 0;
    Vector<canvas_t*, MAX_LAYERS> layers;

public:
    Compositor();
    ~Compositor();

    uint32_t screen_width() { return display_width; }
    uint32_t screen_height() { return display_height; }
    void require_update() { needs_update = true; }

    void load_background_bitmap(const char* file_name, bool is_compressed = false);
    void load_mouse_bitmap(const char* file_name);
    int read_bitmap(const char* file_name, canvas_t* canvas);
    int read_compressed_bitmap(const char* file_name, canvas_t* canvas);
    void update_mouse_position(uint32_t x, uint32_t y, bool is_updating_stack = false);

    void render_borders(canvas_t* canvas);
    void render_canvas(canvas_t* canvas);
    void render_stack();

    void add_render_layer(canvas_t* canvas);
    int remove_render_layer(canvas_t* canvas);
};

#endif
