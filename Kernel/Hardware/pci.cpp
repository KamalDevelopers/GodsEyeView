#include "pci.hpp"

PCI* PCI::active = 0;
PCI::PCI()
    : data_port(0xCFC)
    , command_port(0xCF8)
{
    active = this;
}

PCI::~PCI()
{
}

void PCI::get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar_num)
{
    uint32_t headertype = read(bus, device, function, 0x0E) & 0x7F;
    int max_bars = 6 - (4 * headertype);
    if (bar_num >= max_bars)
        return;

    uint32_t bar_value = read(bus, device, function, 0x10 + 4 * bar_num);
    bar.type = (bar_value & 0x1) ? 1 : 0;
    uint32_t temp;

    if (bar.type == 0) {
        switch ((bar_value >> 1) & 0x3) {
        case 0:
        case 1:
        case 2:
            break;
        }

    } else {
        bar.address = (uint8_t*)(bar_value & ~0x3);
        bar.prefetchable = false;
    }
}

uint32_t PCI::read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (register_offset & 0xFC);

    command_port.write(id);
    uint32_t dev = data_port.read();
    return dev >> (8 * (register_offset % 4));
}

void PCI::write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value)
{
    uint32_t id = 0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (register_offset & 0xFC);
    command_port.write(id);
    data_port.write(value);
}

bool PCI::device_has_functions(uint16_t bus, uint16_t device)
{
    return read(bus, device, 0, 0x0E) & (1 << 7);
}

bool PCI::find_driver(driver_identifier_t identifier)
{
    for (int bus = 0; bus < 8; bus++) {
        for (int device = 0; device < 32; device++) {
            if (find_driver(identifier, bus, device))
                return true;
        }
    }

    return false;
}

bool PCI::find_driver(driver_identifier_t identifier, uint16_t bus, uint16_t device)
{
    int functions = device_has_functions(bus, device) ? 8 : 1;
    for (int function = 0; function < functions; function++) {
        get_device_descriptor(bus, device, function);

        if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xFFFF)
            break;

        for (int bar_num = 0; bar_num < 6; bar_num++) {
            get_base_address_register(bus, device, function, bar_num);
            if (bar.address && (bar.type == 1) && (bar_num == 0))
                dev.bar0 = (uint32_t)bar.address;
            if (bar.address && (bar.type == 1) && (bar_num == 1))
                dev.bar1 = (uint32_t)bar.address;
            if (bar.address && (bar.type == 1) && (bar_num == 2))
                dev.bar2 = (uint32_t)bar.address;
            if (bar.address && (bar.type == 1) && (bar_num == 3))
                dev.bar3 = (uint32_t)bar.address;
        }

        if ((dev.vendor_id == identifier.vendor_id) && (dev.device_id == identifier.device_id))
            return true;

        if ((dev.class_id == identifier.class_id) && (dev.subclass_id == identifier.subclass_id))
            return true;
    }

    return false;
}

device_descriptor_t PCI::get_descriptor()
{
    return dev;
}

void PCI::get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function)
{
    dev.bus = bus;
    dev.device = device;
    dev.function = function;

    dev.vendor_id = read(bus, device, function, 0x00);
    dev.device_id = read(bus, device, function, 0x02);

    dev.class_id = read(bus, device, function, 0x0b);
    dev.subclass_id = read(bus, device, function, 0x0a);
    dev.interface_id = read(bus, device, function, 0x09);

    dev.revision = read(bus, device, function, 0x08);
    dev.interrupt = read(bus, device, function, 0x3c);
}

void PCI::enable_busmaster(device_descriptor_t device)
{
    uint16_t cmd = read(device.bus, device.device, device.function, 0x04);
    uint16_t status = read(device.bus, device.device, device.function, 0x06);
    cmd |= (1 << 2);
    write(device.bus, device.device, device.function, 0x04, (uint32_t)status << 16 | (uint32_t)cmd);
}
