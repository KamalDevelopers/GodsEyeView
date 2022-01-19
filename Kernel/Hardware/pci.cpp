#include "pci.hpp"
#include "Drivers/amd79.hpp"

DeviceDescriptor::DeviceDescriptor()
{
}

DeviceDescriptor::~DeviceDescriptor()
{
}

PCI::PCI()
    : dataport(0xCFC)
    , commandport(0xCF8)
{
}

PCI::~PCI()
{
}

Driver* PCI::get_driver(DeviceDescriptor dev, InterruptManager* interrupts)
{
    Driver* driver = 0;
    switch (dev.vendor_id) {
    case 0x1022:
        switch (dev.device_id) {
        case 0x2000:
            driver = (AmdDriver*)kmalloc(sizeof(AmdDriver));
            if (driver != 0) {
                new (driver) AmdDriver(&dev, interrupts);
            }
            return driver;
        }
        break;
    }

    switch (dev.class_id) {
    case 0x03:
        switch (dev.subclass_id) {
        case 0x00:
            break;
        }
        break;
    }

    return driver;
}

BaseAddressRegister PCI::get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
{
    BaseAddressRegister result;
    uint32_t headertype = read(bus, device, function, 0x0E) & 0x7F;
    int max_bars = 6 - (4 * headertype);
    if (bar >= max_bars)
        return result;

    uint32_t bar_value = read(bus, device, function, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    uint32_t temp;

    if (result.type == MemoryMapping) {
        switch ((bar_value >> 1) & 0x3) {

        case 0: // 32 Bit Mode
        case 1: // 20 Bit Mode
        case 2: // 64 Bit Mode
            break;
        }

    } else // InputOutput
    {
        result.address = (uint8_t*)(bar_value & ~0x3);
        result.prefetchable = false;
    }
    return result;
}

uint32_t PCI::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (register_offset & 0xFC);

    commandport.write(id);
    uint32_t result = dataport.read();
    return result >> (8 * (register_offset % 4));
}

void PCI::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (register_offset & 0xFC);
    commandport.write(id);
    dataport.write(value);
}

bool PCI::device_has_functions(uint16_t bus, uint16_t device)
{
    return read(bus, device, 0, 0x0E) & (1 << 7);
}

void PCI::select_drivers(DriverManager* driver_manager, InterruptManager* interrupts)
{
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            int functions = device_has_functions(bus, device) ? 8 : 1;
            for (int function = 0; function < functions; function++) {
                dev = get_device_descriptor(bus, device, function);

                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
                    break;

                for (int bar_num = 0; bar_num < 6; bar_num++) {
                    BaseAddressRegister bar = get_base_address_register(bus, device, function, bar_num);
                    if (bar.address && (bar.type == InputOutput))
                        dev.port_base = (uint32_t)bar.address;
                }

                Driver* driver = get_driver(dev, interrupts);
                if (driver != 0) {
                    driver_manager->add_driver(driver);
                    klog("Drver amd79 activated");
                }
            }
        }
    }
}

DeviceDescriptor* PCI::get_descriptor()
{
    return &dev;
}

DeviceDescriptor PCI::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    DeviceDescriptor result;

    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = read(bus, device, function, 0x00);
    result.device_id = read(bus, device, function, 0x02);

    result.class_id = read(bus, device, function, 0x0b);
    result.subclass_id = read(bus, device, function, 0x0a);
    result.interface_id = read(bus, device, function, 0x09);

    result.revision = read(bus, device, function, 0x08);
    result.interrupt = read(bus, device, function, 0x3c);

    return result;
}
