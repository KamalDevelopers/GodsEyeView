#ifndef SB16_HPP
#define SB16_HPP

#define SB16_DEFAULT_IRQ 5
#define SB16_DEFAULT_IRQ_BITMASK 0b10
#define SB16_MAGIC 0xAA

#define DSP_PROG_16 0xB0
#define DSP_PROG_8 0xC0
#define DSP_AUTO_INIT 0x06
#define DSP_PLAY 0x00
#define DSP_RECORD 0x08
#define DSP_MONO 0x00
#define DSP_STEREO 0x20
#define DSP_UNSIGNED 0x00
#define DSP_SIGNED 0x10
#define CHUNK_SIZE PAGE_SIZE
#define SB16 SoundBlaster16::active

#include "../audio.hpp"
#include "../interrupts.hpp"
#include "../pci.hpp"
#include "../port.hpp"

class SoundBlaster16 : public InterruptHandler
    , public AudioDriver {
private:
    Port8Bit mixer_port;
    Port8Bit mixer_data_port;
    Port8Bit reset_port;
    Port8Bit read_port;
    Port8Bit write_port;
    Port8Bit read_status_port;

    bool is_activated = false;

    int major_version = 0;
    uint16_t sample_rate = 0;

    uint32_t current_position = 1;
    uint32_t total_size = 0;

    void dsp_write(uint8_t value);
    uint8_t dsp_read();
    void dma_start(void* buffer, uint32_t length);
    void activate();
    void identify();

public:
    SoundBlaster16(InterruptManager* interrupt_manager);
    ~SoundBlaster16();

    static SoundBlaster16* active;

    uint32_t chunk_size() { return PAGE_SIZE; }
    void write(uint8_t* buffer, uint32_t length);
    void wait();
    void set_sample_rate(uint16_t hz);

    bool activated() { return is_activated; }
    void start();
    void stop();

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
