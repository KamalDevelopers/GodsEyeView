#include "ehci.hpp"

#define USB_LEGACY 0x00
#define HCCPARAMS_EECP_MASK 0xff00
#define HCCPARAMS_EECP_SHIFT 8
#define USB_LEGACY_HC_BIOS 0x0001000
#define USB_LEGACY_HC_OS 0x01000000
#define USB_STATUS_HALT 0x1000
#define USB_COMMAND_RESET 0x2
#define USB_COMMAND_ENABLE_INT 0x3f

EHCI::EHCI(InterruptManager* interrupt_manager, device_descriptor_t device)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt)
{
    Paging::map_page(PAGE_ALIGN_DOWN(device.bar0), PAGE_ALIGN_DOWN((device.bar0)));

    base_address = device.bar0;
    capabilities = (ehci_capabilities*)base_address;
    PCI::active->enable_busmaster(device);
    PCI::active->enable_memory_space(device);

    if (!take_ownership(device))
        klog("EHCI: Failed to take ownership from BIOS");

    operations = (ehci_operations*)(base_address + capabilities->length);
    reset();

    /* TODO: Scan ports */
}

void EHCI::reset()
{
    operations->command &= 0xfffffffe;
    while ((operations->status & USB_STATUS_HALT) != USB_STATUS_HALT)
        ;

    operations->command |= USB_COMMAND_RESET;
    /* while ((operations->command & USB_COMMAND_RESET) == USB_COMMAND_RESET)
        ; */

    port_count = capabilities->hcs_params & 0xf;
    operations->control_dss = 0;
    operations->interrupt = USB_COMMAND_ENABLE_INT;
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

void EHCI::activate()
{
}
