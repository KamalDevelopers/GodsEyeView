#include "rtl8139.hpp"
#include "../../Mem/paging.hpp"

#define ROUND(a, b) (((a) + (b)-1) & ~((b)-1))
uint8_t tsad_ports[4] = { 0x20, 0x24, 0x28, 0x2C };
uint8_t tsd_ports[4] = { 0x10, 0x14, 0x18, 0x1C };

RTL8139::RTL8139(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
    , mac0_address_port(device.bar0)
    , mac2_address_port(device.bar0 + 0x02)
    , mac4_address_port(device.bar0 + 0x04)
    , rbstart_port(device.bar0 + 0x30)
    , command_port(device.bar0 + 0x37)
    , capr_port(device.bar0 + 0x38)
    , interrupt_mask_port(device.bar0 + 0x3C)
    , interrupt_status_port(device.bar0 + 0x3E)
    , config0_port(device.bar0 + 0x51)
    , config1_port(device.bar0 + 0x52)
    , tx_config_port(device.bar0 + 0x40)
    , rx_config_port(device.bar0 + 0x44)
    , bar0 { device.bar0 }
{
    packet_index = 0;
    PCI::active->enable_busmaster(device);
    activate();
}

RTL8139::~RTL8139()
{
}

void RTL8139::activate()
{
    config1_port.write(0x0);

    /* Reset */
    command_port.write(0x10);
    while ((command_port.read() & 0x10) != 0)
        ;

    memset(receive_buffer, 0x0, 8192 + 16 + 1500);
    rbstart_port.write((uint32_t)receive_buffer);

    interrupt_mask_port.write(0x0005);
    rx_config_port.write(0xF | (1 << 7));

    uint64_t mac0 = mac0_address_port.read() % 256;
    uint64_t mac1 = mac0_address_port.read() / 256;
    uint64_t mac2 = mac2_address_port.read() % 256;
    uint64_t mac3 = mac2_address_port.read() / 256;
    uint64_t mac4 = mac4_address_port.read() % 256;
    uint64_t mac5 = mac4_address_port.read() / 256;
    mac_address = mac5 << 40 | mac4 << 32 | mac3 << 24 | mac2 << 16 | mac1 << 8 | mac0;
    command_port.write(0x0C);
}

void RTL8139::send(uint8_t* buffer, uint32_t size)
{
    size = (size > TX_BUF_SIZE) ? TX_BUF_SIZE : size;
    memcpy(transfer_buffer[tx_index], buffer, size);
    outbl(bar0 + tsad_ports[tx_index], (uint32_t)transfer_buffer[tx_index]);
    outbl(bar0 + tsd_ports[tx_index++], size);

    if (tx_index > 3)
        tx_index = 0;
}

void RTL8139::receive()
{
    uint32_t status = interrupt_status_port.read();
    interrupt_status_port.write(status & ~(RXFIFOOVER | RXOVERFLOW | ROK));

    uint16_t* data = (uint16_t*)(receive_buffer + packet_index);
    uint16_t packet_length = *(data + 1) - 4;

    data = data + 2;

    void* packet = kmalloc(packet_length);
    memcpy(packet, data, packet_length);

    ETH->handle_packet((uint8_t*)packet, packet_length);

    // packet_index = ROUND(packet_index + packet_length + 4, 4);
    packet_index = (packet_index + packet_length + 4 + 4 + 3) & ~3;
    capr_port.write(packet_index - 0x10);
    interrupt_status_port.write(status & ~(RXFIFOOVER | RXOVERFLOW | ROK));
    kfree(packet);
}

uint32_t RTL8139::interrupt(uint32_t esp)
{
    uint16_t status = interrupt_status_port.read();
    interrupt_status_port.write(0x05);

    if (status & ROK) {
#if RTL_DEBUG
        klog("[RTL8139] packet receive");
#endif
        receive();
    }

#if RTL_DEBUG
    if (status & TER)
        klog("[RTL8139] packet sent error");
    if (status & RER)
        klog("[RTL8139] packet received error");
    if (status & TOK)
        klog("[RTL8139] packet sent");
#endif

    return esp;
}
