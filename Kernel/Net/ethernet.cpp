#include "ethernet.hpp"
#include "arp.hpp"
#include "ipv4.hpp"

Ethernet* Ethernet::active = 0;
Ethernet::Ethernet()
{
    active = this;
}

Ethernet::~Ethernet()
{
}

void Ethernet::set_network_driver(NetworkDriver* driver)
{
    network_driver = driver;
    has_driver = true;
}

uint64_t Ethernet::get_mac_address()
{
    if (!has_driver)
        return 0;
    return network_driver->get_mac_address();
}

bool Ethernet::handle_packet(uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(ethernet_frame_t))
        return false;

    /* TODO: Validate the checksum (last 4 bytes)? */
    if (size > 64)
        size -= 4;

    ethernet_frame_t* frame = (ethernet_frame_t*)buffer;
    uint8_t* data = (uint8_t*)frame + sizeof(ethernet_frame_t);
    int data_size = size - sizeof(ethernet_frame_t);
    bool send_back = false;

    /* This packet does not concern us */
    if ((frame->destination_mac != BROADCAST_MAC) && (frame->destination_mac != get_mac_address()))
        return false;

    if (frame->type == FLIP(ETHERNET_TYPE_ARP))
        send_back = ARP::handle_packet((arp_packet_t*)data, data_size);

    if (frame->type == FLIP(ETHERNET_TYPE_IP))
        send_back = IPV4::handle_packet((ipv4_packet_t*)data, data_size);

    return true;
}

bool Ethernet::send_packet(uint64_t mac, uint8_t* buffer, uint32_t size, uint16_t type)
{
    if (!has_driver)
        return false;

    uint8_t* packet = (uint8_t*)kmalloc(sizeof(ethernet_frame_t) + size);
    ethernet_frame_t* frame = (ethernet_frame_t*)packet;

    frame->destination_mac = mac;
    frame->source_mac = network_driver->get_mac_address();
    frame->type = FLIP(type);

    memcpy(packet + sizeof(ethernet_frame_t), buffer, size);
    network_driver->send(packet, sizeof(ethernet_frame_t) + size);

    kfree(packet);
    return true;
}
