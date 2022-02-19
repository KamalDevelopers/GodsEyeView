#include "es1370.hpp"

MUTEX(es1370);

ES1370::ES1370(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
    , control_port(device.port_base + 0x00)
    , status_port(device.port_base + 0x04)
    , uart_data_port(device.port_base + 0x08)
    , uart_status_port(device.port_base + 0x09)
    , memory_page_port(device.port_base + 0x0C)
    , codec_port(device.port_base + 0x10)
    , legacy_port(device.port_base + 0x18)
    , serial_port(device.port_base + 0x20)
    , dac2_sample_port(device.port_base + 0x28)
    , dac2_buffer_port(device.port_base + 0x38)
    , dac2_buffer_size_port(device.port_base + 0x3C)
{
    PCI::active->enable_busmaster(device);
    activate();
}

void ES1370::activate()
{
    /* Enable codec */
    write_codec(0x16, 0x2);
    delay(20);
    write_codec(0x16, 0x3);
    delay(20);
    write_codec(0x17, 0x0);

    uint32_t control = control_port.read();
    control |= CDC_EN;
    control_port.write(control);

    /* Set master and mixer max volume */
    write_codec(CODEC_MASTER_VOLUME_L, 0x0);
    write_codec(CODEC_MASTER_VOLUME_R, 0x0);
    write_codec(CODEC_VOICE_VOLUME_L, 0x0);
    write_codec(CODEC_VOICE_VOLUME_R, 0x0);
    write_codec(CODEC_OUTPUT_MIX1, OUTPUT_MIX1_CDL | OUTPUT_MIX1_CDR);
    write_codec(CODEC_OUTPUT_MIX2, OUTPUT_MIX2_VOICEL | OUTPUT_MIX2_VOICER);
    delay(50000);

    set_sample_rate(8000);
}

void ES1370::set_sample_rate(uint16_t hz)
{
    sample_rate = hz;
    uint32_t ctrl = control_port.read() & ~CTRL_PCLKDIV;
    ctrl |= DAC2_SRTODIV((uint16_t)sample_rate) << CTRL_SH_PCLKDIV;
    control_port.write(ctrl);
}

void ES1370::write_codec(int reg, uint16_t value)
{
    while ((status_port.read() & 0x00000100) != 0)
        ;
    codec_port.write((reg << 8) | value);
}

void ES1370::write(uint8_t* buffer, uint32_t length)
{
    Mutex::lock(es1370);
    is_playing = true;

    /* Set sample counts and buffer length */
    /* FIXME: How should we properly calculate the sample count? */
    uint16_t sample_count = (length / 8) - 1;
    memory_page_port.write(0xC);
    dac2_buffer_port.write((uint32_t)buffer);
    dac2_buffer_size_port.write((length / 4) - 1);
    dac2_sample_port.write(sample_count);

    /* Looped 16 bit stereo playback */
    uint32_t sctrl = serial_port.read();
    sctrl &= ~(SCTRL_P2SEB | SCTRL_P2SMB);
    sctrl |= SCTRL_P2INTEN | SCTRL_P2SEB | SCTRL_P2SMB;
    sctrl |= (16 + 1) << SCTRL_SH_P2ENDINC;
    serial_port.write(sctrl);

    /* Start dac2 playback */
    uint32_t ctrl = control_port.read();
    control_port.write(ctrl | CTRL_DAC2_EN);
}

uint32_t ES1370::interrupt(uint32_t esp)
{
    uint32_t status = status_port.read();

    if (status & interrupt_mask) {
        uint32_t sctrl = serial_port.read();
        uint32_t ctrl = control_port.read();
        serial_port.write(sctrl & ~SCTRL_P2INTEN);
        serial_port.write(sctrl | SCTRL_P2INTEN);

        if (!is_init_irq) {
            /* Stop dac2 playback */
            control_port.write(ctrl & ~CTRL_DAC2_EN);
            Mutex::unlock(es1370);

            is_playing = false;
            is_init_irq = true;
        } else {
            is_init_irq = false;
        }
    }

    return esp;
}
