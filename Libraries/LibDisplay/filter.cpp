#include "filter.hpp"

static uint32_t box_blur_rcache = 0;
static uint32_t box_blur_b2cache = 0;
static uint32_t box_blur_bbcache = 0;
static uint32_t box_blur_wcache = 0;
static uint32_t box_blur_cache = 0;
static uint32_t box_blur_coffcache = 0;
static uint32_t box_blur_rstcache = 0;

void reset_box_blur_cache()
{
    box_blur_rcache = 0;
    box_blur_rstcache = 0;
}

uint32_t box_blur_pixel(uint32_t* destination, uint32_t i, uint32_t width, uint32_t coff, uint32_t blur_level)
{
    if (box_blur_cache != blur_level || box_blur_coffcache != coff) {
        reset_box_blur_cache();
        box_blur_cache = blur_level;
        box_blur_b2cache = blur_level * 2;
        box_blur_bbcache = box_blur_b2cache * box_blur_b2cache;
        box_blur_wcache = blur_level * width;
        box_blur_coffcache = coff;
    }

    uint32_t sum = 0;
    uint32_t b2 = box_blur_b2cache;
    int width_offset = box_blur_wcache;

    if (!box_blur_rstcache) {
        box_blur_rstcache = blur_level + 4;
        for (uint32_t x_offset = 0; x_offset < b2; x_offset++) {
            int offset = blur_level;
            for (uint32_t y_offset = 0; y_offset < b2; y_offset++) {
                sum += ((destination[width_offset + offset + i] >> coff) & 0x000000FF);
                offset--;
            }
            width_offset -= width;
        }

        box_blur_rcache = sum;
        return sum / box_blur_bbcache;
    }

    sum = box_blur_rcache;

    int loffset = box_blur_wcache;
    for (uint32_t y_offset = 0; y_offset < b2; y_offset++) {
        sum -= ((destination[loffset + i - blur_level + 1] >> coff) & 0x000000FF);
        sum += ((destination[loffset + i + blur_level] >> coff) & 0x000000FF);
        loffset -= width;
    }

    box_blur_rcache = sum;
    box_blur_rstcache--;
    return sum / box_blur_bbcache;
}

