#include "ipv4.hpp"
#include "arp.hpp"
#include "ethernet.hpp"
#include "icmp.hpp"

uint16_t IPV4::calculate_checksum(uint16_t* data, uint32_t size)
{
    uint32_t checksum = 0;

    for (uint32_t i = 0; i < size / 2; i++)
        checksum += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);

    if (size % 2)
        checksum += ((uint16_t)((char*)data)[size - 1]) << 8;

    while (checksum & 0xFFFF0000)
        checksum = (checksum & 0xFFFF) + (checksum >> 16);

    return ((~checksum & 0xFF00) >> 8) | ((~checksum & 0x00FF) << 8);
}

bool IPV4::handle_packet(ipv4_packet_t* ipv4, uint32_t size)
{
    if (size < sizeof(ipv4_packet_t))
        return false;

    if (ipv4->destination_ip == IP) {
        int length = ipv4->length;

        if (length > size)
            length = size;

        uint8_t* data = (uint8_t*)ipv4;
        if (ipv4->protocol) {
            if (ipv4->protocol == 0x1)
                ICMP::receive_ping((icmp_packet_t*)(data + 4 * ipv4->header_length), ipv4->source_ip);
        }
    }

    return true;
}

void IPV4::send_packet(uint32_t destination_ip, uint8_t protocol, uint8_t* buffer, uint32_t size)
{
    uint8_t* packet = (uint8_t*)kmalloc(sizeof(ipv4_packet_t) + size);
    memset(packet, 0, sizeof(ipv4_packet_t) + size);
    ipv4_packet_t* header = (ipv4_packet_t*)packet;

    header->version = 4;
    header->header_length = sizeof(ipv4_packet_t) / 4;
    header->tos = 0;
    header->length = size + sizeof(ipv4_packet_t);
    header->length = ((header->length & 0xFF00) >> 8) | ((header->length & 0x00FF) << 8);
    header->ident = 0x0100;
    header->flags = 0x0040;
    header->ttl = 0x40;
    header->protocol = protocol;

    header->destination_ip = destination_ip;
    header->source_ip = IP;

    header->checksum = 0;
    header->checksum = calculate_checksum((uint16_t*)header);

    uint8_t* data = packet + sizeof(ipv4_packet_t);
    memcpy(data, buffer, size);

    uint32_t route = destination_ip;
    if ((destination_ip & SUBNET) != (header->source_ip & SUBNET))
        route = GATEWAY;

    ETH->send_packet(ARP::resolve(route), packet, sizeof(ipv4_packet_t) + size, ETHERNET_TYPE_IP);
    kfree(buffer);
}
