#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <LibC/stdio.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibDisplay/canvas.hpp>
#include <LibDisplay/connection.hpp>
#include <LibDisplay/events.hpp>

class Window {
private:
    canvas_t* canvas = 0;
    int associated_pid = -1;
    int process_send_event_file = 0;

public:
    Window(int pid=-1);
    ~Window();

    canvas_t* get_canvas() { return canvas; }
    int get_pid() { return associated_pid; }

    void set_position(uint32_t x, uint32_t y);
    void resize(uint32_t width, uint32_t height);

    bool is_point_in_window(uint32_t x, uint32_t y);
    void create_process_connection();
    void mouse_event(mouse_event_t* event);
    void keyboard_event(keyboard_event_t* event);
    void resize_event(canvas_t* canvas);
};

#endif