uint32_t gaussian_blur_pixel(uint32_t* destination, uint32_t i, uint32_t width, uint32_t coff, uint32_t blur_level)
{
    /* clang-format off */

    /* 3x3 kernel */
    if (blur_level <= 16) {
        return (((destination[i] >> coff) & 0x000000FF) * 4 +
            ((destination[i - 1] >> coff) & 0x000000FF) * 2 +
            ((destination[i + 1] >> coff) & 0x000000FF) * 2 +
            ((destination[i - width] >> coff) & 0x000000FF) * 2 +
            ((destination[i - width - 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width + 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width] >> coff) & 0x000000FF) * 2 +
            ((destination[i + width - 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + 1] >> coff) & 0x000000FF) * 1) / 16;
    }

    /* 5x5 kernel */
    if (blur_level <= 273) {
        return (((destination[i] >> coff) & 0x000000FF) * 41 +
            ((destination[i - 1] >> coff) & 0x000000FF) * 26 +
            ((destination[i + 1] >> coff) & 0x000000FF) * 26 +
            ((destination[i - width] >> coff) & 0x000000FF) * 26 +
            ((destination[i - width - 1] >> coff) & 0x000000FF) * 16 +
            ((destination[i - width + 1] >> coff) & 0x000000FF) * 16 +
            ((destination[i + width] >> coff) & 0x000000FF) * 26 +
            ((destination[i + width - 1] >> coff) & 0x000000FF) * 16 +
            ((destination[i + width + 1] >> coff) & 0x000000FF) * 16 +
            ((destination[i - 2] >> coff) & 0x000000FF) * 7 +
            ((destination[i + 2] >> coff) & 0x000000FF) * 7 +
            ((destination[i + width - 2] >> coff) & 0x000000FF) * 4 +
            ((destination[i + width + 2] >> coff) & 0x000000FF) * 4 +
            ((destination[i - width - 2] >> coff) & 0x000000FF) * 4 +
            ((destination[i - width + 2] >> coff) & 0x000000FF) * 4 +
            ((destination[i - width - width] >> coff) & 0x000000FF) * 7 +
            ((destination[i - width - width - 1] >> coff) & 0x000000FF) * 4 +
            ((destination[i - width - width + 1] >> coff) & 0x000000FF) * 4 +
            ((destination[i - width - width - 2] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - width + 2] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + width] >> coff) & 0x000000FF) * 7 +
            ((destination[i + width + width - 1] >> coff) & 0x000000FF) * 4 +
            ((destination[i + width + width + 1] >> coff) & 0x000000FF) * 4 +
            ((destination[i + width + width - 2] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + width + 2] >> coff) & 0x000000FF) * 1)
            / 273;
    }

    /* 7x7 kernel */
    if (blur_level <= 1003) {
        return (((destination[i] >> coff) & 0x000000FF) * 159 +
            ((destination[i - 1] >> coff) & 0x000000FF) * 97 +
            ((destination[i + 1] >> coff) & 0x000000FF) * 97 +
            ((destination[i - width] >> coff) & 0x000000FF) * 97 +
            ((destination[i - width - 1] >> coff) & 0x000000FF) * 59 +
            ((destination[i - width + 1] >> coff) & 0x000000FF) * 59 +
            ((destination[i + width] >> coff) & 0x000000FF) * 97 +
            ((destination[i + width - 1] >> coff) & 0x000000FF) * 59 +
            ((destination[i + width + 1] >> coff) & 0x000000FF) * 59 +
            ((destination[i - 2] >> coff) & 0x000000FF) * 22 +
            ((destination[i + 2] >> coff) & 0x000000FF) * 22 +
            ((destination[i + width - 2] >> coff) & 0x000000FF) * 13 +
            ((destination[i + width + 2] >> coff) & 0x000000FF) * 13 +
            ((destination[i - width - 2] >> coff) & 0x000000FF) * 13 +
            ((destination[i - width + 2] >> coff) & 0x000000FF) * 13 +
            ((destination[i - width - width] >> coff) & 0x000000FF) * 22 +
            ((destination[i - width - width - 1] >> coff) & 0x000000FF) * 13 +
            ((destination[i - width - width + 1] >> coff) & 0x000000FF) * 13 +
            ((destination[i - width - width - 2] >> coff) & 0x000000FF) * 3 +
            ((destination[i - width - width + 2] >> coff) & 0x000000FF) * 3 +
            ((destination[i + width + width] >> coff) & 0x000000FF) * 22 +
            ((destination[i + width + width - 1] >> coff) & 0x000000FF) * 13 +
            ((destination[i + width + width + 1] >> coff) & 0x000000FF) * 13 +
            ((destination[i + width + width - 2] >> coff) & 0x000000FF) * 3 +
            ((destination[i + width + width + 2] >> coff) & 0x000000FF) * 3 +
            ((destination[i + 3] >> coff) & 0x000000FF) * 2 +
            ((destination[i - 3] >> coff) & 0x000000FF) * 2 +
            ((destination[i + width + 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width - 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width + 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + width + width] >> coff) & 0x000000FF) * 2 +
            ((destination[i + width + width + width + 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + width + width - 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - width - width] >> coff) & 0x000000FF) * 2 +
            ((destination[i - width - width - width + 1] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - width - width - 1] >> coff) & 0x000000FF) * 1) / 1003;
    }

    /* 7x7 kernel */
    if (blur_level <= 4096) {
        return (((destination[i] >> coff) & 0x000000FF) * 400 +
            ((destination[i - 1] >> coff) & 0x000000FF) * 300 +
            ((destination[i + 1] >> coff) & 0x000000FF) * 300 +
            ((destination[i - width] >> coff) & 0x000000FF) * 300 +
            ((destination[i - width - 1] >> coff) & 0x000000FF) * 225 +
            ((destination[i - width + 1] >> coff) & 0x000000FF) * 225 +
            ((destination[i + width] >> coff) & 0x000000FF) * 300 +
            ((destination[i + width - 1] >> coff) & 0x000000FF) * 225 +
            ((destination[i + width + 1] >> coff) & 0x000000FF) * 225 +
            ((destination[i - 2] >> coff) & 0x000000FF) * 120 +
            ((destination[i + 2] >> coff) & 0x000000FF) * 120 +
            ((destination[i + width - 2] >> coff) & 0x000000FF) * 90 +
            ((destination[i + width + 2] >> coff) & 0x000000FF) * 90 +
            ((destination[i - width - 2] >> coff) & 0x000000FF) * 90 +
            ((destination[i - width + 2] >> coff) & 0x000000FF) * 90 +
            ((destination[i - width - width] >> coff) & 0x000000FF) * 120 +
            ((destination[i - width - width - 1] >> coff) & 0x000000FF) * 90 +
            ((destination[i - width - width + 1] >> coff) & 0x000000FF) * 90 +
            ((destination[i - width - width - 3] >> coff) & 0x000000FF) * 6 +
            ((destination[i - width - width + 3] >> coff) & 0x000000FF) * 6 +
            ((destination[i - width - width - 2] >> coff) & 0x000000FF) * 36 +
            ((destination[i - width - width + 2] >> coff) & 0x000000FF) * 36 +
            ((destination[i + width + width] >> coff) & 0x000000FF) * 120 +
            ((destination[i + width + width - 1] >> coff) & 0x000000FF) * 90 +
            ((destination[i + width + width + 1] >> coff) & 0x000000FF) * 90 +
            ((destination[i + width + width - 3] >> coff) & 0x000000FF) * 6 +
            ((destination[i + width + width + 3] >> coff) & 0x000000FF) * 6 +
            ((destination[i + width + width - 2] >> coff) & 0x000000FF) * 36 +
            ((destination[i + width + width + 2] >> coff) & 0x000000FF) * 36 +
            ((destination[i + 3] >> coff) & 0x000000FF) * 20 +
            ((destination[i - 3] >> coff) & 0x000000FF) * 20 +
            ((destination[i + width + 3] >> coff) & 0x000000FF) * 15 +
            ((destination[i + width - 3] >> coff) & 0x000000FF) * 15 +
            ((destination[i - width + 3] >> coff) & 0x000000FF) * 15 +
            ((destination[i - width - 3] >> coff) & 0x000000FF) * 15 +
            ((destination[i + width + width + width] >> coff) & 0x000000FF) * 20 +
            ((destination[i + width + width + width + 1] >> coff) & 0x000000FF) * 15 +
            ((destination[i + width + width + width - 1] >> coff) & 0x000000FF) * 15 +
            ((destination[i + width + width + width - 2] >> coff) & 0x000000FF) * 6 +
            ((destination[i + width + width + width + 2] >> coff) & 0x000000FF) * 6 +
            ((destination[i + width + width + width - 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i + width + width + width + 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - width - width] >> coff) & 0x000000FF) * 20 +
            ((destination[i - width - width - width + 1] >> coff) & 0x000000FF) * 15 +
            ((destination[i - width - width - width - 1] >> coff) & 0x000000FF) * 15 +
            ((destination[i - width - width - width - 2] >> coff) & 0x000000FF) * 6 +
            ((destination[i - width - width - width + 2] >> coff) & 0x000000FF) * 6 +
            ((destination[i - width - width - width - 3] >> coff) & 0x000000FF) * 1 +
            ((destination[i - width - width - width + 3] >> coff) & 0x000000FF) * 1) / 4096;
    }

    /* 9x9 kernel */
    return (((destination[i] >> coff) & 0x000000FF) * 125 +
        ((destination[i - 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i + 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i - width] >> coff) & 0x000000FF) * 125 +
        ((destination[i - width - 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i - width + 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i + width] >> coff) & 0x000000FF) * 125 +
        ((destination[i + width - 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i + width + 1] >> coff) & 0x000000FF) * 125 +
        ((destination[i - 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i + 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width - 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width + 2] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width - 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width + 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width - 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width + 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width - 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width + 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width - 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i - 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width - 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width + 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - 3] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width + width] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width + width + 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width + width - 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i + width + width + width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width - 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width - width + 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width - width - 1] >> coff) & 0x000000FF) * 124 +
        ((destination[i - width - width - width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width + 3] >> coff) & 0x000000FF) * 123 +
        ((destination[i - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width - 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + 4] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width + 1] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width - 1] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i - width - width - width - width - 3] >> coff) & 0x000000FF) * 122 +
        ((destination[i - width - width - width - width + 3] >> coff) & 0x000000FF) * 122 +
        ((destination[i - width - width - width - width - 4] >> coff) & 0x000000FF) * 121 +
        ((destination[i - width - width - width - width + 4] >> coff) & 0x000000FF) * 121 +
        ((destination[i + width + width + width + width] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + width + 1] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + width - 1] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + width - 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + width + 2] >> coff) & 0x000000FF) * 123 +
        ((destination[i + width + width + width + width - 3] >> coff) & 0x000000FF) * 122 +
        ((destination[i + width + width + width + width + 3] >> coff) & 0x000000FF) * 122 +
        ((destination[i + width + width + width + width - 4] >> coff) & 0x000000FF) * 121 +
        ((destination[i + width + width + width + width + 4] >> coff) & 0x000000FF) * 121) / 9509;

    /* clang-format on */
}
