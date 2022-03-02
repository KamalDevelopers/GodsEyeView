#ifndef DRAW_HPP
#define DRAW_HPP

#include "font.hpp"
#include <LibC/stat.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibDisplay/canvas.hpp>

#define BACKGROUND_COLOR 0x080808
#define TEXT_GAP_X 12
#define TEXT_GAP_Y 15

void load_font(char* name);
void clear_text(canvas_t* canvas);
void draw_text(canvas_t* canvas, char* str);
void character_set(canvas_t* canvas, int index);

#endif
