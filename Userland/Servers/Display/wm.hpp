#ifndef WM_HPP
#define WM_HPP

#include "compositor.hpp"
#include "window.hpp"
#include <LibC/stdlib.hpp>

#define MAX_WINDOWS 100
#define WINDOW_GAP 15

class WindowManager {
private:
    Window* windows[MAX_WINDOWS];
    Compositor* compositor;

    uint32_t window_index = 0;
    uint32_t active_window = -1;

    Window* compose_window(int pid);

public:
    WindowManager(Compositor* compositor);
    ~WindowManager();

    void require_update() { compositor->require_update(); }
    void mouse_event(mouse_event_t* event);
    void keyboard_event(keyboard_event_t* event);

    int find_window_with_pid(int pid);
    void update_window_positions();
    void create_window(uint32_t width, uint32_t height, int pid);
    void destroy_window(uint32_t index);
    void destroy_window_pid(int pid);
};

#endif
