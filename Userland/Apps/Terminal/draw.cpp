#include "draw.hpp"
#include <LibFont/font.hpp>

static const uint32_t text_mode_colors[] = {
    0x0, 0xAB3030, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,
    0x808080, 0xFF2F2F, 0x33BC33, 0xffff00, 0xa5d1f2, 0xff00ff, 0x79f7f7, 0xffffff
};

static uint32_t pos_x = TEXT_GAP_X;
static uint32_t pos_y = TEXT_GAP_Y;
static uint32_t color = 0xA8A7A7;
static int escape_flag = 0;
static char terminal_text_buffer[2048];
static uint8_t cursor_show = 0;
static uint32_t char_need_clear = 0;
static font_t* default_font;

void init(const char* font)
{
    default_font = font_load(font);
    memset(terminal_text_buffer, 0, 2048);
}

void uninit()
{
    font_unload(default_font);
}

void cursor_set(canvas_t* canvas, bool show)
{
    cursor_show = show;
    int py = pos_y;
    for (uint32_t y = 0; y < 13; y++) {
        if (show)
            canvas->framebuffer[(py + y - 5) * canvas->width + (pos_x + 1)] = 0xD8D8D8;
        else
            canvas->framebuffer[(py + y - 5) * canvas->width + (pos_x + 1)] = BACKGROUND_COLOR;
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
    /* Clear first line */
    canvas_set((uint32_t*)canvas->framebuffer, BACKGROUND_COLOR, TEXT_GAP_Y * canvas->width);

    for (uint32_t y = TEXT_GAP_Y - 1; y < canvas->height - TEXT_GAP_Y - 1; y++)
        for (uint32_t x = TEXT_GAP_X; x < canvas->width - TEXT_GAP_X; x++)
            canvas->framebuffer[y * canvas->width + x] = canvas->framebuffer[(y + TEXT_GAP_Y) * canvas->width + x];
    /* Clear last line */
    canvas_set((uint32_t*)canvas->framebuffer + canvas->size - (TEXT_GAP_Y + 1) * canvas->width, BACKGROUND_COLOR, TEXT_GAP_Y * canvas->width);
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

void character_set(canvas_t* canvas, int index, bool bg_blend)
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
            color = 0xA8A7A7;
            return;
        case 5:
            color = 0x0;
            escape_flag = 10;
            return;
        case 6:
            escape_flag = 20;
            return;
        }
    }

    if (escape_flag == 20) {
        escape_flag = 0;
        if (index == 1) {
            if (pos_x >= 18) {
                if (cursor_show)
                    cursor_set(canvas, false);
                pos_x -= default_font->font_header->width + 1;
                char_need_clear++;
            }
            return;
        }
        return;
    }

    if (escape_flag == 2) {
        escape_flag = 0;
        color = text_mode_colors[index];
        return;
    }

    if (escape_flag >= 10 && escape_flag <= 12) {
        if (escape_flag == 10)
            color = (color & 0xFFFFFF00) | index;
        if (escape_flag == 11)
            color = (color & 0xFFFF00FF) | (uint32_t)index << 8;
        if (escape_flag == 12)
            color = (color & 0xFF00FFFF) | (uint32_t)index << 16;
        color = (color & 0x00FFFFFF) | 0x0 << 24;

        escape_flag++;
        if (escape_flag == 13)
            escape_flag = 0;
        return;
    }

    if (index == 8) {
        if (pos_x >= 18) {
            cursor_set(canvas, false);
            pos_x -= default_font->font_header->width + 1;
            pos_y -= 2;
            character_set(canvas, 32, true);
            pos_y += 2;
            pos_x -= default_font->font_header->width + 1;
            character_set(canvas, 32, true);
            pos_x -= default_font->font_header->width + 1;
        }
        return;
    }

    if (index == 10) {
        cursor_set(canvas, false);
        next_line(canvas);
        return;
    }

    if (index <= 31)
        return;

    if (char_need_clear && index != 32 && index != 8) {
        pos_x += default_font->font_header->width + 1;
        character_set(canvas, 8, true);
        char_need_clear--;
    }

    cursor_set(canvas, false);
    if (!bg_blend && index != ' ')
        font_display_character(default_font, canvas, index, pos_x, pos_y, color);
    else
        font_display_character_with_bg(default_font, canvas, index, pos_x, pos_y, color, BACKGROUND_COLOR, true);

    pos_x += default_font->font_header->width + 1;
    if (pos_x >= canvas->width - TEXT_GAP_X)
        next_line(canvas);
}

void draw_text(canvas_t* canvas, const char* str)
{
    if (strlen(terminal_text_buffer) + strlen(str) >= 2048)
        memset(terminal_text_buffer, 0, 2048);
    strcat(terminal_text_buffer, str);

    for (uint32_t i = 0; i < strlen(str); i++)
        character_set(canvas, str[i]);
    cursor_set(canvas, true);
}

void resize_text(canvas_t* canvas)
{
    clear_text(canvas);
    for (uint32_t i = 0; i < strlen(terminal_text_buffer); i++)
        character_set(canvas, terminal_text_buffer[i]);
    cursor_set(canvas, true);
}
