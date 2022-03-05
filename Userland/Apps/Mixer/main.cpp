#include <LibC/pcm.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

bool play(char* file_name, uint16_t sample_rate)
{
    int file_descriptor = open(file_name, O_RDONLY);
    struct stat statbuffer;

    fstat(file_descriptor, &statbuffer);
    if (statbuffer.st_size == -1) {
        return false;
    }

    uint32_t size = statbuffer.st_size;
    uint8_t* data = (uint8_t*)malloc(size);
    read(file_descriptor, data, size);
    close(file_descriptor);

    pcm_header_t pcm;
    pcm.sample_rate = sample_rate;
    pcm.data = data;
    pcm.size = size;

    printf("Playing %s at %d Hz...", file_name, sample_rate);
    flush();

    audio_device_write(pcm);

    printf("%s", "\b\b\b [done]");
    free(data);
    return true;
}

int main(int argc, char** argv)
{
    if (!argc) {
        printf("Usage: mixer <file> <sample rate>");
        return 0;
    }

    uint16_t sample_rate = 44100;

    if (argc > 1)
        sample_rate = atoi(argv[1]);

    if (!play(argv[0], sample_rate))
        printf("File '%s' does not exist", argv[0]);

    return 0;
}
