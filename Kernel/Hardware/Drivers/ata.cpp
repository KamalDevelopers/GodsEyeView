#include "ata.hpp"

MUTEX(ata);
extern char _binary_hdd_tar_start;
extern char _binary_hdd_tar_end;
#define RAMDISK

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t port_base)
    : data_port(port_base)
    , error_port(port_base + 0x1)
    , sector_count_port(port_base + 0x2)
    , lba_low_port(port_base + 0x3)
    , lba_mid_port(port_base + 0x4)
    , lba_hi_port(port_base + 0x5)
    , device_port(port_base + 0x6)
    , command_port(port_base + 0x7)
    , control_port(port_base + 0x206)
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}

void AdvancedTechnologyAttachment::identify()
{
    device_port.write(master ? 0xA0 : 0xB0);
    control_port.write(0);

    device_port.write(0xA0);
    uint8_t status = command_port.read();
    if (status == 0xFF)
        return;

    device_port.write(master ? 0xA0 : 0xB0);
    sector_count_port.write(0);
    lba_low_port.write(0);
    lba_mid_port.write(0);
    lba_hi_port.write(0);
    command_port.write(0xEC);

    status = command_port.read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01)
        return;

    for (int i = 0; i < 256; i++) {
        uint16_t data = data_port.read();
        char* text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
    }
}

uint8_t* AdvancedTechnologyAttachment::read28(uint32_t sector_num, uint8_t* data, int count)
{
#ifdef RAMDISK
    uint8_t* disk = (uint8_t*)&_binary_hdd_tar_start;
    memcpy(data, disk + sector_num * 512, count);
    return data;
#endif

    Mutex::lock(ata);
    static uint8_t buffer[BUFSIZ];
    uint16_t index = 0;

    for (int i = 0; i < BUFSIZ; i++)
        buffer[i] = 0;

    if (sector_num > 0x0FFFFFFF) {
        Mutex::unlock(ata);
        return nullptr;
    }

    device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_num & 0x000000FF);
    lba_mid_port.write((sector_num & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_num & 0x00FF0000) >> 16);
    command_port.write(0x20);

    uint8_t status = command_port.read();
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01) {
        Mutex::unlock(ata);
        return nullptr;
    }

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data_port.read();

        buffer[index] = wdata & 0x0FF;
        buffer[index + 1] = (wdata >> 8) & 0x0FF;
        index += 2;

        data[i] = wdata & 0x00FF;
        if (i + 1 < count)
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.read();

    buffer[index + 1] = '\0';
    Mutex::unlock(ata);
    return buffer;
}

void AdvancedTechnologyAttachment::write28(uint32_t sector_num, uint8_t* data, uint32_t count)
{
#ifdef RAMDISK
    return;
#endif
    if ((sector_num > 0x0FFFFFFF) || (count > BUFSIZ))
        return;

    Mutex::lock(ata);
    device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_num & 0x000000FF);
    lba_mid_port.write((sector_num & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_num & 0x00FF0000) >> 16);
    command_port.write(0x30);

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;
        data_port.write(wdata);
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.write(0x0000);
    Mutex::unlock(ata);
}

void AdvancedTechnologyAttachment::flush()
{
#ifdef RAMDISK
    return;
#endif
    device_port.write(master ? 0xE0 : 0xF0);
    command_port.write(0xE7);

    uint8_t status = command_port.read();
    if (status == 0x00)
        return;

    Mutex::lock(ata);
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();
    Mutex::unlock(ata);

    if (status & 0x01)
        return;
}
