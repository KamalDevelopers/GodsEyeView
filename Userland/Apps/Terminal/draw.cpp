#include "draw.hpp"
#include "font.hpp"

static const uint32_t text_mode_colors[] = {
    0x0, 0xAB3030, 0x008000, 0x808000, 0x000080, 0x800080, 0x008080, 0xc0c0c0,
    0x808080, 0xFF2F2F, 0x33BC33, 0xffff00, 0x8A8AFF, 0xff00ff, 0x00bbbb, 0xffffff
};

static uint32_t pos_x = TEXT_GAP_X;
static uint32_t pos_y = TEXT_GAP_Y;
static uint32_t color = 0xA8A7A7;
static int escape_flag = 0;
static uint8_t* font_buffer = 0;
static char terminal_text_buffer[2048];

void load_font(const char* name)
{
    struct stat statbuffer;
    int file_descriptor = open(name, O_RDONLY);
    fstat(file_descriptor, &statbuffer);
    font_buffer = (uint8_t*)malloc(sizeof(char) * statbuffer.st_size);

    read(file_descriptor, font_buffer, statbuffer.st_size);
    close(file_descriptor);

    memset(terminal_text_buffer, 0, 2048);
}

void unload_font()
{
    if (font_buffer)
        free(font_buffer);
}

void cursor_set(canvas_t* canvas, bool show)
{
    for (uint32_t y = 0; y < 15; y++) {
        if (show)
            canvas->framebuffer[(pos_y + y - 2) * canvas->width + (pos_x + 1)] = 0xD8D8D8;
        else
            canvas->framebuffer[(pos_y + y - 2) * canvas->width + (pos_x + 1)] = BACKGROUND_COLOR;
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
    for (uint32_t y = TEXT_GAP_Y; y < canvas->height - TEXT_GAP_Y - 1; y++)
        for (uint32_t x = 0; x < canvas->width; x++)
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
            color = 0xA8A7A7;
            return;
        case 5:
            color = 0x0;
            escape_flag = 10;
            return;
        }
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
            pos_x -= ((psf_font_t*)font_buffer)->width + 1;
            character_set(canvas, 32);
            pos_x -= ((psf_font_t*)font_buffer)->width + 1;
        }
        return;
    }

    if (index == 10) {
        cursor_set(canvas, false);
        next_line(canvas);
        return;
    }

    cursor_set(canvas, false);
    display_character(canvas, (psf_font_t*)font_buffer, index, pos_x, pos_y, color, BACKGROUND_COLOR);

    pos_x += ((psf_font_t*)font_buffer)->width + 1;
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
