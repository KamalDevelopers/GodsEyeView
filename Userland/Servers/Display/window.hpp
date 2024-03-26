#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <LibC/stdio.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibDisplay/canvas.hpp>
#include <LibDisplay/connection.hpp>
#include <LibDisplay/events.hpp>

class Window {
private:
    canvas_t* canvas = 0;
    int associated_pid = -1;
    int process_send_event_file = 0;
    int associated_workspace = -1;
    int global_listener_id = -1;
    bool controlled = true;
    bool zombie = false;

public:
    Window(int pid = -1);
    ~Window();

    canvas_t* get_canvas() { return canvas; }
    int get_pid() { return associated_pid; }
    int get_controlled() { return controlled; }
    int get_workspace() { return associated_workspace; }
    bool get_zombie() { return zombie; }
    int get_global_listener_id() { return global_listener_id; }
    int get_is_global_listener();

    void die();
    void disown();
    void adopt(int workspace);
    void set_global_listener_id(int id);
    void set_position(uint32_t x, uint32_t y);
    void set_workspace(int workspace);
    void resize(uint32_t width, uint32_t height);

    bool is_point_in_window(uint32_t x, uint32_t y);
    void create_process_connection();
    void mouse_event(mouse_event_t* event);
    void keyboard_event(keyboard_event_t* event);
    void global_event(global_wm_event_t* event);
    void resize_event(canvas_t* canvas);
};

#endif
