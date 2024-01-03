#include "ehci.hpp"
#include "usb.hpp"

#define HCI_VERSION 0x20
#define PROTOCOL_VERSION 0x100
#define VENDOR_QEMU 0x46f4
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
#define ASYNC_LIST_SIZE 16

#define QH_HS_TYPE_QHEAD (1 << 1)
#define QH_HS_TVALID (0 << 0)
#define QH_HS_TINVALID (1 << 0)

#define PORT_RESET (1 << 8)
#define PORT_ENABLE (1 << 2)
#define PORT_CONNECTION_CHANGE (1 << 1)
#define PORT_ENABLE_CHANGE (1 << 3)

#define INT_ENABLE (1 << 0)
#define INT_ERROR_ENABLE (1 << 1)
#define INT_PORT_CHANGE (1 << 2)
#define INT_FRAME_ROLLOVER (1 << 3)
#define INT_HOST_ERROR (1 << 4)
#define INT_ON_ASYNC (1 << 5)

#define REQUEST_SYNCH_FRAME 12
#define REQUEST_SET_INTERFACE 11
#define REQUEST_GET_INTERFACE 10
#define REQUEST_SET_CONFIGURATION 9
#define REQUEST_GET_CONFIGURATION 8
#define REQUEST_SET_DESCRIPTOR 7
#define REQUEST_GET_DESCRIPTOR 6
#define REQUEST_SET_ADDRESS 5
#define REQUEST_SET_FEATURE 3
#define REQUEST_CLEAR_FEATURE 1
#define REQUEST_GET_STATUS 0

#define DESCRIPTOR_TYPE_POWER 8
#define DESCRIPTOR_TYPE_OTHER 7
#define DESCRIPTOR_TYPE_QUALI 6
#define DESCRIPTOR_TYPE_ENDPOINT 5
#define DESCRIPTOR_TYPE_INTERFACE 4
#define DESCRIPTOR_TYPE_STRING 3
#define DESCRIPTOR_TYPE_CONFIGURATION 2
#define DESCRIPTOR_TYPE_DEVICE 1

static size_t periodic_list[PERIODIC_FRAME_SIZE] __attribute__((aligned(0x1000)));
static ehci_queue_head* queue_head_primary;
static ehci_queue_head* queue_head_secondary;

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
        kdbg("EHCI: Failed to take ownership from BIOS\n");

    ehci_info = PCI::active->read(device, PCI_INFO_REG);
    ehci_version = ehci_info & 0xFF;
    activate();
}

EHCI::~EHCI()
{
    PMM->free_pages((uint32_t)queue_head_primary, PAGE_SIZE);
    PMM->free_pages((uint32_t)queue_head_secondary, PAGE_SIZE);
}

void EHCI::activate()
{
    if (is_active)
        return;
    is_active = true;

    operations->interrupt = 0;
    reset();
    validate_reset();

    if (capabilities->version != PROTOCOL_VERSION)
        klog("EHCI: protocol unsupported version");
    if (ehci_version != HCI_VERSION)
        klog("EHCI: unsupported HCI version");

    init_async_list();
    init_periodic_list();

    operations->periodic_frame_list = (uint32_t)((size_t)&periodic_list);
    operations->async_list = (uint32_t)queue_head_primary;

    operations->control_dss = 0;
    operations->interrupt = INT_ENABLE | INT_ERROR_ENABLE;

    timeout();
    // operations->command = 0x80001 | (0x40 << 16);
    operations->command |= (8 << 16) | (1 << 5) | (1 << 4) | (1 << 0);

    operations->configure_flag = 1;
    ((uint32_t*)(base_address + capabilities->length + 0x40))[0] |= 1;
    probe_ports();
}

void EHCI::timeout(float modifier)
{
    /* 0.0062 seconds = 62 * 10^(-9) * 1000000 */
    int time = (int)(100000.0f * modifier);
    for (int timeout = 100000; timeout > 0; timeout--) {
        /* if (TM->is_active())
            TM->yield(); */
        asm volatile("nop");
    }
}

