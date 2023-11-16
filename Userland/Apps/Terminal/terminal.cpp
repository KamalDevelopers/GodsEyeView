#include "terminal.hpp"

Terminal::Terminal()
{
    tty_master();
    memset(stdout_buffer, 0, sizeof(stdout_buffer));

    init("bitmaps/font.tftf");
    spawn_shell();

    window_events_file = request_display_window(window_canvas, 700, 500, BACKGROUND_COLOR);
    receive_stdout();
    request_update_window();
}

Terminal::~Terminal()
{
    uninit();
    request_destroy_window();
    kill(0, 2);
}

void Terminal::tty_master()
{
    fchown(0, 1, 0);
    setsid();
}

void Terminal::spawn_shell()
{
    shell_pid = spawn("bin/shell", 0, 0);
    sys_yield();
}

void Terminal::kill_shell()
{
    kill(0, 2);
    draw_text(&window_canvas, "\n");
}

void Terminal::resize_window(display_event_t* display_event)
{
    if (window_canvas.height == display_event->canvas.height && window_canvas.width == display_event->canvas.width)
        return;
    window_canvas = display_event->canvas;
    resize_text(&window_canvas);
    request_update_window();
}

void Terminal::receive_keyboard_event(display_event_t* display_event)
{
    keyboard_event_t keyboard_event;
    memcpy(&keyboard_event, &display_event->keyboard, sizeof(keyboard_event_t));

    if (keyboard_event.state == KEY_UP)
        return;

    if (keyboard_event.key == 27) {
        is_running = false;
        return;
    }

    if (keyboard_event.key == 10)
        keys_pressed = -1;

    if (keyboard_event.key == '\b') {
        if (keys_pressed <= 0)
            return;
        keys_pressed -= 2;
    }

    if ((keyboard_event.key == 'c') && (keyboard_event.modifier == 2)) {
        keyboard_event.key = '\b';
        for (uint32_t i = 0; i < keys_pressed + 1; i++)
            write(0, &keyboard_event.key, 1);
        keys_pressed = 0;
        kill_shell();
        spawn_shell();
        return;
    }

    if (keys_pressed == -1)
        keys_pressed = 0;

    keys_pressed++;
    write(0, &keyboard_event.key, 1);
}

void Terminal::receive_events()
{
    display_event_t display_event;
    if (receive_window_event(&display_event)) {
        if (display_event.type == DISPLAY_EVENT_KEYBOARD)
            return receive_keyboard_event(&display_event);
        if (display_event.type == DISPLAY_EVENT_RESIZE)
            return resize_window(&display_event);
    }
}

void Terminal::receive_stdout()
{
    if (read(1, stdout_buffer, sizeof(stdout_buffer) - 1)) {
        draw_text(&window_canvas, stdout_buffer);
        memset(stdout_buffer, 0, sizeof(stdout_buffer));
        skip_frame_render = 0;
    }
}

void Terminal::run()
{
    is_running = true;
    struct pollfd polls[2];
    polls[0].events = POLLIN;
    polls[0].fd = 1;
    polls[1].events = POLLIN;
    polls[1].fd = window_events_file;

    while (1) {
        skip_frame_render = 1;
        poll(polls, 2, 0);
        receive_events();
        receive_stdout();

        if (!skip_frame_render)
            request_update_window();

        if (!is_running)
            return;
    }
}
