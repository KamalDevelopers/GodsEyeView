#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "interrupts.hpp"
#include <LibSound/stream.hpp>

#define AUDIO Audio::active

class AudioDriver {
public:
    AudioDriver() { }
    ~AudioDriver() { }

    virtual void write(uint8_t* buffer, uint32_t length) { }
    virtual void set_sample_rate(uint16_t hz) { }
    virtual uint32_t chunk_size() { return 0; }
    virtual bool is_playing() { return false; }
    virtual void wait() { }
};

class Audio {
private:
    AudioDriver* audio_driver;
    uint32_t audio_position = 0;
    bool has_driver = false;

public:
    Audio();
    ~Audio();

    static Audio* active;
    void set_audio_driver(AudioDriver* driver);
    void write(pcm_stream_t buffer);
    void write(uint8_t* buffer, uint32_t length);
    void set_sample_rate(uint16_t hz);
    uint32_t current_audio_position() { return audio_position; }
    bool is_playing();
};

#endif
