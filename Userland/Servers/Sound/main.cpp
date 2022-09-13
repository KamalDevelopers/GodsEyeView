#include "communication.hpp"
#include "sound.hpp"
#include <LibC/poll.h>
#include <LibC/string.h>
#include <LibC/unistd.h>

int slave_audio_device_write(int slave_file)
{
    usleep(1000);
    pcm_stream_t pcm;
    int audio_device = open("/dev/audio", O_RDWR);
    read(slave_file, (void*)&pcm, sizeof(pcm_stream_t));
    write(audio_device, (void*)&pcm, sizeof(pcm_stream_t));
    close(audio_device);
    return 0;
}

int main(int argc, char** argv)
{
    nice(-1);
    int slave_file = open("/pipe/sound-slave", O_RDWR);
    if (slave_file != -1)
        return slave_audio_device_write(slave_file);

    SoundServer sound_server;
    int client_communication_file = init_communications();
    struct pollfd polls[1];
    polls[0].events = POLLIN;
    polls[0].fd = client_communication_file;

    while (1) {
        poll(polls, 1);
        receive_connections(&sound_server);
    }

    return 0;
}
