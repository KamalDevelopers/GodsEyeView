#include "scsi.hpp"
#include "ehci.hpp"

SCSI::SCSI(usb_device* device)
{
    usb = device;
    transfer_buffer = PMM->allocate_pages(PAGE_SIZE);
}

SCSI::~SCSI()
{
    PMM->free_pages(transfer_buffer, PAGE_SIZE);
}

bool SCSI::send_bulk_data(void* buffer, uint32_t length)
{
    uint8_t address = usb->address;
    uint8_t endpoint = usb->ep_out;
    if (usb->controller == USB_CONTROLLER_EHCI)
        return EHCI::active->send_bulk_data(address, endpoint, buffer, length);
    return 0;
}

bool SCSI::receive_bulk_data(void* buffer, uint32_t length)
{
    uint8_t address = usb->address;
    uint8_t endpoint = usb->ep_in;
    if (usb->controller == USB_CONTROLLER_EHCI)
        return EHCI::active->receive_bulk_data(address, endpoint, toggle, buffer, length);
    return 0;
}

uint8_t SCSI::scsi_read_sector10(uint8_t* data, uint32_t sector, int sector_count)
{
    scsi_command_block_wrapper* cbw = (scsi_command_block_wrapper*)transfer_buffer;
    memset(cbw, 0, sizeof(scsi_command_block_wrapper));
    cbw->signature = 0x43425355;
    cbw->tag = 0xBEEF;
    cbw->transfer_length = 512 * sector_count;
    cbw->flags = 0x80;
    cbw->command_length = 10;
    cbw->data[0] = 0x28;
    cbw->data[1] = 0;

    /* LBA */
    cbw->data[2] = (uint8_t)((sector >> 24) & 0xFF);
    cbw->data[3] = (uint8_t)((sector >> 16) & 0xFF);
    cbw->data[4] = (uint8_t)((sector >> 8) & 0xFF);
    cbw->data[5] = (uint8_t)((sector >> 0) & 0xFF);

    /* Sector offset */
    cbw->data[6] = 0;
    cbw->data[7] = (uint8_t)((sector_count >> 8) & 0xFF);
    cbw->data[8] = (uint8_t)((sector_count)&0xFF);

    toggle = 0;
    if (!send_bulk_data(cbw, sizeof(scsi_command_block_wrapper)))
        return 0;
    if (!receive_bulk_data(data, cbw->transfer_length))
        return 0;

    /* FIXME: Proper toggle */
    toggle = ((cbw->transfer_length / 512) & 2) == 2 ? 0 : 1;
    scsi_command_status_wrapper* csw = (scsi_command_status_wrapper*)transfer_buffer;
    memset(csw, 0, sizeof(scsi_command_status_wrapper));
    receive_bulk_data(csw, sizeof(scsi_command_status_wrapper));

    return sector_count;
}

uint8_t scsi_write_sector10(uint8_t* data, uint32_t sector, int sector_count)
{
    /* TODO */
    return 0;
}
