#ifndef GUI_HPP
#define GUI_HPP
#include "../../kernel/Hardware/Drivers/keyboard.hpp"
#include "../../kernel/Hardware/Drivers/mouse.hpp"
#include "../../kernel/Hardware/Drivers/vga.hpp"
#include "../LibC/string.hpp"
#include <stdarg.h>

namespace GUI {
class Button {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    uint8_t widget_color;
    uint8_t box_color;
    char* widget_text;
    MouseDriver* mouse;
    void (*on_press)(void);

public:
    Button(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text, void (*op)(void));
    void Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
};

class Label {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    uint8_t widget_color;
    uint8_t box_color;
    char* widget_text;

public:
    Label(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text);
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
};

class Window {
private:
    uint8_t save_mouse_press = 0;
    int mouse_down = 0;
    int widget_indexL;
    int widget_indexB;
    int win_height;
    int win_width;
    int win_xpos;
    int win_ypos;
    uint8_t win_color;
    uint8_t win_bar;
    Label* childrenL[100];
    Button* childrenB[100];

public:
    Window(int xpos, int ypos, int w, int h, uint8_t color, uint8_t win_bar = 1);
    void Begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);
    int AddWidget(char* count, ...);
    void MousePress(uint32_t x, uint32_t y, int b, Graphics* vga);
    void MouseRelease(uint32_t x, uint32_t y, int b);

    int GetWidth() { return win_width; }
    int GetHeight() { return win_height; }
    int GetPosX() { return win_xpos; }
    int GetPosY() { return win_ypos; }
};

class Desktop {
private:
    int win_index;
    int desk_height;
    int desk_width;
    int old_mouse_x;
    int old_mouse_y;
    uint8_t old_mouse_color;

    Window* children[100];
    Graphics* vga;
    MouseDriver* mouse;
    KeyboardDriver* keyboard;

public:
    Desktop(int w, int h, Graphics* g, MouseDriver* m, KeyboardDriver* k);
    void Draw();
    int AddWin(int count, ...);
    void MouseRelease(uint32_t MouseX, uint32_t MouseY, int b);
    void MousePress(uint32_t MouseX, uint32_t MouseY, int b);
    void DrawMouse(int32_t x, int32_t y);
    Graphics* GetVGA() { return vga; }
    MouseDriver* GetMouse() { return mouse; };
    KeyboardDriver* GetKeyboard() { return keyboard; };
};
};
#endif