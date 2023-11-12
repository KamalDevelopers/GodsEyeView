#ifndef AC97_HPP
#define AC97_HPP

#include "../../Locks/mutex.hpp"
#include "../audio.hpp"
#include "../interrupts.hpp"
#include "../pci.hpp"
#include "../port.hpp"

#define AC97_PCM_VOLUME(ch_right, ch_left, mute) \
    (((ch_right & 31) << 0)                      \
        | ((ch_left & 31) << 8)                  \
        | ((mute & 1) << 15))

#define AC97_MIXER_VOLUME(ch_right, ch_left, mute) \
    (((ch_right & 63) << 0)                        \
        | ((ch_left & 63) << 8)                    \
        | ((mute & 1) << 15))

#define AC97_VOLUME_MAX 0
#define AC97_VOLUME_MIN 63
#define GLOBAL_FLAG_INT 1 << 0
#define GLOBAL_FLAG_COLD_RESET 1 << 1

#define NABM_OFF_ADDRESS 0x0
#define NABM_OFF_NUM_PROCESSED 0x4
#define NABM_OFF_NUM_ENTRIES 0x5
#define NABM_OFF_TRANSFER_STATUS 0x6
#define NABM_OFF_NUM_TRANSFERED_SAMPLES 0x8
#define NABM_OFF_NUM_NEXT_ENTRY 0xA
#define NABM_OFF_TRANSFER_CONTROL 0xB

/* Extended functionality */
#define EXT_VAR_PCM 1 << 0
#define EXT_DBL_PCM 1 << 1
#define EXT_CTRL_VAR_PCM 1 << 0
#define EXT_CTRL_DBL_PCM 1 << 1

typedef struct buffer_descriptor {
    uint32_t physical_address;
    uint16_t samples;
    uint16_t flags;
} __attribute__((packed)) buffer_descriptor_t;

class AC97 : public InterruptHandler
    , public AudioDriver {
private:
    /* BAR0 */
    Port16Bit reset_port;
    Port16Bit master_volume_port;
    Port16Bit mono_volume_port;
    Port16Bit pcm_volume_port;
    Port16Bit extended_support_port;
    Port16Bit extended_control_port;
    Port16Bit front_sample_rate_port;

    /* BAR1 */
    Port32Bit global_control_port;
    Port32Bit global_status_port;
    void activate();

    uint32_t nabm_reg_pcm_in = 0;
    uint32_t nabm_reg_pcm_out = 0;
    uint32_t nabm_reg_mic = 0;

    uint8_t has_extended_variable_rate = 0;
    uint8_t has_extended_double_rate = 0;
    uint32_t sample_rate = 0;

    uint16_t entry_index = 0;

public:
    AC97(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~AC97();

    static driver_identifier_t identifier() { return { 0x0, 0x0, 0x4, 0x1 }; }
    uint32_t chunk_size() { return 0xFFFE; }
    uint32_t current_sample_rate();

    int set_volume(uint32_t percentage);
    void set_sample_rate(uint16_t hz);
    void write(uint8_t* buffer, uint32_t length);
    bool is_playing();
    void wait();

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
