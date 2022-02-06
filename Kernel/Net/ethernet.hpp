#ifndef ETHERNET_HPP
#define ETHERNET_HPP

#include "../Mem/mm.hpp"
#include <LibC/types.hpp>

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

    void set_network_driver(NetworkDriver* driver);
    bool handle_packet(uint8_t* buffer, uint32_t size);
    bool send_packet(uint64_t mac, uint16_t type, uint8_t* buffer, uint32_t size);
};

#endif
