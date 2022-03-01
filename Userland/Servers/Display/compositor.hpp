#ifndef COMPOSITOR_HPP
#define COMPOSITOR_HPP

#include "window.hpp"
#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibDisplay/canvas.hpp>
#include <LibDisplay/connection.hpp>
#include <LibDisplay/events.hpp>

#define MAX_LAYERS 100
#define MOUSE_WIDTH 12
#define MOUSE_HEIGHT 18
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

class Compositor {
private:
    canvas_t* layers[MAX_LAYERS];

    uint32_t layer_index = 0;
    uint32_t* mouse_bitmap = 0;
    uint32_t display_framebuffer = 0;
    bool needs_update = false;

    canvas_t display_layer;
    canvas_t* final_layer = 0;
    canvas_t* root_layer = 0;
    canvas_t* mouse_layer = 0;
    canvas_t* mouse_ghost_layer = 0;

public:
    Compositor();
    ~Compositor();

    uint32_t screen_width() { return SCREEN_WIDTH; }
    uint32_t screen_height() { return SCREEN_HEIGHT; }
    void require_update() { needs_update = true; }

    void load_background_bitmap(const char* file_name);
    void load_mouse_bitmap(const char* file_name);
    int read_bitmap(const char* file_name, canvas_t* canvas);
    void update_mouse_position(uint32_t x, uint32_t y, bool is_updating_stack = false);

    void render_canvas(canvas_t* canvas);
    void render_stack();

    void add_render_layer(canvas_t* canvas);
    int remove_render_layer(canvas_t* canvas);
};

#endif
