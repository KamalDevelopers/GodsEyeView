#ifndef amd79_HPP
#define amd79_HPP

#include "../interrupts.hpp"
#include "../pci.hpp"
#include "../port.hpp"
#include "driver.hpp"
#include <LibC/stdio.hpp>
#include <LibC/types.hpp>

class AmdDriver;
class RawDataHandler {
protected:
    AmdDriver* backend;

public:
    RawDataHandler(AmdDriver* backend);
    ~RawDataHandler();

    virtual bool on_raw_data_received(uint8_t* buffer, uint32_t size);
    void send(uint8_t* buffer, uint32_t size);
};

class AmdDriver : public Driver
    , public InterruptHandler {
    struct initialization_block {
        uint16_t mode;
        unsigned reserved1 : 4;
        unsigned numSendBuffers : 4;
        unsigned reserved2 : 4;
        unsigned numRecvBuffers : 4;
        uint64_t physicalAddress : 48;
        uint16_t reserved3;
        uint64_t logicalAddress;
        uint32_t recv_buffer_descr_address;
        uint32_t send_buffer_descr_address;
    } __attribute__((packed));

    struct buffer_descriptor {
        uint32_t address;
        uint32_t flags;
        uint32_t flags2;
        uint32_t avail;
    } __attribute__((packed));

    Port16Bit MACAddress0Port;
    Port16Bit MACAddress2Port;
    Port16Bit MACAddress4Port;
    Port16Bit registerDataPort;
    Port16Bit registerAddressPort;
    Port16Bit resetPort;
    Port16Bit busControlRegisterDataPort;

    initialization_block initBlock;

    buffer_descriptor* sendBufferDescr;
    uint8_t sendBufferDescrMemory[2048 + 15];
    uint8_t sendBuffers[2 * 1024 + 15][8];
    uint8_t currentSendBuffer;

    buffer_descriptor* recvBufferDescr;
    uint8_t recvBufferDescrMemory[2048 + 15];
    uint8_t recvBuffers[2 * 1024 + 15][8];
    uint8_t currentRecvBuffer;
    RawDataHandler* handler;

public:
    AmdDriver(DeviceDescriptor* dev, InterruptManager* interrupts);
    ~AmdDriver();

    void activate();
    int reset();
    uint32_t interrupt(uint32_t esp);
    void send(uint8_t* buffer, int count);
    void receive();
    void set_handler(RawDataHandler* handler);
    uint64_t get_mac_address();
    void set_ip_address(uint32_t);
    uint32_t get_ip_address();
};

#endif
