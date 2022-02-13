#include "wave.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav/dr_wav.h"

Wave::Wave(char* file)
{
    if (file == 0)
        return;

    int file_descriptor;
    struct stat statbuffer;

    file_descriptor = open(file);
    fstat(file_descriptor, &statbuffer);

    if (statbuffer.st_size == -1)
        return;

    buffer = (uint8_t*)malloc(sizeof(char) * statbuffer.st_size);
    buffer_size = statbuffer.st_size;

    read(file_descriptor, buffer, statbuffer.st_size);
    close(file_descriptor);

    init();
}

Wave::~Wave()
{
    drwav_uninit(&wav);
    free(buffer);
}

bool Wave::load(uint8_t* data, size_t size)
{
    buffer = data;
    buffer_size = size;
    return init();
}

bool Wave::init()
{
    if ((!buffer) || (!buffer_size))
        return false;

    if (!drwav_init_memory(&wav, buffer, buffer_size, NULL))
        return false;

    channels = wav.channels;
    sample_rate = wav.sampleRate;
    sample_count = (size_t)wav.totalPCMFrameCount * wav.channels * sizeof(int32_t);
    return true;
}

void Wave::read_pcm_16(uint8_t* buffer)
{
    drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, (drwav_int16*)buffer);
}