void EHCI::reset()
{
    operations->command &= ~1;
    while (operations->command & 1)
        asm volatile("nop");
    operations->command |= USB_COMMAND_RESET;
    while (operations->command & USB_COMMAND_RESET)
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

void EHCI::init_async_list()
{
    queue_head_primary = (ehci_queue_head*)PMM->allocate_pages(PAGE_SIZE);
    queue_head_secondary = (ehci_queue_head*)PMM->allocate_pages(PAGE_SIZE);
    queue_head_primary->horizontal_link = (uint32_t)queue_head_secondary | QH_HS_TYPE_QHEAD;
    queue_head_secondary->horizontal_link = (uint32_t)queue_head_primary | QH_HS_TYPE_QHEAD;
}

void EHCI::init_periodic_list()
{
    for (uint32_t i = 0; i < PERIODIC_FRAME_SIZE; i++)
        periodic_list[i] = 1;
}

void EHCI::probe_port(uint16_t port)
{
    uint16_t port_index = port;
    volatile uint32_t port_register = operations->port_source[port_index];
    if (!(port_register & 3))
        return;

    if (((port_register >> 10) & 0x3) == 1) {
        kdbg("EHCI: Low speed device detected, skipping...\n");
        return;
    }

    uint32_t buffer = PMM->allocate_pages(PAGE_SIZE);
    usb_device* usb = usb_allocate_device();
    usb->protocol = 2;
    usb->port = port;
    usb->controller = USB_CONTROLLER_EHCI;

    /* Reset port */
    uint32_t reg = operations->port_source[port_index];
    reg |= PORT_RESET;
    operations->port_source[port_index] = reg;
    timeout();
    operations->port_source[port_index] &= ~PORT_RESET;
    timeout();

    usb->address = address_count;
    if (!device_address())
        kdbg("EHCI: Failed to assign device address\n");
    timeout();

    memset((void*)buffer, 0, sizeof(usb_device_descriptor));
    if (!device_descriptor(usb->address, buffer, DESCRIPTOR_TYPE_DEVICE, 0, sizeof(usb_device_descriptor)))
        kdbg("EHCI: Failed to get device descriptor\n");
    memcpy(&(usb->device_descriptor), (void*)buffer, sizeof(usb_device_descriptor));

    int packet_size = sizeof(usb_conf_descriptor) + sizeof(usb_interface_descriptor) + sizeof(ehci_device_endpoint) * 2;
    memset((void*)buffer, 0, packet_size);
    if (!device_descriptor(usb->address, buffer, DESCRIPTOR_TYPE_CONFIGURATION, 0, packet_size))
        kdbg("EHCI: Failed to get configuration descriptor\n");

    /* Assign endpoints */
    ehci_device_endpoint* ep_primary = (ehci_device_endpoint*)((uint32_t)buffer + packet_size - sizeof(ehci_device_endpoint) * 2);
    ehci_device_endpoint* ep_secondary = (ehci_device_endpoint*)((uint32_t)buffer + packet_size - sizeof(ehci_device_endpoint));

    if (ep_primary->address & (1 << 7)) {
        usb->ep_in = ep_primary->address & 0xF;
        usb->ep_out = ep_secondary->address & 0xF;
    } else {
        usb->ep_out = ep_primary->address & 0xF;
        usb->ep_in = ep_secondary->address & 0xF;
    }

    if (usb->device_descriptor.type != DESCRIPTOR_TYPE_DEVICE)
        kdbg("EHCI: Descriptor requests failed\n");
    /* if (usb->device_descriptor.vendor == VENDOR_QEMU)
        kdbg("USB: Virtual usb device connected\n"); */

    PMM->free_pages(buffer, PAGE_SIZE);
}

void EHCI::probe_ports()
{
    for (uint16_t i = 0; i < port_count; i++)
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

uint8_t EHCI::transaction_send(ehci_transfer_descriptor* transfer)
{
    operations->command |= (1 << 5);
    uint8_t err = 0;

    while (1) {
        volatile uint32_t token = (volatile uint32_t)transfer->token;
        if (token & (1 << 3)) {
            kdbg("EHCI: Transaction error\n");
            err = 3;
            break;
        }
        if (token & (1 << 4)) {
            kdbg("EHCI: Transaction babble error\n");
            err = 4;
            break;
        }
        if (token & (1 << 5)) {
            kdbg("EHCI: Transaction data error\n");
            err = 5;
            break;
        }
        if (token & (1 << 6)) {
            kdbg("EHCI: Transaction fatal error\n");
            err = 6;
            break;
        }
        if (!(token & (1 << 7)))
            break;
        timeout();
    }

    operations->command &= ~(1 << 5);
    return err;
}

bool EHCI::device_descriptor(uint8_t address, uint32_t page_buffer_ptr, uint8_t type, uint8_t index, uint8_t size)
{
    /* Command structure */
    ehci_command* command = (ehci_command*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(command, 0, sizeof(ehci_command));
    command->request = REQUEST_GET_DESCRIPTOR;
    command->length = size;
    command->type |= (4 << 5);
    command->value = (type << 8) | index;

    /* Transfer descriptor structures */
    ehci_transfer_descriptor* status = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(status, 0, sizeof(ehci_transfer_descriptor));
    status->next_link = 1;
    status->alt_link = 1;
    status->token |= (1 << 7) | (0 << 8) | (3 << 10) | (0 << 16) | (1 << 31);

    ehci_transfer_descriptor* transfer_command = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(transfer_command, 0, sizeof(ehci_transfer_descriptor));
    transfer_command->next_link = (uint32_t)status;
    transfer_command->alt_link = 1;
    transfer_command->token |= (1 << 7) | (1 << 8) | (3 << 10) | (size << 16) | (1 << 31);
    transfer_command->buffer[0] = page_buffer_ptr;

    ehci_transfer_descriptor* transfer_descriptor = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(transfer_descriptor, 0, sizeof(ehci_transfer_descriptor));
    transfer_descriptor->next_link = (uint32_t)transfer_command;
    transfer_descriptor->alt_link = 1;
    transfer_descriptor->token |= (1 << 7) | (2 << 8) | (3 << 10) | (8 << 16) | (0 << 31);
    transfer_descriptor->buffer[0] = (uint32_t)command;

    /* Queue head structures */
    ehci_queue_head* head_primary = queue_head_primary;
    memset(head_primary, 0, sizeof(ehci_queue_head));
    head_primary->alt_link = 1;
    head_primary->next_link = 1;
    head_primary->characteristics |= (0 << 12) | (0 << 14) | (1 << 15) | (0 << 16) | (0 << 8) | 0;
    head_primary->token = 0x40;

    ehci_queue_head* head_secondary = queue_head_secondary;
    memset(head_secondary, 0, sizeof(ehci_queue_head));
    head_secondary->alt_link = 1;
    head_secondary->next_link = (uint32_t)transfer_descriptor;
    head_secondary->characteristics |= (2 << 12) | (1 << 14) | (0 << 15) | (64 << 16) | (0 << 8) | address;
    head_secondary->capabilities = 0x40000000;

    head_primary->horizontal_link = (uint32_t)head_secondary | QH_HS_TYPE_QHEAD;
    head_secondary->horizontal_link = (uint32_t)head_primary | QH_HS_TYPE_QHEAD;

    uint8_t err = transaction_send(status);

    /* Done */
    PMM->active->free_pages((uint32_t)command, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)status, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)transfer_command, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)transfer_descriptor, PAGE_SIZE);
    return (err == 0);
}

