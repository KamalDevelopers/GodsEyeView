#ifndef ETHERNET_HPP
#define ETHERNET_HPP

#include "../Mem/mm.hpp"
#include <LibC/types.hpp>

#define FLIP(x) (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8))
#define IP ((14 << 24) | (2 << 16) | (0 << 8) | 10)
#define SUBNET ((0 << 24) | (255 << 16) | (255 << 8) | 255)
#define GATEWAY ((2 << 24) | (2 << 16) | (0 << 8) | 10)
#define BROADCAST_MAC 0xFFFFFFFFFFFF

#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_IP 0x0800
#define ETH Ethernet::active

typedef struct ethernet_frame {
    uint64_t destination_mac : 48;
    uint64_t source_mac : 48;
    uint16_t type;
} __attribute__((packed)) ethernet_frame_t;

class NetworkDriver {
public:
    NetworkDriver() { }
    ~NetworkDriver() { }

    virtual uint64_t get_mac_address() { return 0; }
    virtual void send(uint8_t* buffer, uint32_t size) { }
    virtual void receive() { }
};

class Ethernet {
private:
    NetworkDriver* network_driver;
    bool has_driver = false;

public:
    Ethernet();
    ~Ethernet();

    static Ethernet* active;
    void set_network_driver(NetworkDriver* driver);
    bool handle_packet(uint8_t* buffer, uint32_t size);
    bool send_packet(uint64_t mac, uint8_t* buffer, uint32_t size, uint16_t type);
    uint64_t get_mac_address();
};

#endif
