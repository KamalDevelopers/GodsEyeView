#include "interrupts.hpp"
#include "../panic.hpp"

InterruptHandler::InterruptHandler(InterruptManager* interrupt_manager, uint8_t interrupt_number)
{
    this->interrupt_number = interrupt_number;
    this->interrupt_manager = interrupt_manager;
    interrupt_manager->handlers[interrupt_number] = this;
}

InterruptHandler::~InterruptHandler()
{
    if (interrupt_manager->handlers[interrupt_number] == this)
        interrupt_manager->handlers[interrupt_number] = 0;
}

uint32_t InterruptHandler::interrupt(uint32_t esp)
{
    return esp;
}

InterruptManager::gate_descriptor InterruptManager::interrupt_descriptor_table[256];
InterruptManager* InterruptManager::active_handler = 0;
InterruptManager* InterruptManager::active = 0;

void InterruptManager::set_interrupt_descriptor_table_entry(uint8_t interrupt,
    uint16_t code_segment, void (*handler)(), uint8_t descriptor_privilege_level, uint8_t descriptor_type)
{
    interrupt_descriptor_table[interrupt].handler_address_low_bits = ((uint32_t)handler) & 0xFFFF;
    interrupt_descriptor_table[interrupt].handler_address_high_bits = (((uint32_t)handler) >> 16) & 0xFFFF;
    interrupt_descriptor_table[interrupt].gdt_code_segment_selector = code_segment;

    const uint8_t IDT_DESC_PRESENT = 0x80;
    interrupt_descriptor_table[interrupt].access = IDT_DESC_PRESENT | ((descriptor_privilege_level & 3) << 5) | descriptor_type;
    interrupt_descriptor_table[interrupt].reserved = 0;
}

InterruptManager::InterruptManager(uint16_t hardware_interrupt_offset, GDT* global_descriptor_table, TaskManager* task_manager)
    : master_command_port(0x20)
    , master_data_port(0x21)
    , slave_command_port(0xA0)
    , slave_data_port(0xA1)
{
    active = this;
    this->task_manager = task_manager;
    this->hardware_interrupt_offset = hardware_interrupt_offset;
    uint32_t code_segment = global_descriptor_table->get_code_segment_selector();

    const uint8_t IDT_INTERRUPT_GATE = 0xE;
    for (uint8_t i = 255; i > 0; --i) {
        set_interrupt_descriptor_table_entry(i, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);
        handlers[i] = 0;
    }
    set_interrupt_descriptor_table_entry(0, code_segment, &interrupt_ignore, 0, IDT_INTERRUPT_GATE);
    handlers[0] = 0;

    set_interrupt_descriptor_table_entry(0x00, code_segment, &exception0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x01, code_segment, &exception0x01, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x02, code_segment, &exception0x02, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x03, code_segment, &exception0x03, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x04, code_segment, &exception0x04, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x05, code_segment, &exception0x05, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x06, code_segment, &exception0x06, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x07, code_segment, &exception0x07, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x08, code_segment, &exception0x08, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x09, code_segment, &exception0x09, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0A, code_segment, &exception0x0A, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0B, code_segment, &exception0x0B, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0C, code_segment, &exception0x0C, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0D, code_segment, &exception0x0D, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0E, code_segment, &exception0x0E, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x0F, code_segment, &exception0x0F, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x10, code_segment, &exception0x10, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x11, code_segment, &exception0x11, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x12, code_segment, &exception0x12, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x13, code_segment, &exception0x13, 0, IDT_INTERRUPT_GATE);

    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x00, code_segment, &request0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x01, code_segment, &request0x01, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x02, code_segment, &request0x02, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x03, code_segment, &request0x03, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x04, code_segment, &request0x04, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x05, code_segment, &request0x05, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x06, code_segment, &request0x06, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x07, code_segment, &request0x07, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x08, code_segment, &request0x08, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x09, code_segment, &request0x09, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0A, code_segment, &request0x0A, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0B, code_segment, &request0x0B, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0C, code_segment, &request0x0C, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0D, code_segment, &request0x0D, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0E, code_segment, &request0x0E, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(hardware_interrupt_offset + 0x0F, code_segment, &request0x0F, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x80, code_segment, &request0x80, 0, IDT_INTERRUPT_GATE);

    master_command_port.write(0x11);
    slave_command_port.write(0x11);

    master_data_port.write(hardware_interrupt_offset);
    slave_data_port.write(hardware_interrupt_offset + 8);

    master_data_port.write(0x04);
    slave_data_port.write(0x02);

    master_data_port.write(0x01);
    slave_data_port.write(0x01);

    master_data_port.write(0x00);
    slave_data_port.write(0x00);

    interrupt_descriptor_table_pointer idt_pointer;
    idt_pointer.size = 256 * sizeof(gate_descriptor) - 1;
    idt_pointer.base = (uint32_t)interrupt_descriptor_table;
    asm volatile("lidt %0"
                 :
                 : "m"(idt_pointer));
}

InterruptManager::~InterruptManager()
{
    deactivate();
}

void InterruptManager::activate()
{
    if (active_handler != 0)
        active_handler->deactivate();

    active_handler = this;
    TM->yield();
    asm("sti");
}

void InterruptManager::deactivate()
{
    if (active_handler == this) {
        active_handler = 0;
        asm("cli");
    }
}

uint32_t InterruptManager::interrupt(uint8_t interrupt, uint32_t esp)
{
    if (active_handler != 0)
        return active_handler->handle_interrupt(interrupt, esp);
    return esp;
}

uint32_t InterruptManager::handle_interrupt(uint8_t interrupt, uint32_t esp)
{
    if (handlers[interrupt] != 0) {
        esp = handlers[interrupt]->interrupt(esp);
    } else if (interrupt != hardware_interrupt_offset) {
        if (interrupt == 0x0D) {
            if (TM->is_active()) {
                klog("General protection fault [0x%x]", ((cpu_state*)esp)->eip);
                TM->kill();
            } else {
                PANIC("General protection fault");
            }
        }

        if (interrupt == 0x0E) {
            uint32_t faulting_addr;
            asm volatile("mov %%cr2, %0"
                         : "=r"(faulting_addr));
            klog("\n{Faulty address 0x%x}\n", faulting_addr);
            klog("Page fault");
            if (TM->is_active()) {
                TM->kill();
            } else {
                PANIC("Page fault");
            }
        }
    }

    if (interrupt == hardware_interrupt_offset) {
        esp = (uint32_t)task_manager->schedule((cpu_state*)esp);
    }

    if (hardware_interrupt_offset <= interrupt && interrupt < hardware_interrupt_offset + 16) {
        master_command_port.write(0x20);
        if (hardware_interrupt_offset + 8 <= interrupt)
            slave_command_port.write(0x20);
    }

    return esp;
}
