#include "ac97.hpp"

MUTEX(mutex_ac97);

static buffer_descriptor_t descriptors[32];

AC97::AC97(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
    , reset_port(device.bar0 + 0x00)
    , master_volume_port(device.bar0 + 0x02)
    , mono_volume_port(device.bar0 + 0x06)
    , pcm_volume_port(device.bar0 + 0x18)
    , extended_support_port(device.bar0 + 0x28)
    , extended_control_port(device.bar0 + 0x2A)
    , front_sample_rate_port(device.bar0 + 0x2C)
    , global_control_port(device.bar1 + 0x2C)
    , global_status_port(device.bar1 + 0x30)
    , nabm_reg_pcm_in(device.bar1 + 0x00)
    , nabm_reg_pcm_out(device.bar1 + 0x10)
    , nabm_reg_mic(device.bar1 + 0x20)
{
    /* kdbg("ac97 vend_id=%x\n", device.vendor_id);
    kdbg("ac97 dev_id=%x\n", device.device_id); */

    memset(descriptors, 0, 32 * sizeof(buffer_descriptor_t));
    PCI::active->enable_busmaster(device);
    activate();
}

void AC97::activate()
{
    /*  Cold reset and interrupts */
    uint32_t control = global_control_port.read();
    control |= GLOBAL_FLAG_INT;
    control |= GLOBAL_FLAG_COLD_RESET;
    global_control_port.write(control);

    /* Mixer reset */
    reset_port.write(1);

    /* Extended */
    uint16_t extended_support = extended_support_port.read();
    uint16_t extended_status = extended_control_port.read();

    /* Support for variable PCM and double rate */
    if ((extended_support & EXT_VAR_PCM) > 0) {
        /* kdbg("ac97 has varpcm\n"); */
        extended_status |= EXT_CTRL_VAR_PCM;
        has_extended_variable_rate = 1;
    }
    if ((extended_support & EXT_DBL_PCM) > 0) {
        /* kdbg("ac97 has dbl\n"); */
        extended_status |= EXT_CTRL_DBL_PCM;
        has_extended_double_rate = 1;
    }

    /* Enable extensions */
    extended_control_port.write(extended_status);

    sample_rate = current_sample_rate();
    kdbg("sample_rate=%d\n", sample_rate);
    pcm_volume_port.write(AC97_PCM_VOLUME(0, 0, 0));
    master_volume_port.write(AC97_MIXER_VOLUME(0, 0, 0));
    set_volume(85);

    uint32_t transfer_control = nabm_reg_pcm_out + NABM_OFF_TRANSFER_CONTROL;
    outb(transfer_control, inb(transfer_control) | 2);
    while (inb(transfer_control) & 2)
        ;
    // outbl(transfer_control, inbl(transfer_control) | 0x8 | 0x4 | 0x10);
}

uint32_t AC97::current_sample_rate()
{
    return (uint32_t)(front_sample_rate_port.read() << has_extended_double_rate);
}

void AC97::wait()
{
    Mutex::lock(mutex_ac97);
    Mutex::unlock(mutex_ac97);
}

bool AC97::is_playing()
{
    return mutex_ac97.locked;
}

int AC97::set_volume(uint32_t percentage)
{
    int volume = AC97_VOLUME_MIN - (AC97_VOLUME_MIN * percentage / 100);
    pcm_volume_port.write(AC97_MIXER_VOLUME(volume, volume, 0));
    master_volume_port.write(AC97_MIXER_VOLUME(volume, volume, 0));
    mono_volume_port.write(volume);
    return 0;
}

void AC97::set_sample_rate(uint16_t hz)
{
    if (hz == sample_rate)
        return;

    sample_rate = hz;
    uint32_t shifted_sample_rate = sample_rate >> has_extended_double_rate;
    front_sample_rate_port.write(shifted_sample_rate);
}

void AC97::write(uint8_t* buffer, uint32_t length)
{
    if (length >= 0x20000)
        return;

    Mutex::lock(mutex_ac97);

    uint32_t transfer_control = nabm_reg_pcm_out + NABM_OFF_TRANSFER_CONTROL;
    outb(transfer_control, inb(transfer_control) | 2);
    while (inb(transfer_control) & 2)
        ;

    /* Set sample counts and buffer length */
    int sample_count = length / sizeof(int16_t);

    uint16_t pcm_out_status = inbw(nabm_reg_pcm_out + NABM_OFF_TRANSFER_STATUS);
    uint8_t current_index = inbw(nabm_reg_pcm_out + NABM_OFF_NUM_PROCESSED);
    uint8_t last_valid_index = inbw(nabm_reg_pcm_out + NABM_OFF_NUM_ENTRIES);
    /* kdbg("e=%d ci=%d lvi=%d pcmout=%d\n", entry_index, current_index, last_valid_index, pcm_out_status); */

    /* Data transfer */
    descriptors[entry_index].physical_address = (uint32_t)buffer;
    descriptors[entry_index].samples = sample_count;
    descriptors[entry_index].flags = 0x8000;

    outbl(nabm_reg_pcm_out + NABM_OFF_ADDRESS, (uint32_t)descriptors);
    outbl(nabm_reg_pcm_out + NABM_OFF_NUM_ENTRIES, entry_index);
    outbl(nabm_reg_pcm_out + NABM_OFF_TRANSFER_STATUS, 0x1);
    outbl(transfer_control, inbl(transfer_control) | 0x1);
}

uint32_t AC97::interrupt(uint32_t esp)
{
    uint32_t status = global_status_port.read();
    uint32_t transfer_status = inbw(nabm_reg_pcm_out + NABM_OFF_TRANSFER_STATUS);

    if (transfer_status & 0x4)
        Mutex::unlock(mutex_ac97);

    // global_status_port.write(0x1c);
    outbw(nabm_reg_pcm_out + NABM_OFF_TRANSFER_STATUS, transfer_status | 0x1C);
    // outbw(nabm_reg_pcm_in + NABM_OFF_TRANSFER_STATUS, 0x1C);

    Mutex::unlock(mutex_ac97);
    return esp;
}
