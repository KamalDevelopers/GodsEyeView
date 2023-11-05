#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP

#include "../GDT/gdt.hpp"
#include "../multitasking.hpp"
#include "../tty.hpp"
#include "port.hpp"
#include <LibC/stdio.h>
#include <LibC/types.h>

#define PIT_HZ 2600

class InterruptManager;

class InterruptHandler {
protected:
    uint8_t interrupt_number;
    uint8_t line;
    InterruptManager* interrupt_manager;
    InterruptHandler(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~InterruptHandler();

public:
    virtual uint32_t interrupt(uint32_t esp);
};

class InterruptManager {
    friend class InterruptHandler;

protected:
    InterruptHandler* handlers[5][256];
    TaskManager* task_manager;

    struct gate_descriptor {
        uint16_t handler_address_low_bits;
        uint16_t gdt_code_segment_selector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handler_address_high_bits;
    } __attribute__((packed));

    static gate_descriptor interrupt_descriptor_table[256];

    struct interrupt_descriptor_table_pointer {
        uint16_t size;
        uint32_t base;
    } __attribute__((packed));

    uint16_t hardware_interrupt_offset;
    static void set_interrupt_descriptor_table_entry(uint8_t interrupt,
        uint16_t code_segment_selector_offset, void (*handler)(),
        uint8_t descriptor_privilege_level, uint8_t descriptor_type);

    static void interrupt_ignore();

    static void request0x00();
    static void request0x01();
    static void request0x02();
    static void request0x03();
    static void request0x04();
    static void request0x05();
    static void request0x06();
    static void request0x07();
    static void request0x08();
    static void request0x09();
    static void request0x0A();
    static void request0x0B();
    static void request0x0C();
    static void request0x0D();
    static void request0x0E();
    static void request0x0F();
    static void request0x31();
    static void request0x80();

    static void exception0x00();
    static void exception0x01();
    static void exception0x02();
    static void exception0x03();
    static void exception0x04();
    static void exception0x05();
    static void exception0x06();
    static void exception0x07();
    static void exception0x08();
    static void exception0x09();
    static void exception0x0A();
    static void exception0x0B();
    static void exception0x0C();
    static void exception0x0D();
    static void exception0x0E();
    static void exception0x0F();
    static void exception0x10();
    static void exception0x11();
    static void exception0x12();
    static void exception0x13();

    static uint32_t interrupt(uint8_t interrupt, uint32_t esp);
    uint32_t handle_interrupt(uint8_t interrupt, uint32_t esp);

    Port8BitSlow master_command_port;
    Port8BitSlow master_data_port;
    Port8BitSlow slave_command_port;
    Port8BitSlow slave_data_port;

public:
    InterruptManager(uint16_t hardware_interrupt_offset, TaskManager* task_manager);
    ~InterruptManager();

    static InterruptManager* active_handler;
    static InterruptManager* active;
    uint16_t get_hardware_interrupt_offset() { return hardware_interrupt_offset; }
    void activate();
    void deactivate();
};

namespace IRQ {
static void activate()
{
    if (InterruptManager::active != 0)
        InterruptManager::active->activate();
}

static void deactivate()
{
    if (InterruptManager::active != 0)
        InterruptManager::active->deactivate();
}
};

#endif
