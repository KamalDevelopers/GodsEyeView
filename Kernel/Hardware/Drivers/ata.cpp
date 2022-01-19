#include "ata.hpp"

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t port_base)
    : dataPort(port_base)
    , errorPort(port_base + 0x1)
    , sectorCountPort(port_base + 0x2)
    , lbaLowPort(port_base + 0x3)
    , lbaMidPort(port_base + 0x4)
    , lbaHiPort(port_base + 0x5)
    , devicePort(port_base + 0x6)
    , commandPort(port_base + 0x7)
    , controlPort(port_base + 0x206)
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}

void AdvancedTechnologyAttachment::identify()
{
    devicePort.write(master ? 0xA0 : 0xB0);
    controlPort.write(0);

    devicePort.write(0xA0);
    uint8_t status = commandPort.read();
    if (status == 0xFF)
        return;

    devicePort.write(master ? 0xA0 : 0xB0);
    sectorCountPort.write(0);
    lbaLowPort.write(0);
    lbaMidPort.write(0);
    lbaHiPort.write(0);
    commandPort.write(0xEC);

    status = commandPort.read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.read();

    if (status & 0x01)
        return;

    for (int i = 0; i < 256; i++) {
        uint16_t data = dataPort.read();
        char* text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
    }
}

uint8_t* AdvancedTechnologyAttachment::read28(uint32_t sector_num, uint8_t* data, int count)
{
    static uint8_t buffer[512];
    uint16_t index = 0;

    for (int i = 0; i < 512; i++)
        buffer[i] = 0;

    if (sector_num > 0x0FFFFFFF)
        return nullptr;

    devicePort.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    errorPort.write(0);
    sectorCountPort.write(1);
    lbaLowPort.write(sector_num & 0x000000FF);
    lbaMidPort.write((sector_num & 0x0000FF00) >> 8);
    lbaHiPort.write((sector_num & 0x00FF0000) >> 16);
    commandPort.write(0x20);

    uint8_t status = commandPort.read();
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.read();

    if (status & 0x01)
        return nullptr;

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = dataPort.read();

        char* f = "  \0";
        f[1] = (wdata >> 8) & 0x0FF;
        f[0] = wdata & 0x0FF;

        buffer[index] = f[0];
        buffer[index + 1] = f[1];
        index += 2;

        data[i] = wdata & 0x00FF;
        if (i + 1 < count)
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (int i = count + (count % 2); i < 512; i += 2)
        dataPort.read();

    buffer[index + 1] = '\0';
    return buffer;
}

void AdvancedTechnologyAttachment::write28(uint32_t sector_num, uint8_t* data, uint32_t count)
{
    if ((sector_num > 0x0FFFFFFF) || (count > 512))
        return;

    devicePort.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    errorPort.write(0);
    sectorCountPort.write(1);
    lbaLowPort.write(sector_num & 0x000000FF);
    lbaMidPort.write((sector_num & 0x0000FF00) >> 8);
    lbaHiPort.write((sector_num & 0x00FF0000) >> 16);
    commandPort.write(0x30);

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;
        dataPort.write(wdata);
    }

    for (int i = count + (count % 2); i < 512; i += 2)
        dataPort.write(0x0000);
}

void AdvancedTechnologyAttachment::flush()
{
    devicePort.write(master ? 0xE0 : 0xF0);
    commandPort.write(0xE7);

    uint8_t status = commandPort.read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.read();

    if (status & 0x01)
        return;
}