bool EHCI::device_address()
{
    /* Command structure */
    ehci_command* command = (ehci_command*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(command, 0, sizeof(ehci_command));
    command->request = REQUEST_SET_ADDRESS;
    command->value = address_count;

    /* Transfer descriptor structures */
    ehci_transfer_descriptor* status = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(status, 0, sizeof(ehci_transfer_descriptor));
    status->next_link = 1;
    status->alt_link = 1;
    status->token |= (1 << 7) | (1 << 8) | (3 << 10) | (0 << 16) | (1 << 31);

    ehci_transfer_descriptor* transfer_command = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(transfer_command, 0, sizeof(ehci_transfer_descriptor));
    transfer_command->next_link = (uint32_t)status;
    transfer_command->alt_link = 1;
    transfer_command->token |= (1 << 7) | (2 << 8) | (3 << 10) | (8 << 16) | (0 << 31);
    transfer_command->buffer[0] = (uint32_t)command;

    /* Queue head structures */
    ehci_queue_head* head_primary = queue_head_primary;
    memset(head_primary, 0, sizeof(ehci_queue_head));
    head_primary->alt_link = 1;
    head_primary->next_link = 1;
    head_primary->characteristics |= (0 << 12) | (0 << 14) | (1 << 15) | (0 << 16) | (0 << 8) | 0;
    head_primary->token = 0x40;

    ehci_queue_head* head_secondary = queue_head_secondary;
    memset(head_secondary, 0, sizeof(ehci_queue_head));
    head_secondary->alt_link = 1;
    head_secondary->next_link = (uint32_t)transfer_command;
    head_secondary->characteristics |= (2 << 12) | (1 << 14) | (0 << 15) | (64 << 16) | (0 << 8) | 0;
    head_secondary->capabilities = 0x40000000;

    head_primary->horizontal_link = (uint32_t)head_secondary | QH_HS_TYPE_QHEAD;
    head_secondary->horizontal_link = (uint32_t)head_primary | QH_HS_TYPE_QHEAD;

    uint8_t err = transaction_send(status);

    /* Done */
    PMM->active->free_pages((uint32_t)command, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)status, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)transfer_command, PAGE_SIZE);
    address_count++;
    return (err == 0);
}

