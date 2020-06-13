#ifndef PCI_HPP
#define PCI_HPP

#include "port.hpp"
#include "types.hpp"
#include "interrupts.hpp"

    class PCIcontroller 
    {
        Port32Bit dataPort;
        Port32Bit commandPort;

    public:
        PCIcontroller();
        ~PCIcontroller();

        uint32_t Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset);
        void Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registerOfsset, uint32_t value);
        bool DeviceHasFunctions(uint16_t bus, uint16_t device);
    };

#endif