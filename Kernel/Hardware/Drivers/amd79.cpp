#include "amd79.hpp"

RawDataHandler::RawDataHandler(AmdDriver* backend)
{
    this->backend = backend;
    backend->set_handler(this);
}

RawDataHandler::~RawDataHandler()
{
    backend->set_handler(0);
}

bool RawDataHandler::on_raw_data_received(uint8_t* buffer, uint32_t size)
{
    return false;
}

void RawDataHandler::send(uint8_t* buffer, uint32_t size)
{
    backend->send(buffer, size);
}

AmdDriver::AmdDriver(DeviceDescriptor* dev, InterruptManager* interrupts)
    : Driver()
    , InterruptHandler(interrupts, dev->interrupt + interrupts->get_hardware_interrupt_offset())
    , MACAddress0Port(dev->port_base)
    , MACAddress2Port(dev->port_base + 0x02)
    , MACAddress4Port(dev->port_base + 0x04)
    , registerDataPort(dev->port_base + 0x10)
    , registerAddressPort(dev->port_base + 0x12)
    , resetPort(dev->port_base + 0x14)
    , busControlRegisterDataPort(dev->port_base + 0x16)
{
    this->handler = 0;
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    uint64_t mac0 = MACAddress0Port.read() % 256;
    uint64_t mac1 = MACAddress0Port.read() / 256;
    uint64_t mac2 = MACAddress2Port.read() % 256;
    uint64_t mac3 = MACAddress2Port.read() / 256;
    uint64_t mac4 = MACAddress4Port.read() % 256;
    uint64_t mac5 = MACAddress4Port.read() / 256;

    uint64_t mac = mac5 << 40
        | mac4 << 32
        | mac3 << 24
        | mac2 << 16
        | mac1 << 8
        | mac0;

    // 32 bit mode
    registerAddressPort.write(20);
    busControlRegisterDataPort.write(0x102);

    // STOP reset
    registerAddressPort.write(0);
    registerDataPort.write(0x04);

    // initBlock
    initBlock.mode = 0x0000; // promiscuous mode = false
    initBlock.reserved1 = 0;
    initBlock.numSendBuffers = 3;
    initBlock.reserved2 = 0;
    initBlock.numRecvBuffers = 3;
    initBlock.physicalAddress = mac;
    initBlock.reserved3 = 0;
    initBlock.logicalAddress = 0;

    sendBufferDescr = (buffer_descriptor*)((((uint32_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.send_buffer_descr_address = (uint32_t)sendBufferDescr;
    recvBufferDescr = (buffer_descriptor*)((((uint32_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.recv_buffer_descr_address = (uint32_t)recvBufferDescr;

    for (uint8_t i = 0; i < 8; i++) {
        sendBufferDescr[i].address = (((uint32_t)&sendBuffers[i]) + 15) & ~(uint32_t)0xF;
        sendBufferDescr[i].flags = 0x7FF
            | 0xF000;
        sendBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;

        recvBufferDescr[i].address = (((uint32_t)&recvBuffers[i]) + 15) & ~(uint32_t)0xF;
        recvBufferDescr[i].flags = 0xF7FF
            | 0x80000000;
        recvBufferDescr[i].flags2 = 0;
        sendBufferDescr[i].avail = 0;
    }

    registerAddressPort.write(1);
    registerDataPort.write((uint32_t)(&initBlock) & 0xFFFF);
    registerAddressPort.write(2);
    registerDataPort.write(((uint32_t)(&initBlock) >> 16) & 0xFFFF);
}

AmdDriver::~AmdDriver()
{
}

void AmdDriver::send(uint8_t* buffer, int size)
{
    int send_descriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;

    if (size > 1518)
        size = 1518;

    for (uint8_t *src = buffer + size - 1,
                 *dst = (uint8_t*)(sendBufferDescr[send_descriptor].address + size - 1);
         src >= buffer; src--, dst--)
        *dst = *src;

    printf("\nSending: ");
    for (int i = 0; i < size; i++) {
        printf("%x", buffer[i]);
        printf(" ");
    }

    sendBufferDescr[send_descriptor].avail = 0;
    sendBufferDescr[send_descriptor].flags2 = 0;
    sendBufferDescr[send_descriptor].flags = 0x8300F000
        | ((uint16_t)((-size) & 0xFFF));
    registerAddressPort.write(0);
    registerDataPort.write(0x48);
}

void AmdDriver::receive()
{
    for (; (recvBufferDescr[currentRecvBuffer].flags & 0x80000000) == 0;
         currentRecvBuffer = (currentRecvBuffer + 1) % 8) {
        if (!(recvBufferDescr[currentRecvBuffer].flags & 0x40000000)
            && (recvBufferDescr[currentRecvBuffer].flags & 0x03000000) == 0x03000000)

        {
            uint32_t size = recvBufferDescr[currentRecvBuffer].flags & 0xFFF;
            if (size > 64)
                size -= 4;

            uint8_t* buffer = (uint8_t*)(recvBufferDescr[currentRecvBuffer].address);

            if (handler != 0)
                if (handler->on_raw_data_received(buffer, size))
                    send(buffer, size);

            printf("\nData Received: ");
            size = 64;
            for (int i = 0; i < size; i++) {
                printf("%x", buffer[i]);
                printf(" ");
            }
        }

        recvBufferDescr[currentRecvBuffer].flags2 = 0;
        recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
    }
}

void AmdDriver::set_handler(RawDataHandler* handler)
{
    this->handler = handler;
}

uint64_t AmdDriver::get_mac_address()
{
    return initBlock.physicalAddress;
}

void AmdDriver::set_ip_address(uint32_t ip)
{
    initBlock.logicalAddress = ip;
}

uint32_t AmdDriver::get_ip_address()
{
    return initBlock.logicalAddress;
}

void AmdDriver::activate()
{
    registerAddressPort.write(0);
    registerDataPort.write(0x41);

    registerAddressPort.write(4);
    uint32_t temp = registerDataPort.read();
    registerAddressPort.write(4);
    registerDataPort.write(temp | 0xC00);

    registerAddressPort.write(0);
    registerDataPort.write(0x42);
}

int AmdDriver::reset()
{
    resetPort.read();
    resetPort.write(0);
    return 10;
}

uint32_t AmdDriver::interrupt(uint32_t esp)
{
    printf("%s", "\nINTERRUPT FROM AMD am79c973\n");

    registerAddressPort.write(0);
    uint32_t temp = registerDataPort.read();

    if ((temp & 0x8000) == 0x8000)
        printf("\nAMD am79c973 ERROR\n");
    if ((temp & 0x2000) == 0x2000)
        printf("\nAMD am79c973 COLLISION ERROR\n");
    if ((temp & 0x1000) == 0x1000)
        printf("\nAMD am79c973 MISSED FRAME\n");
    if ((temp & 0x0800) == 0x0800)
        printf("\nAMD am79c973 MEMORY ERROR\n");
    if ((temp & 0x0400) == 0x0400)
        receive();
    if ((temp & 0x0200) == 0x0200)
        printf("\nAMD am79c973 DATA SENT\n");

    registerAddressPort.write(0);
    registerDataPort.write(temp);

    if ((temp & 0x0100) == 0x0100)
        printf("\nAMD am79c973 INIT DONE\n");

    return esp;
}
