#ifndef WM_HPP
#define WM_HPP

#include "compositor.hpp"
#include "window.hpp"

// clang-format off
#define WORKSPACES 3
#define MAX_LISTENERS 10
#define MAX_VISIBLE_WINDOWS 4
#define WINDOW_TOP_GAP 20
#define WINDOW_GAP 43
#define WINDOW_BORDER_COLOR 0x1E1E1E
#define WINDOW_ACTIVE_BORDER_COLOR 0xc3c6c9
#define CLAMP(a, b, c) (a < b ? b : a > c ? c : a)
// clang-format on

typedef struct window_group {
    Window** windows = 0;
    uint32_t window_count = 0;
    int stored_active_window = -1;
    uint32_t stored_active_tiled = 0;
    uint32_t windows_ptr_size = 0;
} window_group_t;

class WindowManager {
private:
    window_group_t workspaces[WORKSPACES + 1];
    Compositor* compositor;
    Window* global_listener_windows[MAX_LISTENERS];

    int active_fullscreen_window = -1;
    int active_windows = 0;
    int active_window = -1;
    uint32_t tiled_windows = 0;
    uint32_t global_listener_windows_count = 0;

    Window* compose_window(int pid);

public:
    WindowManager(Compositor* compositor);
    ~WindowManager();

    void set_active_window(uint32_t index);
    void require_update(int pid);
    void mouse_event(mouse_event_t* event);
    void keyboard_event(keyboard_event_t* event);
    void send_global_event(uint8_t type, uint16_t d0, uint16_t d1);

    int find_window_with_pid(int pid);
    void update_window_positions();
    void create_window(uint32_t width, uint32_t height, int pid, uint32_t bg, uint8_t flags);
    void destroy_window(uint32_t index);
    void update_window_border(uint32_t index);
    void destroy_window_pid(int pid);
    void sanitizer();

    Window** windows();
    uint32_t windows_size();
    void windows_append(Window* window);
    void windows_remove(uint32_t index);
    void set_workspace(uint16_t workspace);
    void show_all_windows();
    void set_fullscreen_window(uint32_t index);
    void set_window_master(uint32_t index);
};

#endif
