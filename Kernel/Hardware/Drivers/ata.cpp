#include "ata.hpp"

MUTEX(ata);
#ifdef RAMDISK
extern char _binary_hdd_tar_start;
extern char _binary_hdd_tar_end;
#endif

ATA::ATA(InterruptManager* interrupt_manager, device_descriptor_t device, uint32_t port_base, bool master)
    : InterruptHandler(interrupt_manager, interrupt_manager->get_hardware_interrupt_offset() + device.interrupt + 14)
    , data_port(port_base)
    , error_port(port_base + 0x1)
    , sector_count_port(port_base + 0x2)
    , lba_low_port(port_base + 0x3)
    , lba_mid_port(port_base + 0x4)
    , lba_hi_port(port_base + 0x5)
    , device_port(port_base + 0x6)
    , command_port(port_base + 0x7)
    , control_port(port_base + 0x206)
{
    PCI::active->enable_busmaster(device);
    dma = false;

    memset(&prdt, 0, sizeof(prdt_t));
    prdt.buffer_phys = 0;
    prdt.transfer_size = 512;
    prdt.mark_end = 0x8000;

    bar4 = PCI::active->read(device.bus, device.device, device.function, 0x20);
    if (bar4 & 0x1)
        bar4 = bar4 & 0xfffffffc;

    this->device = device;
    this->master = master;
}

ATA::~ATA()
{
}

void ATA::set_dma(bool dma)
{
    this->dma = dma;
}

void ATA::identify()
{
    device_port.write(master ? 0xA0 : 0xB0);
    control_port.write(0);

    device_port.write(0xA0);
    uint8_t status = command_port.read();
    if (status == 0xFF)
        return;

    device_port.write(master ? 0xA0 : 0xB0);
    sector_count_port.write(0);
    lba_low_port.write(0);
    lba_mid_port.write(0);
    lba_hi_port.write(0);
    command_port.write(0xEC);

    status = command_port.read();
    if (status == 0x00)
        return;

    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01)
        return;

    for (int i = 0; i < 256; i++) {
        uint16_t data = data_port.read();
        char* text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
    }

    uint32_t pci_command_reg = PCI::active->read(device.bus, device.device, device.function, 0x04);
    if (!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        PCI::active->write(device.bus, device.device, device.function, 0x04, pci_command_reg);
    }
}

uint8_t* ATA::read28_dma(uint32_t sector_num, uint8_t* data, int count, int scount)
{
#ifdef RAMDISK
    uint8_t* disk = (uint8_t*)&_binary_hdd_tar_start;
    memcpy(data, disk + sector_num * 512, count);
    return data;
#endif

    Mutex::lock(ata);
    prdt.transfer_size = count;
    prdt.buffer_phys = (uint32_t)data;

    uint8_t sector[512];
    if (count < 512 && scount == 1) {
        prdt.transfer_size = 512;
        prdt.buffer_phys = (uint32_t)sector;
    }

    outb(bar4, 0);
    outbl(bar4 + 4, (uint32_t)&prdt);
    device_port.write(0xE0 | (!master) << 4 | (sector_num & 0x0f000000) >> 24);
    if (count > 512 && scount == 1)
        scount = count / 512;
    sector_count_port.write(scount);
    lba_low_port.write(sector_num & 0x000000ff);
    lba_mid_port.write((sector_num & 0x0000ff00) >> 8);
    lba_hi_port.write((sector_num & 0x00ff0000) >> 16);
    command_port.write(0xC8);
    outb(bar4 + 0x02, inb(bar4 + 0x02) | 0x04 | 0x02);
    outb(bar4, 0x8 | 0x1);

    while (1) {
        int status = inb(bar4 + 2);
        int dstatus = command_port.read();
        if (!(status & 0x04))
            continue;
        if (!(dstatus & 0x80))
            break;
    }

    if (count < 512 && scount == 1)
        memcpy(data, sector, count);

    Mutex::unlock(ata);
    return data;
}

