#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <LibC/poll.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibDisplay/connection.hpp>

class Launcher {
private:
    int window_events_file = -1;
    int shell_pid = -1;
    bool is_running = false;
    uint8_t* font_buffer = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    canvas_t window_canvas;

public:
    Launcher();
    ~Launcher();

    void clear();
    void display_time();
    uint32_t display_string(const char* text, int pos_x, int pos_y);
    void resize_window(display_event_t* display_event);
    void receive_keyboard_event(display_event_t* display_event);
    void receive_events();
    void run();
};

#endif
