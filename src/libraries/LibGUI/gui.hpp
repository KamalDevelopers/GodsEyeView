#ifndef GUI_HPP
#define GUI_HPP
#include "../../kernel/Hardware/Drivers/keyboard.hpp"
#include "../../kernel/Hardware/Drivers/mouse.hpp"
#include "../../kernel/Hardware/Drivers/vga.hpp"
#include "../LibC/string.hpp"
#include "../LibC/stdlib.hpp"
#include <stdarg.h>

namespace GUI {
class Image {
private:
    int widget_width;
    int widget_height;
    short int* bitmap;

public:
    Image(int width, int height, short int* bmp);
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    uint16_t GetColor(int index) { return bitmap[index]; }
};

class Input
{
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    int input_text_index = 0;
    int backslashoffset = 0;

    uint8_t active_input = 0;
    uint8_t widget_color;
    uint8_t box_color;
    
    char* input_text = " ";
    char* widget_text;

public:
    Input(int xpos, int ypos, int width, int height, uint8_t fcolor, uint8_t bcolor, char* text);
    void Add(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    char* GetInput() { return input_text; }
};

class Button {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    int shadow_offset = 2;
    uint8_t on_hover_state = 0;
    uint8_t widget_color = 0x0;
    uint8_t box_color = 0x7;
    uint8_t shadow_color = 0x0;
    uint8_t render_image = 0;
    char* widget_text;
    MouseDriver* mouse;
    Image* image;
    void (*on_press)(void);

public:
    Button(int xpos, int ypos, int width, int height, char* text, void (*op)(void));
    void Add(Graphics* vga, MouseDriver* mouse, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
    void AddImage(Image* img);
    void Color(uint8_t color) {box_color = color;}
    void ShadowColor(uint8_t scolor) {shadow_color = scolor;}
    void ShadowOffset(int soffset) {shadow_offset = soffset;}
};

class Panel {
private:
    int widget_xpos;
    int widget_ypos;
    int widget_width;
    int widget_height;
    uint8_t panel_color;

public:
    Panel(int xpos, int ypos, int width, int height, uint8_t color);
    void Add(Graphics* vga, int parentPosX, int parentPosY, int parentWidth, int parentHeight);
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
    void SetText(char* new_text) { widget_text = new_text; }
};

class Window {
private:
    uint8_t destroy_win = 0;
    uint8_t win_hidden = 0;
    uint8_t save_mouse_press = 0;

    int mouse_down = 0;
    int widget_indexL;
    int widget_indexB;
    int widget_indexI;
    int widget_indexP;
    int win_height;
    int win_width;
    int win_xpos;
    int win_ypos;

    uint8_t win_color;
    uint8_t win_bar;
    uint8_t border_thickness = 1;
    uint8_t border_color = 0x8;
    char* win_title = " ";
    
    Label* childrenL[100];
    Button* childrenB[100];
    Input* childrenI[100];
    Panel* childrenP[100];

public:
    Window(int xpos, int ypos, int w, int h, uint8_t color, uint8_t win_bar = 1);
    void Begin(Graphics* vga, MouseDriver* mouse, KeyboardDriver* keyboard);
    int AddWidget(char* count, ...);
    void Border(uint8_t thickness, uint8_t color);
    void SetTitle(char* wt) { win_title = wt; }

    void MousePress(uint32_t x, uint32_t y, int b, Graphics* vga);
    void MouseRelease(uint32_t x, uint32_t y, int b);

    void SetHidden(uint8_t hide) { win_hidden = hide; }
    
    void Revive() { destroy_win = 0; }
    void Destroy() { destroy_win = 1; }

    int GetDestroy() { return destroy_win; }
    int GetHidden() { return win_hidden; }
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
    uint8_t render_wallpapaper = 0;

    Image* desk_wallpaper;
    Window* children[100];
    Graphics* vga;
    MouseDriver* mouse;
    KeyboardDriver* keyboard;

public:
    Desktop(int w, int h, Graphics* g, MouseDriver* m, KeyboardDriver* k);
    void Draw();
    int AddWin(int count, ...);
    int AppendWin(Window* win);
    void MouseRelease(uint32_t MouseX, uint32_t MouseY, int b);
    void MousePress(uint32_t MouseX, uint32_t MouseY, int b);
    void DrawMouse(int32_t x, int32_t y);
    void SetWallpaper(Image* img);
    void WinDestroy(int index) { children[index] = 0; }

    Graphics* GetVGA() { return vga; }
    MouseDriver* GetMouse() { return mouse; };
    KeyboardDriver* GetKeyboard() { return keyboard; };
};
};
#endif