#ifndef SOUND_HPP
#define SOUND_HPP

#include <LibC++/vector.hpp>
#include <LibC/liballoc.hpp>
#include <LibC/poll.hpp>
#include <LibC/stat.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/unistd.hpp>
#include <LibSound/connection.hpp>
#include <LibSound/stream.hpp>
#include <LibSound/wave.hpp>

#define MAX_AUDIO_STREAMS 20

typedef struct audio_stream {
    pcm_stream_t pcm;
    int unique_id = 0;
    uint32_t position = 0;
    int events_file = 0;
    bool playback_started = false;
    bool sent_done_event = false;
} audio_stream_t;

class SoundServer {
private:
    int audio_slave_pid = -1;
    int sound_slave_file = 0;
    int audio_device_file = 0;
    uint8_t master_channel_volume = 100;
    Vector<audio_stream, MAX_AUDIO_STREAMS> streams;
    pcm_stream_t main_audio_stream;

public:
    SoundServer();
    ~SoundServer();

    void kill_audio_slave();
    void spawn_audio_slave();
    bool audio_slave_running();

    void play_file(char* file, int unique_id);
    void change_master_volume(uint8_t volume);
    uint32_t audio_playback_position();
    void update_positions();
    int delete_stream(int stream_index);
    int find_stream_with_id(int unique_id);

    int16_t sample_volume_s16(int16_t sample, uint8_t volume);
    void send_stream_events();
    int send_stream_event(int stream_index, stream_event_t* event);

    int16_t mix_samples_s16(int16_t s1, int16_t s2);
    int mix_stream_with_main_s16(int stream_index);
    void mix_streams();
};

#endif
