#ifndef PCI_HPP
#define PCI_HPP

#include "../Mem/mm.hpp"
#include "interrupts.hpp"
#include "port.hpp"
#include <LibC/stdlib.h>
#include <LibC/types.h>

typedef struct base_address_register {
    bool prefetchable;
    uint8_t* address;
    uint32_t size;
    uint8_t type;
} base_address_register_t;

typedef struct device_descriptor {
    uint32_t bar0 = 0;
    uint32_t bar1 = 0;
    uint32_t bar2 = 0;
    uint32_t bar3 = 0;
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
} device_descriptor_t;

typedef struct driver_identifier {
    uint16_t vendor_id = 0;
    uint16_t device_id = 0;
    uint16_t class_id = 0;
    uint16_t subclass_id = 0;
    uint16_t interface_id = 0;
} driver_identifier_t;

class PCI {
    Port32Bit data_port;
    Port32Bit command_port;
    device_descriptor_t dev;
    base_address_register_t bar;

public:
    PCI();
    ~PCI();

    uint32_t read(device_descriptor_t device, uint32_t register_offset);
    void write(device_descriptor_t device, uint32_t register_offset, uint32_t value);
    uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset);
    void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value);
    bool device_has_functions(uint16_t bus, uint16_t device);

    bool find_driver(driver_identifier_t identifier);
    bool find_driver(driver_identifier_t identifier, uint16_t bus, uint16_t device);
    void enable_busmaster(device_descriptor_t device);
    void enable_memory_space(device_descriptor_t device);

    void get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
    void get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
    device_descriptor_t get_descriptor();

    static PCI* active;
};

#endif
