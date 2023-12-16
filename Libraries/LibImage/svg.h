#ifndef SVG_H
#define SVG_H

#include <LibC/stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct svg_image {
    float original_width;
    float original_height;
    int width;
    int height;
    uint8_t* buffer;
} svg_image_t;

int decode_svg(const char* file, uint32_t session_size, svg_image_t* svg_image);
int free_svg(svg_image_t* svg_image);

#ifdef __cplusplus
}
#endif

#endif
