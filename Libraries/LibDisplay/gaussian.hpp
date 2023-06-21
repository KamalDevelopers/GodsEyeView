#ifndef GAUSSIAN_HPP
#define GAUSSIAN_HPP

#include <LibC/types.h>

inline uint32_t gaussian_blur_pixel(uint32_t* destination, uint32_t i, uint32_t width, uint32_t coff, uint32_t blur_level)
{
    /* clang-format off */
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
    /* clang-format on */
}

#endif
