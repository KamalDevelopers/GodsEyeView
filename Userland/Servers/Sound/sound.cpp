#include "sound.hpp"
#include <LibC/mem.h>
#include <LibC/poll.h>
#include <LibC/stat.h>
#include <LibC/stdlib.h>
#include <LibC/unistd.h>

SoundServer::SoundServer()
{
    audio_device_file = open("/dev/audio", O_RDWR);
    sound_slave_file = mkfifo("/pipe/sound-slave", O_RDWR);
    main_audio_stream.data = (uint8_t*)malloc(1024);
}

SoundServer::~SoundServer()
{
    free(main_audio_stream.data);
}

void SoundServer::spawn_audio_slave()
{
    write(sound_slave_file, &main_audio_stream, sizeof(pcm_stream_t));
    audio_slave_pid = spawn("servers/sound", 0, 0);
}

void SoundServer::kill_audio_slave()
{
    if (audio_slave_pid == -1)
        return;

    struct pollfd polls[1];
    polls[0].events = POLLOUT;
    polls[0].fd = audio_device_file;

    poll(polls, 1, 0);
    kill(audio_slave_pid, 2);
}

bool SoundServer::audio_slave_running()
{
    return (kill(audio_slave_pid, 0) != -1);
}

int SoundServer::delete_stream(int stream_index)
{
    if ((stream_index >= streams.size()) || (stream_index < 0))
        return -1;
    if (streams[stream_index].pcm.data) {
        free(streams[stream_index].pcm.data);
        streams[stream_index].pcm.data = 0;
    }
    return streams.remove_at(stream_index);
}

uint32_t SoundServer::audio_playback_position()
{
    uint32_t buffer[1];
    read(audio_device_file, buffer, sizeof(uint32_t));
    return buffer[0];
}

int SoundServer::send_stream_event(int stream_index, stream_event_t* event)
{
    write(streams.at(stream_index).events_file, event, sizeof(stream_event_t));
    return 0;
}

void SoundServer::send_stream_events()
{
    uint32_t audio_position = audio_playback_position();
    bool slave_running = audio_slave_running();
    for (uint32_t i = 0; i < streams.size(); i++) {
        if ((streams[i].sent_done_event) || (!streams[i].playback_started))
            continue;
        if ((streams[i].position + audio_position < streams[i].pcm.size) && slave_running)
            continue;
        if ((audio_position <= 256000) && slave_running)
            continue;

        stream_event_t event;
        event.type = SOUND_EVENT_DONE;
        if (send_stream_event(i, &event) == 0)
            streams[i].sent_done_event = true;
    }
}

void SoundServer::update_positions()
{
    uint32_t audio_position = audio_playback_position();
    if (!audio_position) {
        int size = streams.size();
        for (uint32_t i = 0; i < size; i++)
            delete_stream(0);
        return;
    }

    for (uint32_t i = 0; i < streams.size(); i++) {
        streams[i].position += audio_position;
        if (streams[i].position >= streams[i].pcm.size) {
            streams[i].position = 0;
            streams[i].pcm.size = 0;
        }
    }
}

void SoundServer::change_master_volume(uint8_t volume)
{
    if ((volume > 100) || !volume)
        return;
    master_channel_volume = volume;
}

int16_t SoundServer::sample_volume_s16(int16_t sample, uint8_t volume)
{
    long return_sample = sample;
    return_sample *= volume;
    return_sample >>= 7;
    return (int16_t)return_sample;
}

int16_t SoundServer::mix_samples_s16(int16_t s1, int16_t s2)
{
    int first_sample = s1;
    int second_sample = s2;
    int result_sample = 0;
    first_sample += 32768;
    second_sample += 32768;

    if ((first_sample < 32768) || (second_sample < 32768))
        result_sample = first_sample * second_sample / 32768;
    else
        result_sample = 2 * (first_sample + second_sample) - (first_sample * second_sample) / 32768 - 65536;

    if (result_sample == 65536)
        result_sample = 65535;
    result_sample -= 32768;
    return (int16_t)result_sample;
}

int SoundServer::mix_stream_with_main_s16(int stream_index)
{
    if ((stream_index >= streams.size()) || (stream_index < 0))
        return -1;

    int16_t* samples = (int16_t*)streams.at(stream_index).pcm.data;
    int16_t* main_samples = (int16_t*)main_audio_stream.data;
    uint32_t size = streams.at(stream_index).pcm.size / sizeof(int16_t);
    uint32_t position = streams.at(stream_index).position / sizeof(int16_t);

    for (uint32_t i = 0; i < size - position; i++) {
        int16_t stream_sample = samples[i + position];
        int16_t main_sample = main_samples[i];
        main_samples[i] = mix_samples_s16(main_sample, stream_sample);
    }
    return 0;
}

void SoundServer::mix_streams()
{
    kill_audio_slave();
    uint32_t audio_stream_size = 0;
    uint32_t largest_stream = 0;

    for (uint32_t i = 0; i < streams.size(); i++) {
        int size = streams.at(i).pcm.size - streams.at(i).position;
        if (size > audio_stream_size) {
            audio_stream_size = size;
            largest_stream = i;
        }
    }

    main_audio_stream.data = (uint8_t*)realloc(main_audio_stream.data, audio_stream_size);
    main_audio_stream.size = audio_stream_size;
    memcpy(main_audio_stream.data, streams.at(largest_stream).pcm.data + streams.at(largest_stream).position, audio_stream_size);

    for (uint32_t i = 0; i < streams.size(); i++) {
        if ((i == largest_stream) || (!streams.at(i).pcm.size))
            continue;
        mix_stream_with_main_s16(i);
    }

    int size = audio_stream_size / sizeof(int16_t);
    for (uint32_t i = 0; i < size; i++) {
        int16_t* sample = &((int16_t*)main_audio_stream.data)[i];
        *sample = sample_volume_s16(*sample, master_channel_volume);
    }

    stream_event_t event;
    event.type = SOUND_EVENT_PLAYBACK;
    send_stream_event(streams.size() - 1, &event);
    spawn_audio_slave();
    streams.last().playback_started = true;
}

void SoundServer::play_file(char* file, int unique_id)
{
    Wave* wave = new Wave(file);
    if (!wave->is_valid()) {
        delete wave;
        return;
    }

    char id[10];
    char events_file_name[50];
    memset(id, 0, sizeof(id));
    memset(events_file_name, 0, sizeof(events_file_name));
    itoa(unique_id, id);
    strcat(events_file_name, "/pipe/sound-events-");
    strcat(events_file_name, id);

    update_positions();
    wave->disown_data();
    audio_stream_t audio_stream;
    audio_stream.position = 0;
    audio_stream.unique_id = unique_id;
    audio_stream.pcm.size = wave->size();
    audio_stream.pcm.data = wave->samples();
    audio_stream.events_file = mkfifo(events_file_name, O_RDWR | O_APPEND);
    streams.append(audio_stream);
    mix_streams();
    delete wave;
}

int SoundServer::find_stream_with_id(int unique_id)
{
    for (uint32_t i = 0; i < streams.size(); i++)
        if (streams.at(i).unique_id == unique_id)
            return i;
    return -1;
}
