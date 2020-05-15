#include "ata.hpp"

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, uint16_t portBase)
    : dataPort(portBase)
    , errorPort(portBase + 0x1)
    , sectorCountPort(portBase + 0x2)
    , lbaLowPort(portBase + 0x3)
    , lbaMidPort(portBase + 0x4)
    , lbaHiPort(portBase + 0x5)
    , devicePort(portBase + 0x6)
    , commandPort(portBase + 0x7)
    , controlPort(portBase + 0x206)
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}

void AdvancedTechnologyAttachment::Identify()
{
    devicePort.Write(master ? 0xA0 : 0xB0);
    controlPort.Write(0);

    devicePort.Write(0xA0);
    uint8_t status = commandPort.Read();
    if (status == 0xFF)
        return;

    devicePort.Write(master ? 0xA0 : 0xB0);
    sectorCountPort.Write(0);
    lbaLowPort.Write(0);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
    commandPort.Write(0xEC);

    status = commandPort.Read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if (status & 0x01) {
        printf("%s", "IDENTIFY ERROR");
        return;
    }

    for (int i = 0; i < 256; i++) {
        uint16_t data = dataPort.Read();
        char* text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
    }
    printf("\n");
}

char* AdvancedTechnologyAttachment::Read28(uint32_t sectorNum, int count)
{
    uint16_t index = 0;
    char* buffer;
    for (int i = 0; i < 512; i++)
        buffer[i] = '\0';

    if (sectorNum > 0x0FFFFFFF)
        return nullptr;

    devicePort.Write((master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24));
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(sectorNum & 0x000000FF);
    lbaMidPort.Write((sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write((sectorNum & 0x00FF0000) >> 16);
    commandPort.Write(0x20);

    uint8_t status = commandPort.Read();
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if (status & 0x01)
        return nullptr;

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = dataPort.Read();

        char* f = "  \0";
        f[1] = (wdata >> 8) & 0x0FF;
        f[0] = wdata & 0x0FF;

        buffer[index] = f[0];
        buffer[index + 1] = f[1];
        index += 2;
    }

    for (int i = count + (count % 2); i < 512; i += 2)
        dataPort.Read();

    buffer[index + 1] = '\0';
    printf("\n");
    return buffer;
}

void AdvancedTechnologyAttachment::Write28(uint32_t sectorNum, uint8_t* data, uint32_t count)
{
    if (sectorNum > 0x0FFFFFFF)
        return;
    if (count > 512)
        return;

    devicePort.Write((master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24));
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(sectorNum & 0x000000FF);
    lbaMidPort.Write((sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write((sectorNum & 0x00FF0000) >> 16);
    commandPort.Write(0x30);

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;
        dataPort.Write(wdata);
    }

    for (int i = count + (count % 2); i < 512; i += 2)
        dataPort.Write(0x0000);
}

void AdvancedTechnologyAttachment::Flush()
{
    devicePort.Write(master ? 0xE0 : 0xF0);
    commandPort.Write(0xE7);

    uint8_t status = commandPort.Read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if (status & 0x01) {
        printf("%s", "FLUSH ERROR");
        return;
    }
}