#ifndef DRAW_HPP
#define DRAW_HPP

#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibDisplay/canvas.hpp>

#define BACKGROUND_COLOR 0x080808
#define TEXT_GAP_X 12
#define TEXT_GAP_Y 15
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

void clear_text(canvas_t* canvas);
void draw_text(canvas_t* canvas, char* str);
void character_set(canvas_t* canvas, int index);

#endif
