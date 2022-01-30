#include "pci.hpp"

PCI::PCI()
    : dataport(0xCFC)
    , commandport(0xCF8)
{
}

PCI::~PCI()
{
}

base_address_register_t PCI::get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar)
{
    base_address_register_t result;
    uint32_t headertype = read(bus, device, function, 0x0E) & 0x7F;
    int max_bars = 6 - (4 * headertype);
    if (bar >= max_bars)
        return result;

    uint32_t bar_value = read(bus, device, function, 0x10 + 4 * bar);
    result.type = (bar_value & 0x1) ? 1 : 0;
    uint32_t temp;

    if (result.type == 0) {
        switch ((bar_value >> 1) & 0x3) {
        case 0:
        case 1:
        case 2:
            break;
        }

    } else {
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

void PCI::select_driver(Driver* drivers[], size_t size, int bus, int device)
{
    int functions = device_has_functions(bus, device) ? 8 : 1;
    for (int function = 0; function < functions; function++) {
        dev = get_device_descriptor(bus, device, function);

        if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
            break;

        for (int bar_num = 0; bar_num < 6; bar_num++) {
            base_address_register_t bar = get_base_address_register(bus, device, function, bar_num);
            if (bar.address && (bar.type == 1))
                dev.port_base = (uint32_t)bar.address;
        }

        for (size_t i = 0; i < size; i++) {
            Driver* driver = drivers[i];
            driver_identifier_t identifer = driver->identify();

            if ((dev.vendor_id == identifer.vendor_id) && (dev.device_id == identifer.device_id))
                driver->activate();

            if ((dev.class_id == identifer.class_id) && (dev.subclass_id == identifer.subclass_id))
                driver->activate();
        }
    }
}

void PCI::select_drivers(Driver* drivers[], size_t size)
{
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            select_driver(drivers, size, bus, device);
        }
    }
}

device_descriptor_t* PCI::get_descriptor()
{
    return &dev;
}

device_descriptor_t PCI::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    device_descriptor_t result;

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
