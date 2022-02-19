#include "audio.hpp"

Audio* Audio::active = 0;
Audio::Audio()
{
    active = this;
}

Audio::~Audio()
{
}

void Audio::set_audio_driver(AudioDriver* driver)
{
    audio_driver = driver;
    has_driver = true;
}

void Audio::write(uint8_t* buffer, uint32_t length)
{
    audio_driver->write(buffer, length);
}

void Audio::set_sample_rate(uint16_t hz)
{
    audio_driver->set_sample_rate(hz);
}
