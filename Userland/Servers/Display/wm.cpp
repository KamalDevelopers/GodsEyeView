#include "wm.hpp"
#include <LibC/mem.h>
#include <LibC/stdlib.h>
#include <LibC/unistd.h>

WindowManager::WindowManager(Compositor* compositor)
{
    active_fullscreen_window = -1;
    this->compositor = compositor;
    /* workspaces = (window_group_t*)malloc(sizeof(window_group_t) * (WORKSPACES + 1));
    memset(workspaces, 0, sizeof(window_group_t) * (WORKSPACES + 1)); */

    for (uint16_t i = 0; i < WORKSPACES; i++) {
        workspaces[i].windows = (Window**)malloc(sizeof(Window*) * 50);
        workspaces[i].windows_ptr_size = 50;
        memset(workspaces[i].windows, 0, sizeof(Window*));
        workspaces[i].stored_active_window = -1;
    }
}

WindowManager::~WindowManager()
{
    for (uint16_t i = 0; i < WORKSPACES; i++)
        free(workspaces[i].windows);
}

int WindowManager::find_window_with_pid(int pid)
{
    for (uint32_t i = 0; i < windows_size(); i++)
        if (windows()[i]->get_pid() == pid)
            return i;
    return -1;
}

void WindowManager::set_active_window(uint32_t index)
{
    int old_active_window = active_window;
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
        for (uint32_t i = 0; i < windows_size(); i++) {
            if (windows()[i]->is_point_in_window(event->x, event->y) && windows()[i]->get_controlled())
                set_active_window(i);
        }
    }

    if (active_window != -1)
        windows()[active_window]->mouse_event(event);
    if (!event->modifier)
        compositor->update_mouse_position(event->x, event->y);
}

void WindowManager::keyboard_event(keyboard_event_t* event)
{
    if (active_window != -1 && event->state != KEY_DOWN) {
        windows()[active_window]->keyboard_event(event);
        return;
    }

    if ((event->key >= 49 && event->key <= 49 + WORKSPACES - 1) && (event->modifier == 2) && (WORKSPACES > 1)) {
        int workspace = CLAMP(event->key - 49, 0, WORKSPACES - 1);
        set_workspace(workspace);
        return;
    }

    if ((event->key == ' ') && (event->modifier == 2)) {
        if (active_window < 0)
            return;
        set_window_master(active_window);
        return;
    }

    if ((event->key == 10) && (event->modifier == 2)) {
        if (active_window < 0)
            return;
        if (active_fullscreen_window < 0) {
            active_fullscreen_window = active_window;
            set_fullscreen_window(active_fullscreen_window);
        } else {
            active_fullscreen_window = -1;
            show_all_windows();
        }

        compositor->require_update();
        update_window_positions();
        return;
    }

    if ((event->key == 't') && (event->modifier == 2)) {
        spawn("bin/terminal", 0, 0);
        return;
    }

    if (active_window != -1)
        windows()[active_window]->keyboard_event(event);
}

void WindowManager::set_window_master(uint32_t index)
{
    if (windows_size() <= 0 || index > windows_size())
        return;

    Window* window_ptr_source = windows()[index];
    Window* window_ptr_destination = windows()[0];
    windows()[0] = window_ptr_source;
    windows()[index] = window_ptr_destination;

    active_window = 0;
    update_window_border(0);
    update_window_border(index);
    update_window_positions();
    compositor->require_update();
}

void WindowManager::show_all_windows()
{
    for (uint32_t i = 0; i < windows_size(); i++) {
        if (!windows()[i]->get_controlled())
            continue;
        windows()[i]->get_canvas()->hidden = 0;
    }
}

void WindowManager::set_fullscreen_window(uint32_t index)
{
    for (uint32_t i = 0; i < windows_size(); i++) {
        if (!windows()[i]->get_controlled())
            continue;
        if (i == index)
            continue;
        windows()[i]->get_canvas()->hidden = 1;
    }
}

