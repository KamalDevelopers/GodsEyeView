#include "sb16.hpp"

SoundBlaster16* SoundBlaster16::active = 0;
SoundBlaster16::SoundBlaster16(InterruptManager* interrupt_manager)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + SB16_DEFAULT_IRQ)
    , mixer_port(0x224)
    , mixer_data_port(0x225)
    , reset_port(0x226)
    , read_port(0x22A)
    , write_port(0x22C)
    , read_status_port(0x22E)
{
    active = this;
    identify();
}

SoundBlaster16::~SoundBlaster16()
{
}

void SoundBlaster16::identify()
{
    reset_port.write(1);
    delay(32);
    reset_port.write(0);

    uint8_t data = dsp_read();
    if ((data == SB16_MAGIC) && !is_activated)
        activate();
}

void SoundBlaster16::activate()
{
    is_activated = true;
    dsp_write(0xE1);
    major_version = dsp_read();
    uint8_t minor_version = dsp_read();
    klog("SB16 version %d.%d", major_version, minor_version);

    /* Activate interrupts */
    mixer_port.write(0x80);
    mixer_data_port.write(SB16_DEFAULT_IRQ_BITMASK);

    set_sample_rate(48000);
}

void SoundBlaster16::dsp_write(uint8_t value)
{
    while (write_port.read() & 0x80)
        ;
    write_port.write(value);
}

uint8_t SoundBlaster16::dsp_read()
{
    while (!(read_status_port.read() & 0x80))
        ;
    return read_port.read();
}

void SoundBlaster16::set_sample_rate(uint16_t hz)
{
    sample_rate = hz;
    dsp_write(0x41);
    dsp_write((uint8_t)(hz >> 8));
    dsp_write((uint8_t)hz);
    dsp_write(0x42);
    dsp_write((uint8_t)(hz >> 8));
    dsp_write((uint8_t)hz);
}

void SoundBlaster16::dma_start(void* buffer, uint32_t length)
{
    uint8_t mode = 0x48;
    uint8_t channel = 5;

    outb(0xd4, 4 + (channel % 4));
    outb(0xd8, 1);

    outb(0xd6, (channel % 4) | mode | (1 << 4));

    uint16_t offset = (((uint32_t)buffer) / 2) % 65536;
    outb(0xc4, (uint8_t)((offset >> 0) & 0xFF));
    outb(0xc4, (uint8_t)((offset >> 8) & 0xFF));

    outb(0xc6, (uint8_t)(((length - 1) >> 0) & 0xFF));
    outb(0xc6, (uint8_t)(((length - 1) >> 8) & 0xFF));

    outb(0x8B, ((uint32_t)buffer) >> 16);
    outb(0xD4, channel % 4);
}

void SoundBlaster16::write(void* buffer, uint32_t length)
{
    dma_start(buffer, length);
    uint16_t sample_count = (length / 2) - 1;

    /* Transfer type and type of data */
    dsp_write(DSP_PLAY | DSP_PROG_16 | DSP_AUTO_INIT);
    dsp_write(DSP_SIGNED | DSP_STEREO);

    dsp_write((uint8_t)((sample_count >> 0) & 0xFF));
    dsp_write((uint8_t)((sample_count >> 8) & 0xFF));

    dsp_write(0xD1);
    dsp_write(0xD6);
}

uint32_t SoundBlaster16::interrupt(uint32_t esp)
{
    read_status_port.read();
    if (major_version >= 4)
        inb(0x22F);
    return esp;
}
