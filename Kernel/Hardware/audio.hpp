#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "interrupts.hpp"
#include <LibC/pcm.hpp>

#define AUDIO Audio::active

class AudioDriver {
public:
    AudioDriver() { }
    ~AudioDriver() { }

    virtual void write(uint8_t* buffer, uint32_t length) { }
    virtual void set_sample_rate(uint16_t hz) { }
    virtual uint32_t chunk_size() { return 0; }
    virtual void wait() { }
};

class Audio {
private:
    AudioDriver* audio_driver;
    bool has_driver = false;

public:
    Audio();
    ~Audio();

    static Audio* active;
    void set_audio_driver(AudioDriver* driver);
    void write(pcm_header_t buffer);
    void write(uint8_t* buffer, uint32_t length);
    void set_sample_rate(uint16_t hz);
};

#endif
