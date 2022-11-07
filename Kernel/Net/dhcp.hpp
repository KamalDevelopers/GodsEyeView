#ifndef DHCP_HPP
#define DHCP_HPP

#include <LibC/network.h>
#include <LibC/types.h>

#define DHCP_REQUEST 1
#define DHCP_OFFER 2
#define DHCP_ACK 5

typedef struct dhcp_packet {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t server[64];
    uint8_t file[128];
    uint8_t options[64];
} __attribute__((packed)) dhcp_packet_t;

namespace DHCP {
void discover();
void request(uint32_t request_ip);
void handle_packet(dhcp_packet_t* packet);
void create_packet(dhcp_packet_t* packet, uint8_t type, uint32_t request_ip);
void parse_options(dhcp_packet_t* packet, uint8_t type, void* buff, uint16_t size);
void set_info(dhcp_packet_t* packet);

dhcp_info_t* info();
uint32_t ip();
uint32_t gateway();
uint32_t subnet();
}

#endif
