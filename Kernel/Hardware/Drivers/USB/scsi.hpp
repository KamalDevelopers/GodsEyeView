#ifndef SCSI_HPP
#define SCSI_HPP

#include "usb.hpp"

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t transfer_length;
    uint8_t flags;
    uint8_t lun;
    uint8_t command_length;
    uint8_t data[16];
} __attribute__((packed)) scsi_command_block_wrapper;

typedef struct {
    uint32_t signature;
    uint32_t tag;
    uint32_t data_residue;
    uint8_t status;
} __attribute__((packed)) scsi_command_status_wrapper;

class SCSI {
private:
    usb_device* usb = 0;
    uint8_t toggle = 0;
    uint32_t transfer_buffer = 0;

    bool receive_bulk_data(void* buffer, uint32_t length);
    bool send_bulk_data(void* buffer, uint32_t length);

public:
    SCSI(usb_device* device);
    ~SCSI();

    uint8_t scsi_read_sector10(uint8_t* data, uint32_t sector, int sector_count);
    uint8_t scsi_write_sector10(uint8_t* data, uint32_t sector, int sector_count);
};

#endif
