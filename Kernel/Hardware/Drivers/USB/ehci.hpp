#ifndef EHCI_HPP
#define EHCI_HPP

#include "../../../Locks/mutex.hpp"
#include "../../interrupts.hpp"
#include "../../pci.hpp"
#include <LibC/types.h>

#define MAX_PORT_SOURCES 16

struct ehci_capabilities {
    uint8_t length;
    uint8_t resv;
    uint16_t version;
    uint32_t hcs_params;
    uint32_t hcc_params;
    uint32_t port_route[2];
};

struct ehci_operations {
    uint32_t command;
    uint32_t status;
    uint32_t interrupt;
    uint32_t frame_index;
    uint32_t control_dss;
    uint32_t periodic_frame_list;
    uint32_t async_list;
    uint8_t resv[36];
    uint32_t configure_flag;
    uint32_t port_source[MAX_PORT_SOURCES];
} __attribute__((packed));

struct ehci_device_endpoint {
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t packet_size;
    uint8_t interval;
} __attribute__((packed));

struct ehci_device_descriptor_request {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed));

typedef struct {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) ehci_command;

typedef struct {
    uint32_t next_link;
    uint32_t alt_link;
    uint32_t token;
    uint32_t buffer[5];
    uint32_t ext_buffer[5];
} __attribute__((packed)) ehci_transfer_descriptor;

typedef struct {
    uint32_t horizontal_link;
    uint32_t characteristics;
    uint32_t capabilities;
    uint32_t current_link;
    uint32_t next_link;
    uint32_t alt_link;
    uint32_t token;
    uint32_t buffer[5];
    uint32_t ext_buffer[5];
} __attribute__((packed)) ehci_queue_head;

class EHCI : public InterruptHandler {
private:
    ehci_capabilities* capabilities = 0;
    ehci_operations* operations = 0;

    bool is_active = 0;
    uint16_t ehci_info = 0;
    uint8_t ehci_version = 0;
    uint32_t base_address = 0;
    int port_count = 0;
    int address_count = 0;

    void init_async_list();
    void init_periodic_list();

    void wait();
    void non_irq_timeout();

public:
    EHCI(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~EHCI();

    static EHCI* active;
    static driver_identifier_t identifier() { return { 0x0, 0x0, 0x0C, 0x03, 0x20 }; }

    bool take_ownership(device_descriptor_t device);
    void probe_port(uint16_t port);
    void probe_ports();
    void activate();
    void validate_reset();
    void reset();

    bool device_address();
    uint8_t transaction_send(ehci_transfer_descriptor* transfer);
    bool device_descriptor(uint8_t address, uint32_t page_buffer_ptr, uint8_t type, uint8_t index, uint8_t size);

    bool send_bulk_data(int address, int endpoint, void* buffer, uint32_t length);
    bool receive_bulk_data(int address, int endpoint, uint8_t toggle, void* buffer, uint32_t length);

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
