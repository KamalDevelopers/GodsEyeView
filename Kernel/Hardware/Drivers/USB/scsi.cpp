#include "scsi.hpp"
#include "ehci.hpp"

#define SCSI_READ_FORMAT_CAPACITIES 0x23
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_REQUEST_SENSE 0x03
#define SCSI_INQUIRY 0x12
#define SCSI_READ_CAPACITY_10 0x25
#define SCSI_READ_CAPACITY_16 0x9E
#define SCSI_READ_10 0x28
#define SCSI_WRITE_10 0x2A
#define SCSI_READ_16 0x88
#define SCSI_WRITE_16 0x8A

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

uint8_t SCSI::read_sector10(uint8_t* data, uint32_t sector, int size)
{
    if (size > 512) {
        kdbg("SCSI: read_sector10 read too large\n");
        return 0;
    }

    int sector_count = 1;
    scsi_command_block_wrapper* cbw = (scsi_command_block_wrapper*)transfer_buffer;
    memset(cbw, 0, sizeof(scsi_command_block_wrapper));
    cbw->signature = 0x43425355;
    cbw->tag = 0xBEEF;
    cbw->transfer_length = size;
    cbw->flags = 0x80;
    cbw->command_length = 10;
    cbw->data[0] = SCSI_READ_10;
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

uint8_t SCSI::write_sector10(uint8_t* data, uint32_t sector, int size)
{
    if (size > 512) {
        kdbg("SCSI: write_sector10 write too large\n");
        return 0;
    }

    int sector_count = 1;
    scsi_command_block_wrapper* cbw = (scsi_command_block_wrapper*)transfer_buffer;
    memset(cbw, 0, sizeof(scsi_command_block_wrapper));
    cbw->signature = 0x43425355;
    cbw->tag = 0xBEEF;
    cbw->transfer_length = size;
    cbw->flags = 0;
    cbw->command_length = 10;
    cbw->data[0] = SCSI_WRITE_10;
    cbw->data[1] = 0;

    /* LBA */
    cbw->data[2] = (uint8_t)((sector >> 24) & 0xFF);
    cbw->data[3] = (uint8_t)((sector >> 16) & 0xFF);
    cbw->data[4] = (uint8_t)((sector >> 8) & 0xFF);
    cbw->data[5] = (uint8_t)((sector >> 0) & 0xFF);

    /* Sector offset */
    cbw->data[6] = 0;
    cbw->data[7] = (uint8_t)((sector_count >> 8) & 0xFF);
    cbw->data[8] = (uint8_t)(sector_count & 0xFF);

    toggle = 0;
    if (!send_bulk_data(cbw, sizeof(scsi_command_block_wrapper)))
        return 0;
    if (!send_bulk_data(data, cbw->transfer_length))
        return 0;

    /* FIXME: Proper toggle */
    toggle = ((cbw->transfer_length / 512) & 2) == 2 ? 0 : 1;
    scsi_command_status_wrapper* csw = (scsi_command_status_wrapper*)transfer_buffer;
    memset(csw, 0, sizeof(scsi_command_status_wrapper));
    receive_bulk_data(csw, sizeof(scsi_command_status_wrapper));

    return size;
}

void SCSI::read(uint8_t* data, uint32_t sector, uint32_t count, size_t seek)
{
    static uint8_t buffer[512];
    uint8_t* data_ptr = data;
    uint32_t sector_reads = count / 512;
    uint32_t sector_count = sector + seek / 512;
    uint32_t count_left = count;

    if (!count)
        return;

    if (seek % 512) {
        int transfer_size = 512 - (seek % 512);
        if (!sector_reads && count % 512 && (count + seek) <= 512)
            transfer_size = count % 512;

        read_sector10(buffer, sector_count, 512);
        memcpy(data_ptr, buffer + (seek % 512), transfer_size);

        if (!sector_reads && count % 512 && (count + seek) <= 512)
            return;
        sector_count++;
        sector_reads -= (sector_reads) ? 1 : 0;
        data_ptr += transfer_size;
        count_left -= transfer_size;
    }

    count_left -= sector_reads * 512;
    while (sector_reads) {
        read_sector10(data_ptr, sector_count, 512);
        sector_count++;
        sector_reads--;
        data_ptr += 512;
    }

    if (count_left)
        read_sector10(data_ptr, sector_count, count_left);

    return;
}

void SCSI::write(uint8_t* data, uint32_t sector, uint32_t count)
{
    uint32_t sector_offset = 0;
    for (uint32_t i = 0; i < count;) {
        uint32_t siz = ((count - i) >= 512) ? 512 : count - i;
        write_sector10(data + i, sector + sector_offset, siz);
        sector_offset++;
        i += siz;
    }
}
