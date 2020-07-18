#include "pci.hpp"

PCIcontrollerDeviceDescriptor::PCIcontrollerDeviceDescriptor()
{
}

PCIcontrollerDeviceDescriptor::~PCIcontrollerDeviceDescriptor()
{
}

PCIcontroller::PCIcontroller()
    : dataPort(0xCFC)
    , commandPort(0xCF8)
{
}

PCIcontroller::~PCIcontroller()
{
}

uint32_t PCIcontroller::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);

    commandPort.Write(id);
    uint32_t result = dataPort.Read();
    return result >> (8 * (registerOfsset % 4));
}

void PCIcontroller::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset, uint32_t value)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (registerOfsset & 0xFC);
    commandPort.Write(id);
    dataPort.Write(value);
}

bool PCIcontroller::DeviceHasFunctions(uint16_t bus, uint16_t device)
{
    return Read(bus, device, 0, 0x0E) & (1 << 7);
}

void PCIcontroller::SelectDrivers(DriverManager* driverManager)
{
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
            for (int function = 0; function < numFunctions; function++) {
                dev = GetDeviceDescriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                    break;

                /*printf("%s", "PCI BUS ");
                printf("%x", bus & 0xFF);

                printf("%s", ", DEVICE ");
                printf("%x", device & 0xFF);

                printf("%s", ", FUNCTION ");
                printf("%x", function & 0xFF);

                printf("%s", " = VENDOR ");
                printf("%x", (dev.vendor_id & 0xFF00) >> 8);
                printf("%x", dev.vendor_id & 0xFF);
                printf("%s", ", DEVICE ");
                printf("%x", (dev.device_id & 0xFF00) >> 8);
                printf("%x\n", dev.device_id & 0xFF);*/
            }
        }
    }
}

PCIcontrollerDeviceDescriptor* PCIcontroller::GetDescriptor()
{
    return &dev;
}

PCIcontrollerDeviceDescriptor PCIcontroller::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    PCIcontrollerDeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = Read(bus, device, function, 0x00);
    result.device_id = Read(bus, device, function, 0x02);

    result.class_id = Read(bus, device, function, 0x0b);
    result.subclass_id = Read(bus, device, function, 0x0a);
    result.interface_id = Read(bus, device, function, 0x09);

    result.revision = Read(bus, device, function, 0x08);
    result.interrupt = Read(bus, device, function, 0x3c);

    return result;
}