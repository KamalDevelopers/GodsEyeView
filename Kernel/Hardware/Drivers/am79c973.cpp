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
    activate();
}

AM79C973::~AM79C973()
{
}

void AM79C973::activate()
{
    is_activated = true;
    uint64_t mac0 = mac0_address_port.read() % 256;
    uint64_t mac1 = mac0_address_port.read() / 256;
    uint64_t mac2 = mac2_address_port.read() % 256;
    uint64_t mac3 = mac2_address_port.read() / 256;
    uint64_t mac4 = mac4_address_port.read() % 256;
    uint64_t mac5 = mac4_address_port.read() / 256;
    uint64_t mac = mac5 << 40 | mac4 << 32 | mac3 << 24 | mac2 << 16 | mac1 << 8 | mac0;

    register_address_port.write(20);
    bus_control_register_data_port.write(0x102);

    register_address_port.write(0);
    register_data_port.write(0x04);

    init_block.mode = 0x0000;
    init_block.reserved1 = 0;
    init_block.send_buffers = 3;
    init_block.reserved2 = 0;
    init_block.receive_buffers = 3;
    init_block.physical_address = mac;
    init_block.reserved3 = 0;
    init_block.logical_address = 0;

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
}

void AM79C973::receive()
{
}

uint32_t AM79C973::interrupt(uint32_t esp)
{
    register_address_port.write(0);
    uint32_t register_data_port_value = register_data_port.read();

    /* TODO: handle interrupt */

    register_address_port.write(0);
    register_data_port.write(register_data_port_value);

    return esp;
}
