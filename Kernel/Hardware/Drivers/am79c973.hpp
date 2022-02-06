#ifndef AM79C973_HPP
#define AM79C973_HPP

#include "../interrupts.hpp"
#include "../pci.hpp"
#include "../port.hpp"

#define RX_BUF_SIZE 8192
#define TX_BUF_SIZE 1518

typedef struct init_block {
    uint16_t mode;
    unsigned reserved1 : 4;
    unsigned send_buffers : 4;
    unsigned reserved2 : 4;
    unsigned receive_buffers : 4;
    uint64_t physical_address : 48;
    uint16_t reserved3;
    uint64_t logical_address;
    uint32_t receive_descriptor_address;
    uint32_t send_descriptor_address;
} __attribute__((packed)) init_block_t;

typedef struct buffer_description {
    uint32_t address;
    uint32_t flags;
    uint32_t flags2;
    uint32_t avail;
} __attribute__((packed)) buffer_description_t;

class AM79C973 : public InterruptHandler {
private:
    bool is_activated = false;

    Port16Bit mac0_address_port;
    Port16Bit mac2_address_port;
    Port16Bit mac4_address_port;
    Port16Bit register_data_port;
    Port16Bit register_address_port;
    Port16Bit reset_port;
    Port16Bit bus_control_register_data_port;

    init_block_t init_block;

    buffer_description_t* send_buffer_descriptions;
    uint8_t send_buffer_descriptions_memory[2048 + 15];
    uint8_t send_buffers[2 * 1024 + 15][8];
    uint8_t send_buffer_index = 0;

    buffer_description_t* receive_buffer_descriptions;
    uint8_t receive_buffer_descriptions_memory[2048 + 15];
    uint8_t receive_buffers[2 * 1024 + 15][8];
    uint8_t receive_buffer_index = 0;
    uint64_t mac_address = 0;

public:
    AM79C973(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~AM79C973();

    static driver_identifier_t identifier() { return { 0x1022, 0x2000 }; }
    void send(uint8_t* buffer, uint32_t size);
    void receive();

    void activate();
    virtual uint32_t interrupt(uint32_t esp);
};

#endif
