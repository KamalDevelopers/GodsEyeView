#include "ehci.hpp"

#define USB_LEGACY 0x00
#define HCCPARAMS_EECP_MASK 0xff00
#define HCCPARAMS_EECP_SHIFT 8
#define USB_LEGACY_HC_BIOS 0x0001000
#define USB_LEGACY_HC_OS 0x01000000
#define USB_STATUS_HALT 0x1000
#define USB_COMMAND_RESET 0x2
#define USB_COMMAND_ENABLE_INT 0x3f
#define PCI_INFO_REG 0x60
#define PERIODIC_FRAME_SIZE 1024

static size_t periodic_list[PERIODIC_FRAME_SIZE] __attribute__((aligned(0x1000)));
EHCI* EHCI::active = 0;

EHCI::EHCI(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
{
    active = this;
    Paging::map_page(PAGE_ALIGN_DOWN(device.bar0), PAGE_ALIGN_DOWN((device.bar0)));

    base_address = device.bar0;
    capabilities = (ehci_capabilities*)base_address;
    operations = (ehci_operations*)(base_address + capabilities->length);
    PCI::active->enable_busmaster(device);
    PCI::active->enable_memory_space(device);
    port_count = capabilities->hcs_params & 0xf;

    if (!take_ownership(device))
        klog("EHCI: Failed to take ownership from BIOS");

    ehci_info = PCI::active->read(device, PCI_INFO_REG);
    ehci_version = ehci_info & 0xFF;
    activate();
}

void EHCI::activate()
{
    if (is_active)
        return;
    is_active = true;

    reset();
    validate_reset();

    if (capabilities->version != 0x100)
        klog("EHCI: protocol unsupported version");
    if (ehci_version != 0x20)
        klog("EHCI: unsupported HCI version");

    for (uint32_t i = 0; i < PERIODIC_FRAME_SIZE; i++)
        periodic_list[i] |= 1;

    operations->control_dss = 0;
    operations->interrupt = 7; // Enable all USB interrupt
    operations->periodic_frame_list = (uint32_t)((size_t)&periodic_list);
    operations->command = 0x80001 | (0x40 << 16);
    ((uint32_t*)(base_address + capabilities->length + 0x40))[0] |= 1;
    probe_ports();
}

void EHCI::reset()
{
    operations->command &= ~1;
    while (operations->command & 1)
        asm volatile("nop");
    operations->command |= USB_COMMAND_RESET;
    while (operations->command & 2)
        asm volatile("nop");
}

void EHCI::validate_reset()
{
    if (operations->command != 0x80000)
        klog("EHCI: Warning unexpected USBCMD value");
    if (operations->interrupt != 0)
        klog("EHCI: Warning unexpected USBINT value");
    if (operations->control_dss != 0)
        klog("EHCI: Warning unexpected CTRLDSS value");
    if (operations->frame_index != 0)
        klog("EHCI: Warning unexpected FRINDEX value");
    if (operations->configure_flag != 0)
        klog("EHCI: Warning unexpected CONFIGFLAG value");
}

void EHCI::probe_port(uint16_t port)
{
    volatile uint32_t port_register = operations->port_source[port - 1];
    if (!(port_register & 3))
        return;

    /* TODO: Create usb device */
    klog("got connection on port=%d", port);
}

void EHCI::probe_ports()
{
    for (uint16_t i = 1; i < port_count; i++)
        probe_port(i);
}

bool EHCI::take_ownership(device_descriptor_t device)
{
    uint32_t ecp_offset = (capabilities->hcc_params & HCCPARAMS_EECP_MASK)
        >> HCCPARAMS_EECP_SHIFT;

    if (ecp_offset < 40)
        return true;

    uint32_t legacy_offset = ecp_offset + USB_LEGACY;
    uint32_t legsup = PCI::active->read(device, legacy_offset);

    if (!(legsup & USB_LEGACY_HC_BIOS))
        return true;

    PCI::active->write(device, legacy_offset, legsup | USB_LEGACY_HC_OS);

    for (;;) {
        legsup = PCI::active->read(device, legacy_offset);
        if (~legsup & USB_LEGACY_HC_BIOS && legsup & USB_LEGACY_HC_OS)
            return true;
    }

    return false;
}
