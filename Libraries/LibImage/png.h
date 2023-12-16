#ifndef UPNG_H
#define UPNG_H

#include <LibC/types.h>

typedef struct png_image {
    int width;
    int height;
    void* internal_upng;
    uint8_t* buffer;
} png_image_t;

int free_png(png_image_t* png_image);
int decode_png(const char* file, png_image_t* png_image);

#endif
