#ifndef EHCI_HPP
#define EHCI_HPP

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

struct ehci_queue_head {
    uint32_t data[17];
} __attribute__((packed));

struct ehci_device_descriptor_request {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed));

class EHCI : public InterruptHandler {
private:
    ehci_capabilities* capabilities = 0;
    ehci_operations* operations = 0;

    bool is_active = 0;
    uint16_t ehci_info = 0;
    uint8_t ehci_version = 0;
    uint32_t base_address = 0;
    int port_count = 0;

public:
    EHCI(InterruptManager* interrupt_manager, device_descriptor_t device);
    ~EHCI();

    static driver_identifier_t identifier() { return { 0x0, 0x0, 0x0C, 0x03, 0x20 }; }

    bool take_ownership(device_descriptor_t device);
    void probe_port(uint16_t port);
    void probe_ports();
    void activate();
    void validate_reset();
    void reset();

    static EHCI* active;
};

#endif