void WindowManager::require_update(int pid)
{
    int index = find_window_with_pid(pid);
    if ((index == -1) || (windows()[index]->get_canvas()->hidden))
        return;
    update_window_border(index);
    compositor->require_update_canvas(windows()[index]->get_canvas());
}

void WindowManager::update_window_border(uint32_t index)
{
    windows()[index]->get_canvas()->border_decoration = (index == active_window) ? WINDOW_ACTIVE_BORDER_COLOR : WINDOW_BORDER_COLOR;
}

void WindowManager::sanitizer()
{
    uint32_t index = 0;
    while (index < windows_size()) {
        if ((windows()[index]->get_zombie()) || (!windows()[index]->get_controlled())) {
            index++;
            continue;
        }

        int pid = windows()[index]->get_pid();
        if (kill(pid, 0) < 0) {
            destroy_window(index);
            index++;
            continue;
        }

        index++;
    }
}

void WindowManager::update_window_positions()
{
    sanitizer();

    uint32_t return_tiled_windows = tiled_windows;
    if (active_fullscreen_window >= 0)
        tiled_windows = 1;
    if (tiled_windows >= MAX_VISIBLE_WINDOWS + 1)
        tiled_windows = MAX_VISIBLE_WINDOWS;

    uint32_t position_x = WINDOW_GAP;
    uint32_t position_y = WINDOW_GAP;
    uint32_t windows_tile_vertical = 2;
    uint32_t tile_vertical_max = CLAMP(tiled_windows, 1, windows_tile_vertical);
    uint32_t tile_horizontal_max = (tiled_windows > windows_tile_vertical) ? tiled_windows - 1 : 1;
    uint32_t vertical_section = compositor->screen_width() / tile_vertical_max;
    uint32_t horizontal_section = compositor->screen_height() / tile_horizontal_max;

    uint32_t tiled_index = 0;
    for (uint32_t index = 0; index < windows_size(); index++) {
        if (windows()[index]->get_zombie())
            continue;
        if (!windows()[index]->get_controlled())
            continue;
        if (active_fullscreen_window >= 0 && index != active_fullscreen_window)
            continue;
        if (index >= MAX_VISIBLE_WINDOWS + 1) {
            windows()[index]->resize(0, 0);
            continue;
        }

        uint32_t height = horizontal_section - WINDOW_GAP * 2;
        uint32_t width = vertical_section - WINDOW_GAP;

        if (tiled_index >= tile_vertical_max - 1)
            width -= WINDOW_GAP;
        if (tiled_index == 0)
            height = compositor->screen_height() - WINDOW_GAP * 2;
        if (tiled_index > windows_tile_vertical)
            height += WINDOW_GAP;

        if (tiled_index == 0) {
            height -= WINDOW_TOP_GAP;
            position_y += WINDOW_TOP_GAP;
        } else if (tiled_index == 1) {
            height -= WINDOW_TOP_GAP;
        }

        windows()[index]->resize(width, height);
        windows()[index]->set_position(position_x, position_y);

        if (windows_tile_vertical > 1) {
            position_x += width + WINDOW_GAP;
            windows_tile_vertical--;
        } else {
            position_y += height + WINDOW_GAP;
        }

        tiled_index++;
    }

    tiled_windows = return_tiled_windows;
    if (!tiled_windows)
        compositor->require_update();
    compositor->require_update_next();
    /* compositor->require_update(); */
}

Window* WindowManager::compose_window(int pid)
{
    Window* window = new Window(pid);
    return window;
}

