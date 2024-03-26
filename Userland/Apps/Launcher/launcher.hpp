#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <LibC/poll.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibDisplay/connection.hpp>
#include <LibFont/font.hpp>

class Launcher {
private:
    int window_events_file = -1;
    int shell_pid = -1;
    int active_workspace = 0;
    int max_workspaces = 3;
    bool is_running = false;
    uint32_t width = 0;
    uint32_t height = 0;
    const uint32_t bg = 0x65080808;
    const uint32_t fg = 0xA8A9AD;
    font_t* default_font;
    canvas_t window_canvas;

public:
    Launcher();
    ~Launcher();

    void clear();
    void spawns();
    void display_time();
    void display_workspace();
    void display_cpu_usage();
    uint32_t display_string(const char* text, int pos_x, int pos_y);
    void resize_window(display_event_t* display_event);
    void receive_keyboard_event(display_event_t* display_event);
    void receive_global_event(display_event_t* display_event);
    void receive_events();
    void run();
};

#endif
