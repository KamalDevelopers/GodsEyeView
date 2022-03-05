#ifndef WM_HPP
#define WM_HPP

#include "compositor.hpp"
#include "window.hpp"
#include <LibC/stdlib.hpp>
#include <LibC/unistd.hpp>

// clang-format off
#define MAX_WINDOWS 100
#define WINDOW_GAP 12
#define WINDOW_BORDER_COLOR 0x1E1E1E
#define WINDOW_ACTIVE_BORDER_COLOR 0x70C2FF
#define CLAMP(a, b, c) (a < b ? b : a > c ? c : a)
// clang-format on

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

    void set_active_window(uint32_t index);
    void require_update(int pid);
    void mouse_event(mouse_event_t* event);
    void keyboard_event(keyboard_event_t* event);

    int find_window_with_pid(int pid);
    void update_window_positions();
    void create_window(uint32_t width, uint32_t height, int pid);
    void destroy_window(uint32_t index);
    void update_window_border(uint32_t index);
    void destroy_window_pid(int pid);
};

#endif
