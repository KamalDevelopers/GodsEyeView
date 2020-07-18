#include "amd79.hpp"

AmdDriver::AmdDriver(PCIcontrollerDeviceDescriptor* dev, InterruptManager* interrupts)
    : Driver()
    , InterruptHandler(interrupts, dev->interrupt + interrupts->HardwareInterruptOffset())
    , MACAddress0Port(dev->portBase)
    , MACAddress2Port(dev->portBase + 0x02)
    , MACAddress4Port(dev->portBase + 0x04)
    , registerDataPort(dev->portBase + 0x10)
    , registerAddressPort(dev->portBase + 0x12)
    , resetPort(dev->portBase + 0x14)
    , busControlRegisterDataPort(dev->portBase + 0x16)
{
    currentSendBuffer = 0;
    currentRecvBuffer = 0;

    uint64_t MAC0 = MACAddress0Port.Read() % 256;
    uint64_t MAC1 = MACAddress0Port.Read() / 256;
    uint64_t MAC2 = MACAddress2Port.Read() % 256;
    uint64_t MAC3 = MACAddress2Port.Read() / 256;
    uint64_t MAC4 = MACAddress4Port.Read() % 256;
    uint64_t MAC5 = MACAddress4Port.Read() / 256;

    uint64_t MAC = MAC5 << 40
        | MAC4 << 32
        | MAC3 << 24
        | MAC2 << 16
        | MAC1 << 8
        | MAC0;

    registerAddressPort.Write(20);
    busControlRegisterDataPort.Write(0x102);

    registerAddressPort.Write(0);
    registerDataPort.Write(0x04);

    initBlock.mode = 0x0000;
    initBlock.reserved1 = 0;
    initBlock.numSendBuffers = 3;
    initBlock.reserved2 = 0;
    initBlock.numRecvBuffers = 3;
    initBlock.physicalAddress = MAC;
    initBlock.reserved3 = 0;
    initBlock.logicalAddress = 0;

    sendBufferDescr = (BufferDescriptor*)((((uint32_t)&sendBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.sendBufferDescrAddress = (uint32_t)sendBufferDescr;
    recvBufferDescr = (BufferDescriptor*)((((uint32_t)&recvBufferDescrMemory[0]) + 15) & ~((uint32_t)0xF));
    initBlock.recvBufferDescrAddress = (uint32_t)recvBufferDescr;

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

    registerAddressPort.Write(1);
    registerDataPort.Write((uint32_t)(&initBlock) & 0xFFFF);
    registerAddressPort.Write(2);
    registerDataPort.Write(((uint32_t)(&initBlock) >> 16) & 0xFFFF);
}

AmdDriver::~AmdDriver()
{
}

void AmdDriver::Send(uint8_t* buffer, int size)
{
    int sendDescriptor = currentSendBuffer;
    currentSendBuffer = (currentSendBuffer + 1) % 8;

    if (size > 1518)
        size = 1518;

    for (uint8_t *src = buffer + size - 1,
                 *dst = (uint8_t*)(sendBufferDescr[sendDescriptor].address + size - 1);
         src >= buffer; src--, dst--)
        *dst = *src;

    sendBufferDescr[sendDescriptor].avail = 0;
    sendBufferDescr[sendDescriptor].flags2 = 0;
    sendBufferDescr[sendDescriptor].flags = 0x8300F000
        | ((uint16_t)((-size) & 0xFFF));
    registerAddressPort.Write(0);
    registerDataPort.Write(0x48);
}

void AmdDriver::Receive()
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

            for (int i = 0; i < size; i++) {
                printf("%x", buffer[i]);
                printf("%s", " ");
            }
        }

        recvBufferDescr[currentRecvBuffer].flags2 = 0;
        recvBufferDescr[currentRecvBuffer].flags = 0x8000F7FF;
    }
}

void AmdDriver::Activate()
{
    registerAddressPort.Write(0);
    registerDataPort.Write(0x41);

    registerAddressPort.Write(4);
    uint32_t temp = registerDataPort.Read();
    registerAddressPort.Write(4);
    registerDataPort.Write(temp | 0xC00);

    registerAddressPort.Write(0);
    registerDataPort.Write(0x42);
}

int AmdDriver::Reset()
{
    resetPort.Read();
    resetPort.Write(0);
    return 10;
}

uint32_t AmdDriver::HandleInterrupt(uint32_t esp)
{
    registerAddressPort.Write(0);
    uint32_t temp = registerDataPort.Read();

    if ((temp & 0x0400) == 0x0400)
        Receive();

    registerAddressPort.Write(0);
    registerDataPort.Write(temp);
    return esp;
}