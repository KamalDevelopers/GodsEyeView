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

void Audio::write(pcm_header_t pcm)
{
    /* TODO: Support dynamic audio modes */
    if ((pcm.bits != 16) || (pcm.channels != 2)) {
        klog("Received invalid pcm buffer channels=%d bits=%d", pcm.bits, pcm.channels);
        return;
    }

    set_sample_rate(pcm.sample_rate);
    uint32_t chunk_size = audio_driver->chunk_size();

    for (uint32_t i = 0; i < pcm.size; i += chunk_size) {
        uint32_t size = ((pcm.size - i) < chunk_size) ? pcm.size - i : chunk_size;
        if ((size != chunk_size) && (pcm.size > chunk_size))
            break;
        audio_driver->write(pcm.data + i, size);
    }

    audio_driver->wait();
}

void Audio::write(uint8_t* buffer, uint32_t length)
{
    pcm_header_t pcm;

    /* Assume that 12 byte data is a pcm buffer header */
    if (length == sizeof(pcm_header_t)) {
        memcpy((void*)&pcm, buffer, sizeof(pcm_header_t));
        return write(pcm);
    }

    pcm.data = buffer;
    pcm.size = length;
    return write(pcm);
}

void Audio::set_sample_rate(uint16_t hz)
{
    audio_driver->set_sample_rate(hz);
}