bool EHCI::receive_bulk_data(int address, int endpoint, uint8_t toggle, void* buffer, uint32_t length)
{
    if (length > 512)
        kdbg("EHCI: Warning receive_bulk_data length\n");

    uint32_t aligned_buffer = PMM->allocate_pages(PAGE_ALIGN(length));
    uint16_t package_size = length < 512 ? length : 512;

    ehci_transfer_descriptor* status = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(status, 0, sizeof(ehci_transfer_descriptor));
    status->next_link = 1;
    status->alt_link = 1;
    status->token |= (1 << 7) | (1 << 8) | (3 << 10) | (package_size << 16) | (toggle << 31);
    status->buffer[0] = (uint32_t)aligned_buffer;

    /* Queue head structure */
    ehci_queue_head* head_primary = queue_head_primary;
    memset(head_primary, 0, sizeof(ehci_queue_head));
    head_primary->alt_link = 1;
    head_primary->next_link = 1;
    head_primary->characteristics |= (0 << 12) | (0 << 14) | (1 << 15) | (0 << 16) | (0 << 8) | 0;
    head_primary->token = 0x40;

    ehci_queue_head* head_secondary = queue_head_secondary;
    memset(head_secondary, 0, sizeof(ehci_queue_head));
    head_secondary->alt_link = 1;
    head_secondary->next_link = (uint32_t)status;
    head_secondary->characteristics |= (2 << 12) | (1 << 14) | (0 << 15) | (512 << 16) | (endpoint << 8) | address;
    head_secondary->capabilities = 0x40000000;

    head_primary->horizontal_link = (uint32_t)head_secondary | QH_HS_TYPE_QHEAD;
    head_secondary->horizontal_link = (uint32_t)head_primary | QH_HS_TYPE_QHEAD;

    uint8_t err = transaction_send(status);

    if (buffer)
        memcpy(buffer, (void*)aligned_buffer, length);
    PMM->free_pages(aligned_buffer, PAGE_ALIGN(length));
    PMM->free_pages((uint32_t)status, PAGE_SIZE);
    return (err == 0);
}

bool EHCI::send_bulk_data(int address, int endpoint, void* buffer, uint32_t length)
{
    /* Transfer descriptor structures */
    ehci_transfer_descriptor* command_status = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(command_status, 0, sizeof(ehci_transfer_descriptor));
    command_status->next_link = 1;
    command_status->alt_link = 1;
    command_status->token |= (1 << 7) | (0 << 8) | (3 << 10) | (0 << 16) | (1 << 31);

    ehci_transfer_descriptor* status = (ehci_transfer_descriptor*)PMM->active->allocate_pages(PAGE_SIZE);
    memset(status, 0, sizeof(ehci_transfer_descriptor));
    status->next_link = (uint32_t)status;
    status->alt_link = 1;
    status->token |= (1 << 7) | (0 << 8) | (3 << 10) | (length << 16) | (0 << 31);
    status->buffer[0] = (uint32_t)buffer;

    /* Queue head structure */
    ehci_queue_head* head_primary = queue_head_primary;
    memset(head_primary, 0, sizeof(ehci_queue_head));
    head_primary->alt_link = 1;
    head_primary->next_link = 1;
    head_primary->characteristics |= (0 << 12) | (0 << 14) | (1 << 15) | (0 << 16) | (0 << 8) | 0;
    head_primary->token = 0x40;

    ehci_queue_head* head_secondary = queue_head_secondary;
    memset(head_secondary, 0, sizeof(ehci_queue_head));
    head_secondary->alt_link = 1;
    head_secondary->next_link = (uint32_t)status;
    head_secondary->characteristics |= (2 << 12) | (1 << 14) | (0 << 15) | (512 << 16) | (endpoint << 8) | address;
    head_secondary->capabilities = 0x40000000;

    head_primary->horizontal_link = (uint32_t)head_secondary | QH_HS_TYPE_QHEAD;
    head_secondary->horizontal_link = (uint32_t)head_primary | QH_HS_TYPE_QHEAD;

    uint8_t err = transaction_send(status);

    PMM->active->free_pages((uint32_t)status, PAGE_SIZE);
    PMM->active->free_pages((uint32_t)command_status, PAGE_SIZE);
    return (err == 0);
}

uint32_t EHCI::interrupt(uint32_t esp)
{
    uint32_t status = operations->status;
    operations->status = status;

    if (status & 0x2) {
        klog("EHCI: Transaction error");
    }

    /* if (status & 0x4)
     * TODO: Support dynamic probing
     *  klog("EHCI: Port change"); */
    return esp;
}
