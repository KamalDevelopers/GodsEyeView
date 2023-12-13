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

void move_cursor_y(canvas_t* canvas, int start, int lines)
{
    start += TEXT_GAP_Y * lines;
    if (start >= canvas->height - TEXT_GAP_Y)
        return;
    if (start < TEXT_GAP_Y)
        return;
    cursor_set(canvas, false);
    pos_y = start;
}

void move_cursor_x(canvas_t* canvas, int start, int columns)
{
    start += (default_font->font_header->width + 1) * columns;
    if (start >= canvas->width - TEXT_GAP_X)
        return;
    if (start < TEXT_GAP_X)
        return;
    cursor_set(canvas, false);
    pos_x = start;
}

bool handle_escape_flag_legacy(canvas_t* canvas, int index)
{
    static char legacy_escape_sequence[25];
    static int legacy_escape_sequence_index;
    bool do_return = 0;

    if (index == '[') {
        escape_flag = 200;
        return 1;
    }

    if (escape_flag == 200 && index == 'H') {
        pos_x = TEXT_GAP_X;
        pos_y = TEXT_GAP_Y;
        escape_flag = 0;
        return 1;
    }

    if ((index == 'A' || index == 'B') && escape_flag >= 200 && legacy_escape_sequence_index) {
        for (int i = 0; i < legacy_escape_sequence_index; ++i)
            if (!isdigit(legacy_escape_sequence[i]))
                return 1;
        int mul = (index == 'B') ? 1 : -1;
        int lines = atoi(legacy_escape_sequence);
        move_cursor_y(canvas, pos_y, lines * mul);
        escape_flag = 0;
        legacy_escape_sequence_index = 0;
        return 1;
    }

    if ((index == 'C' || index == 'D' || index == 'G') && escape_flag >= 200 && legacy_escape_sequence_index) {
        for (int i = 0; i < legacy_escape_sequence_index; ++i)
            if (!isdigit(legacy_escape_sequence[i]))
                return 1;
        int mul = (index == 'D') ? -1 : 1;
        int start_col = (index == 'G') ? TEXT_GAP_X : pos_x;
        int cols = atoi(legacy_escape_sequence);
        move_cursor_x(canvas, start_col, cols * mul);
        escape_flag = 0;
        legacy_escape_sequence_index = 0;
        return 1;
    }

    if (index >= 48 && index <= 57 && escape_flag == 200) {
        if (legacy_escape_sequence_index < sizeof(legacy_escape_sequence) - 1) {
            legacy_escape_sequence[legacy_escape_sequence_index] = index;
            legacy_escape_sequence_index++;
            legacy_escape_sequence[legacy_escape_sequence_index] = 0;
        }
        escape_flag = 200;
        do_return = 1;
    }

    if (index == '2' && escape_flag == 200) {
        escape_flag = 202;
        return 1;
    }

    if (index == '3' && escape_flag == 200) {
        escape_flag = 203;
        return 1;
    }

    if (escape_flag == 202) {
        escape_flag = 200;
        if (index == 'J')
            clear_text(canvas);
        return 1;
    }

    if (escape_flag == 203) {
        escape_flag = 200;
        int term_color_index = index - 48;
        if (term_color_index < 0 || term_color_index > 16)
            return 1;
        color = text_mode_colors[term_color_index];
        return 1;
    }

    if (index == '0' && escape_flag == 200) {
        color = 0xA8A7A7;
        return 1;
    }

    if (index == ';') {
        escape_flag = 200;
        legacy_escape_sequence_index = 0;
        return 1;
    }

    if (index == 'm' && escape_flag == 200) {
        escape_flag = 0;
        legacy_escape_sequence_index = 0;
        return 1;
    }

    return do_return;
}

bool handle_escape_flag(canvas_t* canvas, int index)
{
    if (escape_flag == 1) {
        escape_flag = 0;
        switch (index) {
        case 1:
            clear_text(canvas);
            return 1;
        case 2:
            escape_flag = 2;
            return 1;
        case 3:
            color = 0xA8A7A7;
            return 1;
        case 5:
            color = 0x0;
            escape_flag = 10;
            return 1;
        case 6:
            escape_flag = 20;
            return 1;
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
            return 1;
        }
        return 1;
    }

    if (escape_flag == 2) {
        escape_flag = 0;
        color = text_mode_colors[index];
        return 1;
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
        return 1;
    }

    return 0;
}

void character_set(canvas_t* canvas, int index, bool bg_blend)
{
    if ((index == '\x1B') && (!escape_flag || escape_flag >= 200)) {
        escape_flag = 100;
        return;
    }

    if ((index == '\34') && !escape_flag) {
        escape_flag = 1;
        return;
    }

    if (escape_flag >= 100) {
        if (handle_escape_flag_legacy(canvas, index))
            return;
    } else if (escape_flag) {
        if (handle_escape_flag(canvas, index))
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
