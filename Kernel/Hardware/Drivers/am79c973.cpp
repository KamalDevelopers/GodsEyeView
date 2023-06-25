#include "am79c973.hpp"

AM79C973::AM79C973(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
    , mac0_address_port(device.port_base)
    , mac2_address_port(device.port_base + 0x02)
    , mac4_address_port(device.port_base + 0x04)
    , register_data_port(device.port_base + 0x10)
    , register_address_port(device.port_base + 0x12)
    , reset_port(device.port_base + 0x14)
    , bus_control_register_data_port(device.port_base + 0x16)
{
    PCI::active->enable_busmaster(device);
    activate();
}

AM79C973::~AM79C973()
{
}

void AM79C973::activate()
{
    uint64_t mac0 = mac0_address_port.read() % 256;
    uint64_t mac1 = mac0_address_port.read() / 256;
    uint64_t mac2 = mac2_address_port.read() % 256;
    uint64_t mac3 = mac2_address_port.read() / 256;
    uint64_t mac4 = mac4_address_port.read() % 256;
    uint64_t mac5 = mac4_address_port.read() / 256;
    mac_address = mac5 << 40 | mac4 << 32 | mac3 << 24 | mac2 << 16 | mac1 << 8 | mac0;

    register_address_port.write(20);
    bus_control_register_data_port.write(0x102);

    register_address_port.write(0);
    register_data_port.write(0x04);

    init_block.mode = 0x0000;
    init_block.reserved1 = 0;
    init_block.send_buffers = 3;
    init_block.reserved2 = 0;
    init_block.receive_buffers = 3;
    init_block.physical_address = mac_address;
    init_block.reserved3 = 0;
    init_block.logical_address = 0;

    memset(send_buffer_descriptions_memory, 0, 2048 + 15);
    memset(receive_buffer_descriptions_memory, 0, 2048 + 15);

    send_buffer_descriptions = (buffer_description_t*)((((uint32_t)&send_buffer_descriptions_memory[0]) + 15) & ~((uint32_t)0xF));
    init_block.send_descriptor_address = (uint32_t)send_buffer_descriptions;
    receive_buffer_descriptions = (buffer_description_t*)((((uint32_t)&receive_buffer_descriptions_memory[0]) + 15) & ~((uint32_t)0xF));
    init_block.receive_descriptor_address = (uint32_t)receive_buffer_descriptions;

    for (uint8_t i = 0; i < 8; i++) {
        send_buffer_descriptions[i].address = (((uint32_t)&send_buffers[i]) + 15) & ~(uint32_t)0xF;
        send_buffer_descriptions[i].flags = 0x7FF
            | 0xF000;
        send_buffer_descriptions[i].flags2 = 0;
        send_buffer_descriptions[i].avail = 0;

        receive_buffer_descriptions[i].address = (((uint32_t)&receive_buffers[i]) + 15) & ~(uint32_t)0xF;
        receive_buffer_descriptions[i].flags = 0xF7FF
            | 0x80000000;
        receive_buffer_descriptions[i].flags2 = 0;
        send_buffer_descriptions[i].avail = 0;
    }

    register_address_port.write(1);
    register_data_port.write((uint32_t)(&init_block) & 0xFFFF);
    register_address_port.write(2);
    register_data_port.write(((uint32_t)(&init_block) >> 16) & 0xFFFF);

    /* Activate */
    register_address_port.write(0);
    register_data_port.write(0x41);

    register_address_port.write(4);
    uint32_t register_data_port_value = register_data_port.read();
    register_address_port.write(4);
    register_data_port.write(register_data_port_value | 0xC00);

    register_address_port.write(0);
    register_data_port.write(0x42);
}

void AM79C973::send(uint8_t* buffer, uint32_t size)
{
    uint8_t send_descriptor = send_buffer_index;
    send_buffer_index = (send_buffer_index + 1) % 8;
    size = (size > TX_BUF_SIZE) ? TX_BUF_SIZE : size;

    for (uint8_t *src = buffer + size - 1,
                 *dst = (uint8_t*)(send_buffer_descriptions[send_descriptor].address + size - 1);
         src >= buffer; src--, dst--)
        *dst = *src;

    send_buffer_descriptions[send_descriptor].flags2 = 0;
    send_buffer_descriptions[send_descriptor].flags = 0x8300F000 | ((uint16_t)((-size) & 0xFFF));

    register_address_port.write(0);
    register_data_port.write(0x48);
}

void AM79C973::receive()
{
    for (; (receive_buffer_descriptions[receive_buffer_index].flags & 0x80000000) == 0; receive_buffer_index = (receive_buffer_index + 1) % 8) {
        if (!(receive_buffer_descriptions[receive_buffer_index].flags & 0x40000000) && (receive_buffer_descriptions[receive_buffer_index].flags & 0x03000000) == 0x03000000) {
            uint32_t size = receive_buffer_descriptions[receive_buffer_index].flags & 0xFFF;
            if (size > 64)
                size -= 4;

            uint8_t* buffer = (uint8_t*)(receive_buffer_descriptions[receive_buffer_index].address);
            uint8_t* persistent_buffer = (uint8_t*)kmalloc(size);
            memcpy(persistent_buffer, buffer, size);
            ETH->handle_packet(persistent_buffer, size);
            kfree(persistent_buffer);
        }

        receive_buffer_descriptions[receive_buffer_index].flags2 = 0;
        receive_buffer_descriptions[receive_buffer_index].flags = 0x8000F7FF;
    }
}

uint32_t AM79C973::interrupt(uint32_t esp)
{
    register_address_port.write(0);
    uint32_t register_data_port_value = register_data_port.read();

    if (register_data_port_value & 0x8000)
        klog("[am79c973] error");
    if (register_data_port_value & 0x2000)
        klog("[am79c973] collision error");
    if (register_data_port_value & 0x1000)
        klog("[am79c973] missed frame");
    if (register_data_port_value & 0x0800)
        klog("[am79c973] memory error");
    if (register_data_port_value & 0x0400)
        receive();
    /*
    if (register_data_port_value & 0x0200)
        klog("[am79c973] packet sent");
    if (register_data_port_value & 0x0100)
        klog("[am79c973] init done");
    */

    register_address_port.write(0);
    register_data_port.write(register_data_port_value);

    return esp;
}
