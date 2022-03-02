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

void WindowManager::set_active_window(uint32_t index)
{
    uint32_t old_active_window = active_window;
    active_window = index;
    if (active_window != -1)
        update_window_border(active_window);
    if (old_active_window != -1)
        update_window_border(old_active_window);
    compositor->require_update();
}

void WindowManager::mouse_event(mouse_event_t* event)
{
    if (event->modifier == 1) {
        for (uint32_t i = 0; i < window_index; i++)
            if (windows[i]->is_point_in_window(event->x, event->y))
                set_active_window(i);
    }

    if (active_window != -1)
        windows[active_window]->mouse_event(event);
    if (!event->modifier)
        compositor->update_mouse_position(event->x, event->y);
}

void WindowManager::keyboard_event(keyboard_event_t* event)
{
    if ((event->key == 't') && (event->modifier == 2)) {
        spawn((char*)"bin/terminal", 0);
        return;
    }

    if (active_window != -1)
        windows[active_window]->keyboard_event(event);
}

void WindowManager::require_update(int pid)
{
    int window_index = find_window_with_pid(pid);
    if (window_index == -1)
        return;

    update_window_border(window_index);
    compositor->require_update();
}

void WindowManager::update_window_border(uint32_t index)
{
    windows[index]->get_canvas()->border_decoration = (index == active_window) ? WINDOW_ACTIVE_BORDER_COLOR : WINDOW_BORDER_COLOR;
}

void WindowManager::update_window_positions()
{
    uint32_t position_x = WINDOW_GAP;
    uint32_t position_y = WINDOW_GAP;
    uint32_t windows_tile_vertical = 2;
    uint32_t tile_vertical_max = clamp<int>(window_index, 1, windows_tile_vertical);
    uint32_t tile_horizontal_max = (window_index > windows_tile_vertical) ? window_index - 1 : 1;
    uint32_t vertical_section = SCREEN_WIDTH / tile_vertical_max;
    uint32_t horizontal_section = SCREEN_HEIGHT / tile_horizontal_max;

    for (uint32_t index = 0; index < window_index; index++) {
        uint32_t height = horizontal_section - WINDOW_GAP * 2;
        uint32_t width = vertical_section - WINDOW_GAP;

        if (index >= tile_vertical_max - 1)
            width -= WINDOW_GAP;
        if (index == 0)
            height = SCREEN_HEIGHT - WINDOW_GAP * 2;
        if (index > windows_tile_vertical)
            height += WINDOW_GAP;

        windows[index]->resize(width, height);
        windows[index]->set_position(position_x, position_y);

        if (windows_tile_vertical > 1) {
            position_x += width + WINDOW_GAP;
            windows_tile_vertical--;
        } else {
            position_y += height + WINDOW_GAP;
        }
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

    window_index++;
    update_window_positions();

    compositor->add_render_layer(window->get_canvas());
    window->create_process_connection();

    set_active_window(window_index - 1);
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
