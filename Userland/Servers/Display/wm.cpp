#include "wm.hpp"

WindowManager::WindowManager(Compositor* compositor)
{
    this->compositor = compositor;
}

WindowManager::~WindowManager()
{
}

int WindowManager::find_window_with_pid(int pid)
{
    for (uint32_t i = 0; i < window_index; i++)
        if (windows[i]->get_pid() == pid)
            return i;
    return -1;
}

void WindowManager::mouse_event(mouse_event_t* event)
{
    if (event->modifier == 1) {
        for (uint32_t i = 0; i < window_index; i++)
            if (windows[i]->is_point_in_window(event->x, event->y))
                active_window = i;
    }

    if (active_window != -1)
        windows[active_window]->mouse_event(event);
    if (!event->modifier)
        compositor->update_mouse_position(event->x, event->y);
}

void WindowManager::keyboard_event(keyboard_event_t* event)
{
    if (active_window != -1)
        windows[active_window]->keyboard_event(event);
}

void WindowManager::update_window_positions()
{
    uint32_t section = SCREEN_WIDTH;
    if (window_index > 1)
        section = SCREEN_WIDTH / window_index;

    for (uint32_t index = 0; index < window_index; index++) {
        uint32_t height = SCREEN_HEIGHT - WINDOW_GAP * 2;
        uint32_t width = section - WINDOW_GAP * 2;
        uint32_t x = section * index + WINDOW_GAP;
        uint32_t y = WINDOW_GAP;

        windows[index]->resize(width, height);
        windows[index]->set_position(x, y);
    }
    compositor->require_update();
}

Window* WindowManager::compose_window(int pid)
{
    Window* window = new Window(pid);
    return window;
}

void WindowManager::create_window(uint32_t width, uint32_t height, int pid)
{
    Window* window = compose_window(pid);
    windows[window_index] = window;

    active_window = window_index;
    window_index++;
    update_window_positions();

    compositor->add_render_layer(window->get_canvas());
    window->create_process_connection();
}

void WindowManager::destroy_window(uint32_t index)
{
    compositor->remove_render_layer(windows[index]->get_canvas());
    windows[index]->~Window();
    if (index == active_window) {
        if (index == window_index - 1)
            active_window = index - 1;
    }

    free(windows[index]);
    delete_element(index, window_index, windows);
    window_index--;

    if (!window_index)
        active_window = -1;
}

void WindowManager::destroy_window_pid(int pid)
{
    int window_index = find_window_with_pid(pid);
    if (window_index == -1)
        return;
    destroy_window(window_index);
    update_window_positions();
    compositor->require_update();
}
