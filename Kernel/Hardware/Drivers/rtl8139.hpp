#ifndef RTL8139_HPP
#define RTL8139_HPP

#include "../pci.hpp"

#define RX_BUF_SIZE 8192
#define TX_BUF_SIZE 1518

#define RX_READ_POINTER_MASK (~3)
#define ROK (1 << 0)
#define RER (1 << 1)
#define TOK (1 << 2)
#define TER (1 << 3)
#define TX_TOK (1 << 15)

typedef struct description {
    uint32_t physical_address;
    uint32_t packet_size;
} description_t;

class RTL8139 : public InterruptHandler {
private:
    bool is_activated = false;
    Port16Bit mac0_address_port;
    Port16Bit mac2_address_port;
    Port16Bit mac4_address_port;

    Port32Bit rbstart_port;
    Port8Bit command_port;
    Port16Bit interrupt_mask_port;
    Port16Bit interrupt_status_port;
    Port8Bit capr_port;

    Port8Bit config0_port;
    Port8Bit config1_port;

    Port32Bit tx_config_port;
    Port32Bit rx_config_port;

    char receive_buffer[8192 + 16 + 1500];
    uint64_t mac_address = 0;
    uint32_t port_base = 0;
    uint32_t tx_index = 0;
    uint32_t packet_index = 0;
    uint8_t transfer_buffer[4][TX_BUF_SIZE];

public:
    RTL8139(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~RTL8139();

    static driver_identifier_t identifier() { return { 0x10EC, 0x8139 }; }
    void send(uint8_t* buffer, uint32_t size);
    void receive();

    void activate();
    virtual uint32_t interrupt(uint32_t esp);
};

#endif
