#ifndef PCI_HPP
#define PCI_HPP

#include "../Mem/mm.hpp"
#include "Drivers/driver.hpp"
#include "LibC/stdlib.hpp"
#include "LibC/types.hpp"
#include "interrupts.hpp"
#include "port.hpp"

enum BaseAddressRegisterType {
    MemoryMapping = 0,
    InputOutput = 1
};

class BaseAddressRegister {
public:
    bool prefetchable;
    uint8_t* address;
    uint32_t size;
    BaseAddressRegisterType type;
};

class DeviceDescriptor {
public:
    uint32_t port_base;
    uint32_t interrupt;

    uint16_t bus;
    uint16_t device;
    uint16_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;

    uint8_t revision;

    DeviceDescriptor();
    ~DeviceDescriptor();
};

class PCI {
    Port32Bit dataport;
    Port32Bit commandport;
    DeviceDescriptor dev;

public:
    PCI();
    ~PCI();

    uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
    void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);
    bool device_has_functions(uint16_t bus, uint16_t device);

    void select_drivers(DriverManager* driver_manager, InterruptManager* interrupts);
    DeviceDescriptor get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
    DeviceDescriptor* get_descriptor();
    Driver* get_driver(DeviceDescriptor dev, InterruptManager* interrupts);
    BaseAddressRegister get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
};

#endif
