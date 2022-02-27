#include "draw.hpp"
#include "font.hpp"

static const uint32_t text_mode_colors[] = {
    0x0, 0x800000, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,
    0x808080, 0xFF2F2F, 0x33BC33, 0xffff00, 0x8A8AFF, 0xff00ff, 0x00ffff, 0xffffff
};

static uint32_t pos_x = TEXT_GAP_X;
static uint32_t pos_y = TEXT_GAP_Y;
static uint32_t color = 0xd4d4d4;
static int escape_flag = 0;

void cursor_set(canvas_t* canvas, bool show)
{
    for (uint32_t y = 0; y < 12; y++) {
        if (show) {
            canvas->framebuffer[(pos_y + y - 2) * canvas->width + (pos_x + 1)] = 0xD8D8D8;
        } else {
            canvas->framebuffer[(pos_y + y - 2) * canvas->width + (pos_x + 1)] = BACKGROUND_COLOR;
        }
    }
}

void clear_text(canvas_t* canvas)
{
    pos_x = TEXT_GAP_X;
    pos_y = TEXT_GAP_Y;
    canvas_set(canvas->framebuffer, BACKGROUND_COLOR, canvas->size);
}

void scroll_text(canvas_t* canvas)
{
    pos_x = TEXT_GAP_X;
    for (uint32_t y = TEXT_GAP_Y; y < canvas->height - TEXT_GAP_Y; y++)
        for (uint32_t x = 0; x < canvas->width; x++)
            canvas->framebuffer[y * canvas->width + x] = canvas->framebuffer[(y + TEXT_GAP_Y) * canvas->width + x];
}

void next_line(canvas_t* canvas)
{
    if (pos_y + TEXT_GAP_Y >= canvas->height - TEXT_GAP_Y) {
        scroll_text(canvas);
        return;
    }
    pos_x = TEXT_GAP_X;
    pos_y += TEXT_GAP_Y;
}

void character_set(canvas_t* canvas, int index)
{
    if ((index == '\33') && !escape_flag) {
        escape_flag = 1;
        return;
    }

    if (escape_flag == 1) {
        escape_flag = 0;
        switch (index) {
        case 1:
            clear_text(canvas);
            return;
        case 2:
            escape_flag = 2;
            return;
        case 3:
            color = 0xd4d4d4;
            return;
        }
    }

    if (escape_flag == 2) {
        escape_flag = 0;
        color = text_mode_colors[index];
        return;
    }

    if (index == 8) {
        if (pos_x >= 18) {
            cursor_set(canvas, false);
            pos_x -= FONT_WIDTH;
            character_set(canvas, 0);
            pos_x -= FONT_WIDTH;
        }
        return;
    }

    if (index == 10) {
        cursor_set(canvas, false);
        next_line(canvas);
        return;
    }

    uint32_t rgb = color;

    cursor_set(canvas, false);
    for (uint32_t x = 0; x < FONT_WIDTH; x++) {
        for (uint32_t y = 0; y < FONT_HEIGHT; y++) {
            canvas->framebuffer[(pos_y + y) * canvas->width + (pos_x + x)] = BACKGROUND_COLOR;
            if (index == 0) {
                rgb = BACKGROUND_COLOR;
                continue;
            }
            if ((index - 33 >= 100) || (index - 33 < 0))
                continue;
            if (font8x8[index - 33][y] & 1 << x)
                canvas->framebuffer[(pos_y + y) * canvas->width + (pos_x + x)] = color;
        }
    }

    pos_x += FONT_WIDTH;
    if (pos_x >= canvas->width - TEXT_GAP_X)
        next_line(canvas);
}

void draw_text(canvas_t* canvas, char* str)
{
    for (uint32_t i = 0; i < strlen(str); i++)
        character_set(canvas, str[i]);
    cursor_set(canvas, true);
}
