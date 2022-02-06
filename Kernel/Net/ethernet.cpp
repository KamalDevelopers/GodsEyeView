#include "ethernet.hpp"

Ethernet::Ethernet()
{
}

Ethernet::~Ethernet()
{
}

void Ethernet::set_network_driver(NetworkDriver* driver)
{
    network_driver = driver;
    has_driver = true;
}

bool Ethernet::handle_packet(uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(ethernet_frame_t))
        return false;

    ethernet_frame_t* frame = (ethernet_frame_t*)buffer;

    /* TODO: handle packet */

    return true;
}

bool Ethernet::send_packet(uint64_t mac, uint16_t type, uint8_t* buffer, uint32_t size)
{
    if (!has_driver)
        return false;

    uint8_t* packet = (uint8_t*)kmalloc(sizeof(ethernet_frame_t) + size);
    ethernet_frame_t* frame = (ethernet_frame_t*)packet;

    frame->destination_mac = mac;
    frame->source_mac = network_driver->get_mac_address();
    frame->type = type;

    memcpy(packet + sizeof(ethernet_frame_t), buffer, size);
    network_driver->send(packet, sizeof(ethernet_frame_t) + size);

    kfree(packet);
    return true;
}
