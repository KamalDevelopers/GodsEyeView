#ifndef PCI_HPP
#define PCI_HPP

#include "interrupts.hpp"
#include "port.hpp"
#include "types.hpp"
#include "Drivers/driver.hpp"

class PCIcontrollerDeviceDescriptor {
public:
    uint32_t portBase;
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

    PCIcontrollerDeviceDescriptor();
    ~PCIcontrollerDeviceDescriptor();
};

class PCIcontroller {
    Port32Bit dataPort;
    Port32Bit commandPort;
    PCIcontrollerDeviceDescriptor dev;

public:
    PCIcontroller();
    ~PCIcontroller();

    uint32_t Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
    void Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value);
    bool DeviceHasFunctions(uint16_t bus, uint16_t device);

    void SelectDrivers(DriverManager* driverManager);
    PCIcontrollerDeviceDescriptor GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function);
    PCIcontrollerDeviceDescriptor* GetDescriptor();
};

#endif