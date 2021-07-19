#include "pcnet.h"
#include <acpi/apic.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <proc/smp.h>
#include <fs/devfs/devfs.h>

static pci_device_t *pcnetDevice = NULL;
static volatile uint64_t ioBase;

static size_t txBufferPtr = 0;
static size_t rxBufferPtr = 0;

#define txBufferCnt 8
#define rxBufferCnt 32

#define bufferSize 1548
#define deSize 16

static uint8_t *tdes;
static uint8_t *rdes;

static uint64_t rxBuffers;
static uint64_t txBuffers;

//Interface values
static volatile uint8_t firstPacketReceived = 0;
static volatile int64_t lastPacketHandled = 0;

void writeIndex(uint32_t value) {
    outl(ioBase + 0x14, value);
}

uint32_t readCsr32(uint32_t csr) {
    writeIndex(csr);
    return inl(ioBase + 0x10);
}

uint32_t readBcr32(uint32_t csr) {
    writeIndex(csr);
    return inl(ioBase + 0x1C);
}

void writeCsr32(uint32_t csr, uint32_t value) {
    writeIndex(csr);
    outl(ioBase + 0x10, value);
}

void writeBcr32(uint32_t csr, uint32_t value) {
    writeIndex(csr);
    outl(ioBase + 0x1C, value);
}

uint8_t doesIOwnBuffer(uint8_t *desc, size_t idx) {
    return (desc[deSize * idx + 7] & 0x80) == 0;
}

size_t getNextTxIdx(size_t idx) {
    size_t ret = idx + 1;
    if(ret == txBufferCnt) ret = 0;
    return ret;
}

size_t getNextRxIdx(size_t idx) {
    size_t ret = idx + 1;
    if(ret == rxBufferCnt) ret = 0;
    return ret;
}

void initDe(uint8_t *desc, size_t idx, uint8_t isTx) {
    memset(&desc[idx * deSize], 0, deSize);

    uint64_t bufAddr = rxBuffers;
    if(isTx) bufAddr = txBuffers;
    *(uint32_t *)&desc[idx * deSize] = bufAddr + idx * bufferSize;

    uint16_t bcnt = (uint16_t)(-bufferSize);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t *)&desc[idx * deSize + 4] = bcnt;

    if(!isTx) desc[idx * deSize + 7] = 0x80;
}

/*static void pcnet_handle_packet(uint8_t *buffer, size_t length) {
    for(size_t i = 0; i < length; i++) {
        if(i % 0x10 == 0) printf("\n");
        else printf(", ");
        printf("%02X", buffer[i]);
    }
    printf("\n");
}*/

static uint16_t getRxPacketLength(size_t idx) {
    return *(uint16_t *)&rdes[idx * deSize + 8];
}

static void *getRxBuffer(size_t idx) {
    return (void *)((rxBuffers + VIRT_PHYS_BASE) + idx * bufferSize);
}

static void pcnet_irq_handler(irq_regs_t *regs) {
    uint32_t status = readCsr32(0);
    if(!(status & (1 << 7))) return; //Other device

    while(doesIOwnBuffer(rdes, rxBufferPtr)) {
        uint16_t plen = getRxPacketLength(rxBufferPtr);

        void *pbuf = getRxBuffer(rxBufferPtr);

        firstPacketReceived = 1;

        //handle packet
        //pcnet_handle_packet(pbuf, plen);

        rdes[rxBufferPtr * deSize + 7] = 0x80;
        rxBufferPtr = getNextRxIdx(rxBufferPtr);

        if(rxBufferPtr == 0) {
            lastPacketHandled = -1;
        }
    }

    writeCsr32(0, readCsr32(0) | 0x400);
}

size_t sendPacket(void *packet, size_t length, uint8_t *dest) {
    if(!doesIOwnBuffer(tdes, txBufferPtr)) {
        return 0;
    }

    memcpy((void *)((txBuffers + VIRT_PHYS_BASE) + txBufferPtr * bufferSize), packet, length);

    tdes[txBufferPtr * deSize + 7] |= 0x3;

    uint16_t bcnt = (uint16_t)(-length);
    bcnt &= 0x0FFF;
    bcnt |= 0xF000;
    *(uint16_t *)&tdes[txBufferPtr * deSize + 4] = bcnt;

    tdes[txBufferPtr * deSize + 7] |= 0x80;

    writeCsr32(0, readCsr32(0) | 0x8);

    txBufferPtr = getNextTxIdx(txBufferPtr);

    return 1;
}

//utilities
static int64_t getReceivedPacketIndex() {
    if(firstPacketReceived && lastPacketHandled < +rxBufferPtr) {
        if(lastPacketHandled == -1) return rxBufferCnt - 1;
        else return lastPacketHandled;
    } else {
        return -1;
    }
}

