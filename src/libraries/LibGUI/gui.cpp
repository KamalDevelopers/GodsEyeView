#include "gui.hpp"

using namespace GUI;

Label::Label(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text)
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;

    widget_color = fcolor;
    box_color = bcolor;
    widget_text = text;
}

void Label::Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight)
{
    int Xsize = str_len(widget_text) * 8; //8 = Bitmap size
    int twidget_xpos = widget_xpos + parentPosX;
    int twidget_ypos = widget_ypos + parentPosY;

    if (Xsize > parentWidth)
        return;

    if (twidget_xpos + Xsize > parentWidth + parentPosX)
        return;

    if (twidget_ypos + 8 > parentHeight + parentPosY)
        return;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width + Xsize; x++)
            vga->PutPixel(twidget_xpos + x, twidget_ypos + y, box_color);
    }

    vga->ResetOffset();
    vga->Print(widget_text, widget_color, twidget_xpos, twidget_ypos);
}

Button::Button(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text, void (*op)(void))
{
    widget_xpos = xpos;
    widget_ypos = ypos;
    widget_width = width;
    widget_height = height;

    widget_color = fcolor;
    box_color = bcolor;
    widget_text = text;
    on_press = op;
}

void Button::Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight)
{
    int Xsize = str_len(widget_text) * 8; //8 = Bitmap size
    int twidget_xpos = widget_xpos + parentPosX;
    int twidget_ypos = widget_ypos + parentPosY;

    if (Xsize > parentWidth)
        return;

    if (twidget_xpos + Xsize > parentWidth + parentPosX)
        return;

    if (twidget_ypos + 8 > parentHeight + parentPosY)
        return;

    for (int y = 0; y < widget_height; y++) {
        for (int x = 0; x < widget_width + Xsize; x++) {
            int x_t = twidget_xpos + x;
            int y_t = twidget_ypos + y;

            if ((mouse->GetMouseX() == x_t) && (mouse->GetMouseY() == y_t) && (mouse->GetMousePress() == 1))
                on_press();

            vga->PutPixel(x_t, y_t, box_color);
        }
    }

    vga->ResetOffset();
    vga->Print(widget_text, widget_color, twidget_xpos, twidget_ypos);
}

Window::Window(int xpos, int ypos, int w, int h, uint8_t color, uint8_t wb)
{
    win_bar = wb;
    win_xpos = xpos;
    win_ypos = ypos;
    win_height = h;
    win_width = w;
    win_color = color;
}

void Window::Begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard)
{
    for (int y = 0 + win_ypos; y < win_height + win_ypos; y++)
        for (int x = 0 + win_xpos; x < win_width + win_xpos; x++)
            vga->PutPixel(x, y, win_color);

    if (win_bar == 1) {
        for (int y = 0 + win_ypos; y < win_ypos + 4; y++) {
            for (int x = 0 + win_xpos; x < win_width + win_xpos; x++) {
                if ((mouse->GetMouseX() == x) && (mouse->GetMouseY() == y - 4) && (mouse->GetMousePress() == 1))
                    save_mouse_press = 1;

                if ((mouse->GetMousePress() == 0) && (save_mouse_press == 1))
                    save_mouse_press = 0;

                if (save_mouse_press == 1) {
                    win_xpos = mouse->GetMouseX();
                    win_ypos = mouse->GetMouseY();
                }
                vga->PutPixel(x, y - 4, 0x8);
            }
        }
    }

    for (int i = 0; i < widget_indexL; i++)
        childrenL[i]->Add(vga, win_xpos, win_ypos, win_width, win_height);

    for (int i = 0; i < widget_indexB; i++)
        childrenB[i]->Add(vga, mouse, win_xpos, win_ypos, win_width, win_height);
}

int Window::AddWidget(char* count, ...)
{
    va_list list;
    int j = 0;

    va_start(list, count);
    for (j = 0; j < str_len(count); j++) {
        switch (count[j]) {
        case 'l':
            childrenL[widget_indexL] = va_arg(list, Label*);
            widget_indexL++;
            break;
        case 'b':
            childrenB[widget_indexB] = va_arg(list, Button*);
            widget_indexB++;
            break;
        }
    }

    va_end(list);
    return 0;
}

void Window::MouseRelease(uint32_t x, uint32_t y, int b)
{
    mouse_down = 0;
}

void Window::MousePress(uint32_t x, uint32_t y, int b, Graphics* vga)
{
    mouse_down = 1;
    if (win_bar == 1) {
        for (int bar_y = 0 + win_ypos; bar_y < 4 + win_ypos; bar_y++) {
            for (int bar_x = 0 + win_xpos; bar_x < win_width + win_xpos; bar_x++) {
                if ((bar_x == x) and (bar_y == y)) {
                    win_xpos = x;
                    win_ypos = y;
                }
            }
        }
    }
}

Desktop::Desktop(int w, int h, Graphics* g, MouseDriver* m, KeyboardDriver* k)
{
    desk_height = h;
    desk_width = w;
    vga = g;
    mouse = m;
    keyboard = k;
}

void Desktop::MouseRelease(uint32_t x, uint32_t y, int b)
{
    for (int i = 0; i < win_index; i++)
        children[i]->MouseRelease(x, y, b);
}

void Desktop::MousePress(uint32_t x, uint32_t y, int b)
{
    for (int i = 0; i < win_index; i++)
        children[i]->MousePress(x, y, b, vga);
}

void Desktop::DrawMouse(int32_t x, int32_t y)
{
    uint8_t tempc;

    if ((old_mouse_x != x) or (old_mouse_y != y)) {
        tempc = *vga->GetPixelColor(x, y);
        vga->PutPixel(x, y, 0x6);
        vga->PutPixel(old_mouse_x, old_mouse_y, old_mouse_color);
        old_mouse_color = tempc;
    }
    old_mouse_x = x;
    old_mouse_y = y;
}

int Desktop::AddWin(int count, ...)
{
    if (count > 100)
        return 1;

    va_list list;
    int j = 0;

    va_start(list, count);
    for (j = 0; j < count; j++) {
        children[win_index] = va_arg(list, Window*);
        win_index++;
    }

    va_end(list);
    return 0;
}

void Desktop::Draw()
{
    for (int y = 0; y < desk_height; y++)
        for (int x = 0; x < desk_width; x++)
            vga->PutPixel(x, y, 0x0);

    for (int i = 0; i < win_index; i++)
        children[i]->Begin(vga, mouse, keyboard);

    int index = 0;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            if (mouse_bitmap[index] != -1)
                vga->PutPixel(x + mouse->GetMouseX(), y + mouse->GetMouseY(), mouse_bitmap[index]);
            index++;
        }

    vga->RenderScreen();
}