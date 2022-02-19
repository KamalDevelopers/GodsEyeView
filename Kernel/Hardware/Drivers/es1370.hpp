#ifndef ES1370_HPP
#define ES1370_HPP

#include "../interrupts.hpp"
#include "../pci.hpp"
#include "../port.hpp"

#define CODEC_MASTER_VOLUME_L 0x00
#define CODEC_MASTER_VOLUME_R 0x01
#define CODEC_VOICE_VOLUME_L 0x02
#define CODEC_VOICE_VOLUME_R 0x03
#define CODEC_OUTPUT_MIX1 0x10
#define CODEC_OUTPUT_MIX2 0x11
#define BIT16_STEREO 0x20020C

#define SCTRL_P2INTEN 0x00000200
#define SCTRL_P1INTEN 0x00000100
#define SCTRL_P2SEB 0x00000008
#define SCTRL_P2SMB 0x00000004
#define SCTRL_SH_P2ENDINC 19
#define CTRL_DAC2_EN 0x00000020
#define CTRL_PCLKDIV 0x1FFF0000
#define CTRL_SH_PCLKDIV 16
#define XCTL0 0x0100
#define CDC_EN 0x0002

#define OUTPUT_MIX1_CDL (1 << 2)
#define OUTPUT_MIX1_CDR (1 << 1)
#define OUTPUT_MIX2_VOICEL (1 << 3)
#define OUTPUT_MIX2_VOICER (1 << 2)
#define DAC2_SRTODIV(x) (((1411200 + (x) / 2) / (x)-2) & 0x1FFF)

class ES1370 : public InterruptHandler {
private:
    Port32Bit control_port;
    Port32Bit status_port;
    Port8Bit uart_data_port;
    Port8Bit uart_status_port;
    Port32Bit memory_page_port;
    Port32Bit codec_port;
    Port32Bit legacy_port;
    Port32Bit serial_port;
    Port32Bit dac2_sample_port;
    Port32Bit dac2_buffer_port;
    Port32Bit dac2_buffer_size_port;

    bool is_playing = false;
    bool is_init_irq = true;
    uint16_t sample_rate = 0;
    const uint8_t interrupt_mask = 0x3;

public:
    ES1370(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~ES1370();

    static driver_identifier_t identifier() { return { 0x1274, 0x5000 }; }
    void set_sample_rate(uint16_t hz);
    void write_codec(int reg, uint16_t value);
    void write(uint8_t* buffer, uint32_t length);
    bool playing() { return is_playing; }
    void activate();

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