void WindowManager::create_window(uint32_t width, uint32_t height, int pid, uint32_t bg, uint8_t flags)
{
    if (active_fullscreen_window >= 0) {
        active_fullscreen_window = -1;
        show_all_windows();
    }

    nice(2);
    Window* window = compose_window(pid);
    windows_append(window);

    if ((flags & DISPLAY_FLAG_DISOWNED) > 0) {
        window->resize(width, height);
        window->disown();
    } else {
        tiled_windows++;
    }

    update_window_positions();

    canvas_set(window->get_canvas()->framebuffer, bg, window->get_canvas()->size);
    window->get_canvas()->border_decoration = 0;
    if (((bg >> 24) & 0x000000FF) > 0)
        canvas_create_alpha(window->get_canvas(), bg);

    compositor->add_render_layer(window->get_canvas());
    window->create_process_connection();
    if (window->get_controlled())
        set_active_window(windows_size() - 1);
    nice(-2);
}

void WindowManager::destroy_window(uint32_t index)
{
    if (active_fullscreen_window >= 0) {
        active_fullscreen_window = -1;
        show_all_windows();
    }

    compositor->remove_render_layer(windows()[index]->get_canvas());
    if (windows()[index]->get_controlled())
        tiled_windows--;

    windows()[index]->die();
    if (tiled_windows) {
        /* Prepare other windows */
        update_window_positions();
        sys_yield();
    }

    delete windows()[index];
    windows_remove(index);

    if (!tiled_windows) {
        active_window = -1;
        return;
    }

    if (active_window > 0)
        active_window--;

    uint32_t next = CLAMP(active_window, 0, tiled_windows - 1);
    uint32_t current = 0;
    for (uint32_t i = 0; i < windows_size(); i++) {
        if (!windows()[i]->get_controlled())
            continue;
        if (current >= next) {
            active_window = i;
            break;
        }
        current++;
    }

    if (tiled_windows)
        update_window_border(active_window);

    update_window_positions();
    compositor->require_update();
}

void WindowManager::destroy_window_pid(int pid)
{
    int index = find_window_with_pid(pid);
    if (index == -1)
        return;

    destroy_window(index);
    if (active_window == -1)
        update_window_positions();
}

Window** WindowManager::windows()
{
    return workspaces[active_windows].windows;
}

uint32_t WindowManager::windows_size()
{
    return workspaces[active_windows].window_count;
}

void WindowManager::windows_append(Window* window)
{
    window_group_t* group = &workspaces[active_windows];

    if (group->window_count + 1 >= group->windows_ptr_size) {
        group->windows = (Window**)realloc(group->windows, (group->window_count + 1) * sizeof(Window*));
        group->windows_ptr_size = group->window_count + 1;
    }

    group->windows[group->window_count] = window;
    group->window_count++;
}

void WindowManager::windows_remove(uint32_t index)
{
    window_group_t* group = &workspaces[active_windows];

    if (group->window_count + 1 >= group->windows_ptr_size) {
        group->windows = (Window**)realloc(group->windows, (group->window_count - 1) * sizeof(Window*));
        group->windows_ptr_size = group->window_count + 1;
    }

    if (index >= group->window_count)
        return;
    for (int i = index; i < group->window_count - 1; i++)
        group->windows[i] = group->windows[i + 1];
    group->window_count--;
}

void WindowManager::set_workspace(uint16_t set_workspace)
{
    nice(2);
    workspaces[active_windows].stored_active_tiled = tiled_windows;
    workspaces[active_windows].stored_active_window = active_window;

    for (uint32_t i = 0; i < windows_size(); i++) {
        if (!windows()[i]->get_controlled())
            continue;
        windows()[i]->get_canvas()->hidden = 1;
    }

    active_windows = CLAMP(set_workspace, 0, WORKSPACES - 1);

    for (uint32_t i = 0; i < windows_size(); i++) {
        if (!windows()[i]->get_controlled())
            continue;
        windows()[i]->get_canvas()->hidden = 0;
    }

    tiled_windows = workspaces[active_windows].stored_active_tiled;
    active_window = workspaces[active_windows].stored_active_window;

    compositor->require_update();
    update_window_positions();
    nice(-2);
}
