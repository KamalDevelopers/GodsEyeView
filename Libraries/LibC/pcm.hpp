#ifndef PCM_HPP
#define PCM_HPP

#include "types.hpp"
#include "unistd.hpp"

typedef struct pcm_header {
    uint8_t* data = 0;
    uint32_t size = 0;
    uint16_t sample_rate = 44100;
    uint8_t channels = 2;
    uint8_t bits = 16;
} pcm_header_t;

inline void audio_device_write(pcm_header_t buffer)
{
    int fd = open((char*)"/dev/audio");
    write(fd, (void*)&buffer, sizeof(pcm_header_t));
    close(fd);
}

#endif
