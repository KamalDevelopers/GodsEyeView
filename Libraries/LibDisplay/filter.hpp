#ifndef FILTER_HPP
#define FILTER_HPP

#include <LibC/types.h>

uint32_t gaussian_blur_pixel(uint32_t* destination, uint32_t i, uint32_t width, uint32_t coff, uint32_t blur_level);
uint32_t box_blur_pixel(uint32_t* destination, uint32_t i, uint32_t width, uint32_t coff, uint32_t blur_level);
void reset_box_blur_cache();

#endif
