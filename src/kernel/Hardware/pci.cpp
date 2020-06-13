#include <pci.hpp>

PCIcontroller::PCIcontroller()
    : dataPort(0xCFC),
      commandPort(0xCF8)
{
}

PCIcontroller::~PCIcontroller() 
{
}

uint32_t PCIcontroller::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset)
{
    uint32_t id =   
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);

    commandPort.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8* (registerOfsset % 4));
}

void PCIcontroller::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset, uint32_t value)
{
    uint32_t id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value);
}

bool PCIcontroller::DeviceHasFunctions(uint16_t bus, uint16_t device) 
{
    return Read(bus, device, 0, 0x0E) & (1<<7);
}

