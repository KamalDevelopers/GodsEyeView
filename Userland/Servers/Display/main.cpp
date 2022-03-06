#include "communication.hpp"
#include "compositor.hpp"
#include "event.hpp"
#include "wm.hpp"
#include <LibC/poll.hpp>

int main(int argc, char** argv)
{
    nice(-1);
    events_files_t event_files = init_events();
    int client_communication_file = init_communications();

    Compositor compositor;
    WindowManager wm(&compositor);
    compositor.load_background_bitmap("bitmaps/wallpaper.raw");
    compositor.load_mouse_bitmap("bitmaps/mouse.raw");
    compositor.update_mouse_position(compositor.screen_width() / 2 + 10, compositor.screen_height() / 2 - 10);
    compositor.require_update();
    compositor.render_stack();

    struct pollfd polls[3];
    polls[0].events = POLLIN;
    polls[0].fd = event_files.mouse;
    polls[1].events = POLLIN;
    polls[1].fd = event_files.keyboard;
    polls[2].events = POLLIN;
    polls[2].fd = client_communication_file;

    while (1) {
        poll(polls, 3);
        send_events(&wm);
        receive_connections(&wm);
        compositor.render_stack();
    }

    return 0;
}
