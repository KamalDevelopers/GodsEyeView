#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <LibC/types.hpp>

#define AUDIO Audio::active

class AudioDriver {
public:
    AudioDriver() { }
    ~AudioDriver() { }

    virtual void write(uint8_t* buffer, uint32_t length) { }
    virtual void set_sample_rate(uint16_t hz) { }
    virtual bool playing() { return 0; }
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
    void write(uint8_t* buffer, uint32_t length);
    void set_sample_rate(uint16_t hz);
    bool playing();
};

#endif