uint8_t* ATA::read28_pio(uint32_t sector_num, uint8_t* data, int count)
{
#ifdef RAMDISK
    uint8_t* disk = (uint8_t*)&_binary_hdd_tar_start;
    memcpy(data, disk + sector_num * 512, count);
    return data;
#endif

    Mutex::lock(ata);
    static uint8_t buffer[BUFSIZ];
    uint16_t index = 0;

    for (int i = 0; i < BUFSIZ; i++)
        buffer[i] = 0;

    if (sector_num > 0x0FFFFFFF) {
        Mutex::unlock(ata);
        return nullptr;
    }

    device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_num & 0x000000FF);
    lba_mid_port.write((sector_num & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_num & 0x00FF0000) >> 16);
    command_port.write(0x20);

    uint8_t status = command_port.read();
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();

    if (status & 0x01) {
        Mutex::unlock(ata);
        return nullptr;
    }

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data_port.read();

        buffer[index] = wdata & 0x0FF;
        buffer[index + 1] = (wdata >> 8) & 0x0FF;
        index += 2;

        data[i] = wdata & 0x00FF;
        if (i + 1 < count)
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.read();

    Mutex::unlock(ata);
    return buffer;
}

void ATA::write28_pio(uint32_t sector_num, uint8_t* data, uint32_t count)
{
#ifdef RAMDISK
    return;
#endif
    if ((sector_num > 0x0FFFFFFF) || (count > BUFSIZ))
        return;

    Mutex::lock(ata);
    device_port.write((master ? 0xE0 : 0xF0) | ((sector_num & 0x0F000000) >> 24));
    error_port.write(0);
    sector_count_port.write(1);
    lba_low_port.write(sector_num & 0x000000FF);
    lba_mid_port.write((sector_num & 0x0000FF00) >> 8);
    lba_hi_port.write((sector_num & 0x00FF0000) >> 16);
    command_port.write(0x30);

    for (int i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if (i + 1 < count)
            wdata |= ((uint16_t)data[i + 1]) << 8;
        data_port.write(wdata);
    }

    for (int i = count + (count % 2); i < BUFSIZ; i += 2)
        data_port.write(0x0000);
    Mutex::unlock(ata);
}

void ATA::write28_dma(uint32_t sector_num, uint8_t* data, uint32_t count)
{
    Mutex::lock(ata);

    prdt.transfer_size = count;
    prdt.buffer_phys = (uint32_t)data;
    outb(bar4, 0);
    outbl(bar4 + 4, (uint32_t)&prdt);

    uint8_t sector[512];
    if (count < 512) {
        memcpy(sector, data, 512);
        prdt.transfer_size = 512;
        prdt.buffer_phys = (uint32_t)sector;
    }

    device_port.write(0xE0 | (!master) << 4 | (sector_num & 0x0f000000) >> 24);
    sector_count_port.write(1);
    lba_low_port.write(sector_num & 0x000000ff);
    lba_mid_port.write((sector_num & 0x0000ff00) >> 8);
    lba_hi_port.write((sector_num & 0x00ff0000) >> 16);
    command_port.write(0xCA);
    outb(bar4, 0x1);

    while (1) {
        int status = inb(bar4 + 2);
        int dstatus = command_port.read();
        if (!(status & 0x04))
            continue;
        if (!(dstatus & 0x80))
            break;
    }
    Mutex::unlock(ata);
}

void ATA::flush_pio()
{
    if (dma)
        return;
#ifdef RAMDISK
    return;
#endif
    device_port.write(master ? 0xE0 : 0xF0);
    command_port.write(0xE7);

    uint8_t status = command_port.read();
    if (status == 0x00)
        return;

    Mutex::lock(ata);
    while (((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01))
        status = command_port.read();
    Mutex::unlock(ata);

    if (status & 0x01)
        return;
}

uint8_t* ATA::read28(uint32_t sector_num, uint8_t* data, int count, int scount)
{
    if (dma)
        return read28_dma(sector_num, data, count, scount);
    return read28_pio(sector_num, data, count);
}

void ATA::write28(uint32_t sector_num, uint8_t* data, uint32_t count)
{
    if (dma)
        return write28_dma(sector_num, data, count);
    write28_pio(sector_num, data, count);
}

void ATA::flush()
{
    if (dma)
        return;
    flush_pio();
}

uint32_t ATA::interrupt(uint32_t esp)
{
    control_port.read();
    inb(bar4 + 2);
    outb(bar4, 0x0);
    return esp;
}