static void waitUntilReceived() {
    while(getReceivedPacketIndex() == -1) { //Wait to receive response
        asm volatile("hlt");
    }
}

static uint16_t htons(uint16_t v) {
  return (v >> 8) | (v << 8);
}
static uint32_t htonl(uint32_t v) {
  return htons(v >> 16) | (htons((uint16_t) v) << 16);
}

static int _init() {
    printf("Initing PcNet\n");

    pcnetDevice = pci_get_device_by_vendor(0x1022, 0x2000);
    if(pcnetDevice) {
        printf("[PCNET] TODO\n");

        uint16_t config = pci_read_word(pcnetDevice, 0x4);
        pci_write_word(pcnetDevice, 0x4, config | 0x5);

        pci_bar_t bar0;
        pci_read_bar(pcnetDevice, 0, &bar0);
        ioBase = bar0.base;

        //Reset
        inl(ioBase + 0x18);
        inw(ioBase + 0x14);
        ksleep(100);

        outl(ioBase + 0x10, 0); //Enable DWIO
        ksleep(100);

        //Connect the interrupt handler
        connectDeviceToPin(pcnetDevice->gsi, pcnet_irq_handler, pcnetDevice->gsiFlags);
        //irq_functions[0x32] = (idt_fn_t)pcnet_irq_handler;
        //lapic_connect_gsi_to_vec(0, 0x52, pcnetDevice->gsi, pcnetDevice->gsiFlags, 1);

        uint32_t csr58 = readCsr32(58);
        csr58 &= 0xFFF0;
        csr58 |= 0x2;
        writeCsr32(58, csr58);

        uint32_t bcr2 = readBcr32(2);
        bcr2 |= 0x2;
        writeBcr32(2, bcr2);

        //Alloc the buffers
        void *lrxBufferPtr = (void *)kmalloc(bufferSize * rxBufferCnt);
        void *ltxBufferPtr = (void *)kmalloc(bufferSize * txBufferCnt);

        rxBuffers = ((uint64_t)lrxBufferPtr - VIRT_PHYS_BASE);
        txBuffers = ((uint64_t)ltxBufferPtr - VIRT_PHYS_BASE);

        rdes = (uint8_t *)kmalloc(deSize * rxBufferCnt);
        for(size_t i = 0; i < rxBufferCnt; i++) {
            initDe(rdes, i, 0);
        }

        tdes = (uint8_t *)kmalloc(deSize * txBufferCnt);
        for(size_t i = 0; i < txBufferCnt; i++) {
            initDe(tdes, i, 1);
        }

        config_bytes_t *configBytes = (config_bytes_t *)kmalloc(sizeof(config_bytes_t));
        memset(configBytes, 0x00, sizeof(config_bytes_t));

        configBytes->mode = 0;
        configBytes->tlen = 3 << 4;
        configBytes->rlen = 5 << 4;
        configBytes->mac[0] = 0xAE;
        configBytes->mac[1] = 0x00;
        configBytes->mac[2] = 0xF0;
        configBytes->mac[3] = 0xBA;
        configBytes->mac[4] = 0xCA;
        configBytes->mac[5] = 0xFE;

        configBytes->physRx = (uint32_t)((uint64_t)rdes - VIRT_PHYS_BASE);
        configBytes->physTx = (uint32_t)((uint64_t)tdes - VIRT_PHYS_BASE);

        uint32_t configBytesPhysAddr = (uint32_t)((uint64_t)configBytes - VIRT_PHYS_BASE);
        writeCsr32(1, configBytesPhysAddr & 0xFFFF);
        writeCsr32(2, (configBytesPhysAddr >> 16) & 0xFFFF);

        printf("Initialicing...\n");

        //TODO: interesting configs, pads, etc
        uint16_t csr3 = readCsr32(3);
        csr3 &= ~0x4;
        csr3 &= ~0x400;
        writeCsr32(3, csr3 | 0x300); //Disable done interrupt

        writeCsr32(4, readCsr32(4) | 0x5801);
        writeCsr32(0, readCsr32(0) | 0x41); //Start and enable interrupts

        while(!(readCsr32(0) & 0x100));
        
        uint16_t csr0 = readCsr32(0);
        csr0 &= ~0x5;
        writeCsr32(0, csr0 | 0x2);
        ksleep(100);

        uint8_t routerMac[6];
        {
            ethernet_header_t *ethHeader = (ethernet_header_t *)kmalloc(sizeof(ethernet_header_t) + sizeof(arp_header_t));
            memset(ethHeader, 0x00, sizeof(ethernet_header_t) + sizeof(arp_header_t));

            ethHeader->ethType = htons(0x0806);
            ethHeader->srcMac.n[0] = 0xAE;
            ethHeader->srcMac.n[1] = 0x00;
            ethHeader->srcMac.n[2] = 0xF0;
            ethHeader->srcMac.n[3] = 0xBA;
            ethHeader->srcMac.n[4] = 0xCA;
            ethHeader->srcMac.n[5] = 0xFE;
            arp_header_t *arpHeader = (arp_header_t *)((uint64_t)ethHeader + sizeof(ethernet_header_t));

            arpHeader->htype = htons(0x1);
            arpHeader->ptype = htons(0x0800);
            arpHeader->hlen = 6;
            arpHeader->plen = 4;
            arpHeader->opcode = htons(0x0001);

            arpHeader->srcHw[0] = 0xAE;
            arpHeader->srcHw[1] = 0x00;
            arpHeader->srcHw[2] = 0xF0;
            arpHeader->srcHw[3] = 0xBA;
            arpHeader->srcHw[4] = 0xCA;
            arpHeader->srcHw[5] = 0xFE;

            arpHeader->srcPr[0] = 10;
            arpHeader->srcPr[1] = 0;
            arpHeader->srcPr[2] = 2;
            arpHeader->srcPr[3] = 5;

            arpHeader->dstPr[0] = 10;
            arpHeader->dstPr[1] = 0;
            arpHeader->dstPr[2] = 2;
            arpHeader->dstPr[3] = 2;

            sendPacket(ethHeader, sizeof(ethernet_header_t) + sizeof(arp_header_t), NULL);

            kfree(ethHeader);

            waitUntilReceived();

            ethernet_header_t *respEthHeader = getRxBuffer(getReceivedPacketIndex());
            uint16_t respLength = getRxPacketLength(getReceivedPacketIndex());

            for(size_t i = 0; i < respLength; i++) {
                if(i % 0x10 == 0) printf("\n");
                else printf(", ");
                printf("%02X", ((uint8_t *)respEthHeader)[i]);
            }
            printf("\n");

            memcpy(routerMac, respEthHeader->srcMac.n, 6);

            lastPacketHandled++;
        }

        {
            ethernet_header_t *ethHeader = (ethernet_header_t *)kmalloc(sizeof(ethernet_header_t) + sizeof(arp_header_t));
            memset(ethHeader, 0x00, sizeof(ethernet_header_t) + sizeof(arp_header_t));

            ethHeader->ethType = htons(0x0806);
            ethHeader->srcMac.n[0] = 0xAE;
            ethHeader->srcMac.n[1] = 0x00;
            ethHeader->srcMac.n[2] = 0xF0;
            ethHeader->srcMac.n[3] = 0xBA;
            ethHeader->srcMac.n[4] = 0xCA;
            ethHeader->srcMac.n[5] = 0xFE;
            arp_header_t *arpHeader = (arp_header_t *)((uint64_t)ethHeader + sizeof(ethernet_header_t));

            arpHeader->htype = htons(0x1);
            arpHeader->ptype = htons(0x0800);
            arpHeader->hlen = 6;
            arpHeader->plen = 4;
            arpHeader->opcode = htons(0x0001);

            arpHeader->srcHw[0] = 0xAE;
            arpHeader->srcHw[1] = 0x00;
            arpHeader->srcHw[2] = 0xF0;
            arpHeader->srcHw[3] = 0xBA;
            arpHeader->srcHw[4] = 0xCA;
            arpHeader->srcHw[5] = 0xFE;

            arpHeader->srcPr[0] = 10;
            arpHeader->srcPr[1] = 0;
            arpHeader->srcPr[2] = 2;
            arpHeader->srcPr[3] = 5;

            arpHeader->dstPr[0] = 8;
            arpHeader->dstPr[1] = 8;
            arpHeader->dstPr[2] = 8;
            arpHeader->dstPr[3] = 8;

            sendPacket(ethHeader, sizeof(ethernet_header_t) + sizeof(arp_header_t), NULL);

            kfree(ethHeader);

            waitUntilReceived();

            ethernet_header_t *respEthHeader = getRxBuffer(getReceivedPacketIndex());
            uint16_t respLength = getRxPacketLength(getReceivedPacketIndex());

            for(size_t i = 0; i < respLength; i++) {
                if(i % 0x10 == 0) printf("\n");
                else printf(", ");
                printf("%02X", ((uint8_t *)respEthHeader)[i]);
            }
            printf("\n");

            lastPacketHandled++;
        }

        printf("Initialization done\n");

        return 0;
    }

    return 1;
}

static int _exit() {
    return 0;
}

MODULE_DEF(pcnet, _init, _exit);
