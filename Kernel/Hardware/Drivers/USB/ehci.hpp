#ifndef EHCI_HPP
#define EHCI_HPP

#include "../../../Locks/mutex.hpp"
#include "../../interrupts.hpp"
#include "../../pci.hpp"
#include <LibC/types.h>

#define MAX_PORT_SOURCES 16

struct ehci_capabilities {
    volatile uint8_t length;
    volatile uint8_t resv;
    volatile uint16_t version;
    volatile uint32_t hcs_params;
    volatile uint32_t hcc_params;
    volatile uint32_t port_route[2];
};

struct ehci_operations {
    volatile uint32_t command;
    volatile uint32_t status;
    volatile uint32_t interrupt;
    volatile uint32_t frame_index;
    volatile uint32_t control_dss;
    volatile uint32_t periodic_frame_list;
    volatile uint32_t async_list;
    volatile uint8_t resv[36];
    volatile uint32_t configure_flag;
    volatile uint32_t port_sc[MAX_PORT_SOURCES];
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
    volatile uint32_t next_link;
    volatile uint32_t alt_link;
    volatile uint32_t token;
    volatile uint32_t buffer[5];
    volatile uint32_t ext_buffer[5];
} __attribute__((packed)) ehci_transfer_descriptor;

typedef struct {
    volatile uint32_t horizontal_link;
    volatile uint32_t characteristics;
    volatile uint32_t capabilities;
    volatile uint32_t current_link;
    volatile uint32_t next_link;
    volatile uint32_t alt_link;
    volatile uint32_t token;
    volatile uint32_t buffer[5];
    volatile uint32_t ext_buffer[5];
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
    void timeout(float modifier = 1.0f);

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
    uint8_t transaction_send(volatile ehci_transfer_descriptor* transfer);
    bool device_descriptor(uint8_t address, uint32_t page_buffer_ptr, uint8_t type, uint8_t index, uint8_t size);

    bool send_bulk_data(int address, int endpoint, void* buffer, uint32_t length);
    bool receive_bulk_data(int address, int endpoint, uint8_t toggle, void* buffer, uint32_t length);

    virtual uint32_t interrupt(uint32_t esp);
};

#endif
