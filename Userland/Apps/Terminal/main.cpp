#include "draw.hpp"
#include <LibC/poll.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibDisplay/connection.hpp>

int main(int argc, char** argv)
{
    canvas_t canvas;
    int events_file = request_display_window(&canvas, 700, 500);
    if (events_file == -1)
        return 0;

    canvas_set(canvas.framebuffer, 0x080808, canvas.size);
    int pid = spawn((char*)"shell", 0);
    request_update_window();

    keyboard_event_t keyboard_event;
    char key_input[2];
    int keys_pressed = 0;
    memset(key_input, 0, sizeof(key_input));

    char stdout_buffer[1024];
    bool continuous_stdout = false;
    memset(stdout_buffer, 0, sizeof(stdout_buffer));

    while (1) {
        display_event_t event;
        if (receive_event(&event)) {
            if (event.type == DISPLAY_EVENT_KEYBOARD) {
                memcpy(&keyboard_event, &event.keyboard, sizeof(keyboard_event_t));
                if (keyboard_event.key == 27)
                    break;
                if (keyboard_event.key == 10)
                    keys_pressed = -1;
                if ((keyboard_event.key == '\b') && (keys_pressed <= 0))
                    continue;

                keys_pressed += (keyboard_event.key == '\b') ? -1 : 1;
                write(0, &keyboard_event.key, 1);
                flush();
            }

            if (event.type == DISPLAY_EVENT_RESIZE) {
                memcpy(&canvas, event.canvas, sizeof(canvas_t));
                clear_text(&canvas);
            }
        }

        if (read(1, stdout_buffer, sizeof(stdout_buffer))) {
            draw_text(&canvas, stdout_buffer);
            continuous_stdout = true;
        } else if (continuous_stdout) {
            continuous_stdout = false;
            request_update_window();
        }
    }

    request_destroy_window();
    return 0;
}
