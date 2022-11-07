#include "arp.hpp"
#include "dhcp.hpp"
#include "ethernet.hpp"

uint32_t cache_entry_index = 0;
arp_cache_t cache[128];

bool ARP::handle_packet(arp_packet_t* arp, uint32_t size)
{
    if (size < sizeof(arp_packet_t))
        return false;

    if ((arp->hardware_type != 0x0100) || (arp->protocol != 0x0008))
        return false;

    if ((arp->hardware_address_size != 6) || (arp->protocol_address_size != 4) || (arp->destination_ip != DHCP::ip()))
        return false;

    if (arp->command == 0x0100) {
        arp->command = 0x0200;
        arp->destination_ip = arp->source_ip;
        arp->destination_mac = arp->source_mac;
        arp->source_ip = DHCP::ip();
        arp->source_mac = ETH->get_mac_address();
        return true;
    }

    if ((arp->command == 0x0200) && (cache_entry_index < 128)) {
        cache[cache_entry_index].ip = arp->source_ip;
        cache[cache_entry_index].mac = arp->source_mac;
        cache_entry_index++;
    }

    return false;
}

void ARP::broadcast_mac_address(uint32_t ip)
{
    arp_packet_t arp;
    arp.hardware_type = 0x0100;
    arp.protocol = 0x0008;
    arp.hardware_address_size = 6;
    arp.protocol_address_size = 4;
    arp.command = 0x0200;

    arp.source_mac = ETH->get_mac_address();
    arp.source_ip = DHCP::ip();
    arp.destination_mac = resolve(ip);
    arp.destination_ip = ip;

    ETH->send_packet(arp.destination_mac, (uint8_t*)&arp, sizeof(arp_packet_t), ETHERNET_TYPE_ARP);
}

void ARP::request_mac_address(uint32_t ip)
{
    arp_packet_t arp;
    arp.hardware_type = 0x0100;
    arp.protocol = 0x0008;
    arp.hardware_address_size = 6;
    arp.protocol_address_size = 4;
    arp.command = 0x0100;

    arp.source_mac = ETH->get_mac_address();
    arp.source_ip = DHCP::ip();
    arp.destination_mac = BROADCAST_MAC;
    arp.destination_ip = ip;

    ETH->send_packet(arp.destination_mac, (uint8_t*)&arp, sizeof(arp_packet_t), ETHERNET_TYPE_ARP);
}

uint64_t ARP::mac_address_from_cache(uint32_t ip)
{
    for (int i = 0; i < cache_entry_index; i++)
        if (cache[i].ip == ip)
            return cache[i].mac;
    return BROADCAST_MAC;
}

uint64_t ARP::resolve(uint32_t ip)
{
    uint64_t result = mac_address_from_cache(ip);

    if (result == BROADCAST_MAC)
        request_mac_address(ip);

    /* FIXME: Prevent infinite loop */
    while (result == BROADCAST_MAC)
        result = mac_address_from_cache(ip);

    return result;
}
