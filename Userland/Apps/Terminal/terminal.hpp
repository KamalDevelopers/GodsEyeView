#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "draw.hpp"
#include <LibC/poll.h>
#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>
#include <LibC/unistd.h>
#include <LibDisplay/connection.hpp>

class Terminal {
private:
    char stdout_buffer[BUFSIZ];
    int window_events_file = -1;
    int keys_pressed = -1;
    int shell_pid = -1;
    bool is_running = false;
    canvas_t window_canvas;

public:
    Terminal();
    ~Terminal();

    void tty_master();
    void spawn_shell();
    void kill_shell();
    void resize_window(display_event_t* display_event);
    void receive_keyboard_event(display_event_t* display_event);
    void receive_events();
    void receive_stdout();
    void run();
};

#endif
