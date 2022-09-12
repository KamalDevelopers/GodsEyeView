#ifndef STREAM_HPP
#define STREAM_HPP

#include <LibC/types.h>

typedef struct pcm_stream {
    uint8_t* data = 0;
    uint32_t size = 0;
    uint16_t sample_rate = 44100;
    uint8_t channels = 2;
    uint8_t bits = 16;
} pcm_stream_t;

#endif
