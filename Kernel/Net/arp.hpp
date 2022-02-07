#ifndef ARP_HPP
#define ARP_HPP

#include <LibC/types.hpp>

typedef struct arp_packet {
    uint16_t hardware_type;
    uint16_t protocol;
    uint8_t hardware_address_size;
    uint8_t protocol_address_size;
    uint16_t command;

    uint64_t source_mac : 48;
    uint32_t source_ip;
    uint64_t destination_mac : 48;
    uint32_t destination_ip;
} __attribute__((packed)) arp_packet_t;

typedef struct arp_cache {
    uint32_t mac = 0;
    uint32_t ip = 0;
} arp_cache_t;

namespace ARP {
bool handle_packet(arp_packet_t* arp, uint32_t size);
void request_mac_address(uint32_t ip);
uint64_t mac_address_from_cache(uint32_t ip);
uint64_t resolve(uint32_t ip);
void broadcast_mac_address(uint32_t ip);
}

#endif
