#ifndef PCI_HPP
#define PCI_HPP

#include "../Mem/mm.hpp"
#include "interrupts.hpp"
#include "port.hpp"
#include <LibC/stdlib.hpp>
#include <LibC/types.hpp>

typedef struct base_address_register {
    bool prefetchable;
    uint8_t* address;
    uint32_t size;
    uint8_t type;
} base_address_register_t;

typedef struct device_descriptor {
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
} device_descriptor_t;

typedef struct driver_identifier {
    uint16_t vendor_id = 0;
    uint16_t device_id = 0;
    uint16_t class_id = 0;
    uint16_t subclass_id = 0;
} driver_identifier_t;

class Driver {
public:
    Driver() {};
    ~Driver() {};

    virtual driver_identifier_t identify() { return {}; };
    virtual void activate() {};
};

class PCI {
    Port32Bit dataport;
    Port32Bit commandport;
    device_descriptor_t dev;

public:
    PCI();
    ~PCI();

    uint32_t read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
    void write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);
    bool device_has_functions(uint16_t bus, uint16_t device);

    void select_driver(Driver* drivers[], size_t size, int bus, int device);
    void select_drivers(Driver* drivers[], size_t size);

    device_descriptor_t get_device_descriptor(uint16_t bus, uint16_t device, uint16_t function);
    device_descriptor_t* get_descriptor();
    base_address_register_t get_base_address_register(uint16_t bus, uint16_t device, uint16_t function, uint16_t bar);
};

#endif
