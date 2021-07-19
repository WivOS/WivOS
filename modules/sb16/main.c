#include "sb16.h"
#include <acpi/apic.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <proc/smp.h>
#include <fs/devfs/devfs.h>
#include "../sound/sound.h"

#define RESET_REG 0x0
#define MASTER_VOL_REG 0x2
#define MIC_VOL_REG 0xE
#define PCM_VOL_REG 0x18
#define REC_SLC_REG 0x1A
#define REC_GAIN_REG 0x1C
#define MIC_GAIN_REG 0x1E
#define EXT_ID_REG 0x28
#define EXT_CTRL_REG 0x2A
#define EXT_FRONT_RATE_REG 0x2C

#define PCM_INPUT_REGISTERS 0x00
#define PCM_OUTPUT_REGISTERS 0x10
#define MICOPHONE_INPUT_REGISTERS 0x20
#define GLOBAL_CTL 0x2C
#define GLOBAL_STS 0x30

#define BUFFER_DESC_ADDR_REG 0x00
#define CUR_ENTRY_VAL_REG 0x04
#define LAST_VALID_ENTRY_REG 0x05
#define TRANSFER_STS_REG 0x06
#define CUR_IDX_PROC_SAMPLES_REG 0x08
#define PRCSD_ENTRY_REG 0x0A
#define BUFFER_CNT_REG 0x0B

static pci_device_t *sb16Device = NULL;
static uint64_t nabmAddr;
static uint64_t namAddr;

uint8_t *data;
uint16_t *audioSample;
static uint8_t *bufferDescriptorList;
static uint32_t currIndex = 0;
static void irq_handler(irq_regs_t *regs) {
    if(!(inw(nabmAddr + (PCM_OUTPUT_REGISTERS | TRANSFER_STS_REG)) & 0x1C)) return; //Another device interrupted

    outw(nabmAddr + (PCM_OUTPUT_REGISTERS | TRANSFER_STS_REG), 0x1C);

    currIndex++;

    outl(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_DESC_ADDR_REG), (uint32_t)((uint64_t)(bufferDescriptorList + (0x8 * 0x20 * currIndex)) - VIRT_PHYS_BASE));
    outb(nabmAddr + (PCM_OUTPUT_REGISTERS | CUR_ENTRY_VAL_REG), 0x0);
    outb(nabmAddr + (PCM_OUTPUT_REGISTERS | LAST_VALID_ENTRY_REG), 0x1F);
    //outb(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_CNT_REG), 0x19);
}


unsigned char sample_440[] = {
  0x0f, 0x00, 0x1f, 0x00, 0xe0, 0x02, 0xc0, 0x05, 0xfb, 0x05, 0xf2, 0x0b,
  0xb7, 0x08, 0x74, 0x11, 0xc3, 0x0b, 0x81, 0x17, 0x7d, 0x0e, 0x00, 0x1d,
  0x61, 0x11, 0xbc, 0x22, 0x13, 0x14, 0x28, 0x28, 0xc3, 0x16, 0x86, 0x2d,
  0x61, 0x19, 0xc3, 0x32, 0xde, 0x1b, 0xbb, 0x37, 0x55, 0x1e, 0xac, 0x3c,
  0x9f, 0x20, 0x38, 0x41, 0xdd, 0x22, 0xc2, 0x45, 0xf2, 0x24, 0xdd, 0x49,
  0xf1, 0x26, 0xe5, 0x4d, 0xc6, 0x28, 0x8e, 0x51, 0x81, 0x2a, 0xfc, 0x54,
  0x11, 0x2c, 0x2a, 0x58, 0x7f, 0x2d, 0xf7, 0x5a, 0xc8, 0x2e, 0x93, 0x5d,
  0xe2, 0x2f, 0xc4, 0x5f, 0xdf, 0x30, 0xbc, 0x61, 0xa5, 0x31, 0x4d, 0x63,
  0x4d, 0x32, 0x98, 0x64, 0xc2, 0x32, 0x87, 0x65, 0x10, 0x33, 0x1c, 0x66,
  0x33, 0x33, 0x69, 0x66, 0x26, 0x33, 0x4a, 0x66, 0xf5, 0x32, 0xe9, 0x65,
  0x8e, 0x32, 0x21, 0x65, 0x0a, 0x32, 0x0e, 0x64, 0x4d, 0x31, 0xa1, 0x62,
  0x74, 0x30, 0xdf, 0x60, 0x66, 0x2f, 0xd2, 0x5e, 0x35, 0x2e, 0x6a, 0x5c,
  0xe0, 0x2c, 0xbe, 0x59, 0x5e, 0x2b, 0xbe, 0x56, 0xbe, 0x29, 0x77, 0x53,
  0xf3, 0x27, 0xed, 0x4f, 0x0e, 0x26, 0x16, 0x4c, 0x02, 0x24, 0x0a, 0x48,
  0xdd, 0x21, 0xb5, 0x43, 0x98, 0x1f, 0x32, 0x3f, 0x37, 0x1d, 0x6f, 0x3a,
  0xc2, 0x1a, 0x82, 0x35, 0x31, 0x18, 0x63, 0x30, 0x8e, 0x15, 0x1d, 0x2b,
  0xda, 0x12, 0xb3, 0x25, 0x12, 0x10, 0x27, 0x20, 0x43, 0x0d, 0x80, 0x1a,
  0x5f, 0x0a, 0xc5, 0x14, 0x7e, 0x07, 0xf5, 0x0e, 0x89, 0x04, 0x1a, 0x09,
  0xa0, 0x01, 0x39, 0x03, 0xa5, 0xfe, 0x50, 0xfd, 0xbb, 0xfb, 0x6f, 0xf7,
  0xc6, 0xf8, 0x94, 0xf1, 0xe3, 0xf5, 0xbf, 0xeb, 0x02, 0xf3, 0x09, 0xe6,
  0x2c, 0xf0, 0x56, 0xe0, 0x6a, 0xed, 0xd1, 0xda, 0xac, 0xea, 0x5e, 0xd5,
  0x0e, 0xe8, 0x16, 0xd0
};

static int _init() {
    printf("Initing SoundBlaster 16\n");

    sb16Device = pci_get_device_by_vendor(0x8086, 0x2415);
    if(sb16Device) {
        printf("[SB16] TODO\n");

        pci_bar_t bar0;
        pci_read_bar(sb16Device, 0, &bar0);
        namAddr = bar0.base;

        pci_bar_t bar1;
        pci_read_bar(sb16Device, 1, &bar1);
        nabmAddr = bar1.base;
        
        /*irq_functions[0x2F] = (idt_fn_t)irq_handler;

        lapic_connect_gsi_to_vec(0, 0x4F, sb16Device->gsi, sb16Device->gsiFlags, 1);*/
        connectDeviceToPin(sb16Device->gsi, irq_handler, sb16Device->gsiFlags);

        uint16_t commandRegister = pci_read_word(sb16Device, 0x4);
        pci_write_word(sb16Device, 0x4, 0x5);

        //Cold reset + Enable Interrupts
        outl(nabmAddr + GLOBAL_CTL, 0x3);
        ksleep(100);

        //Reset the register default values
        outw(namAddr + RESET_REG, 0x55AA);
        ksleep(100);

        outw(namAddr + MASTER_VOL_REG, 0x0000);
        outw(namAddr + PCM_VOL_REG, 0x0);
        ksleep(100);

        vfs_node_t *wav = kopen("/test2.wav", 0);
        
        uint8_t *wavHeader = kmalloc(12);
        vfs_read(wav, (char *)wavHeader, 12);
        uint32_t chunkHeader[2];
        while(true) {
            vfs_read(wav, (char *)chunkHeader, 8);
            if(!memcmp(chunkHeader, "data", 4)) break;
            wavHeader = krealloc(wavHeader, chunkHeader[1]);
            vfs_read(wav, (char *)wavHeader, chunkHeader[1]);
        }

        uint32_t size = chunkHeader[1];

        uint32_t bufferDescs = (size / 0xFFFF);
        if((bufferDescs * 0xFFFF) != size) bufferDescs++;
        bufferDescs /= 2;

        data = (uint8_t *)kmalloc(0xFFFF * bufferDescs * 2);
        vfs_read(wav, (char *)data, 0xFFFF * bufferDescs * 2);

        bufferDescriptorList = (uint8_t *)kmalloc(8 * bufferDescs); //Well fornow it's 1
        memset(bufferDescriptorList, 0, 8 * bufferDescs);
        uint32_t dataAddr = (uint32_t)((uint64_t)(data) - VIRT_PHYS_BASE);
        for(uint32_t i = 0; i < bufferDescs; i++) {
            *((uint32_t *)&bufferDescriptorList[0x0 + (i * 0x8)]) = dataAddr;
            *((uint16_t *)&bufferDescriptorList[0x4 + (i * 0x8)]) = 0xFFFE;
            *((uint16_t *)&bufferDescriptorList[0x6 + (i * 0x8)]) = (i == (bufferDescs - 1)) ? 0xC000 : ((i & 0x1F) == 0x1F) ? 0x8000 : 0x0000;
            dataAddr += (0xFFFF * 2);
        }
        
        /**((uint32_t *)&bufferDescriptorList[0x0]) = dataAddr;
        *((uint16_t *)&bufferDescriptorList[0x4]) = 0xFFFE;
        *((uint16_t *)&bufferDescriptorList[0x6]) = 0x0000;
        *((uint32_t *)&bufferDescriptorList[0x0+0x8]) = dataAddr + (0xFFFF * 2);
        *((uint16_t *)&bufferDescriptorList[0x4+0x8]) = 0xFFFE;
        *((uint16_t *)&bufferDescriptorList[0x6+0x8]) = 0x0000;
        *((uint32_t *)&bufferDescriptorList[0x0+0x10]) = dataAddr + (0xFFFF * 2) + (0xFFFF * 2);
        *((uint16_t *)&bufferDescriptorList[0x4+0x10]) = 0xFFFE;
        *((uint16_t *)&bufferDescriptorList[0x6+0x10]) = 0x0000;
        *((uint32_t *)&bufferDescriptorList[0x0+0x18]) = dataAddr + (0xFFFF * 2) + (0xFFFF * 2) + (0xFFFF * 2);
        *((uint16_t *)&bufferDescriptorList[0x4+0x18]) = 0xFFFE;
        *((uint16_t *)&bufferDescriptorList[0x6+0x18]) = 0x0000;
        *((uint32_t *)&bufferDescriptorList[0x0+0x20]) = dataAddr + (0xFFFF * 2) + (0xFFFF * 2) + (0xFFFF * 2) + (0xFFFF * 2);
        *((uint16_t *)&bufferDescriptorList[0x4+0x20]) = 0xFFFE;
        *((uint16_t *)&bufferDescriptorList[0x6+0x20]) = 0xC000;*/

        outb(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_CNT_REG), 0x1A);
        while(inb(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_CNT_REG)) & 0x2);
        outl(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_DESC_ADDR_REG), (uint32_t)((uint64_t)(bufferDescriptorList) - VIRT_PHYS_BASE));
        ksleep(100);
        
        outb(nabmAddr + (PCM_OUTPUT_REGISTERS | LAST_VALID_ENTRY_REG), 0x1F);
        ksleep(100);

        outb(nabmAddr + (PCM_OUTPUT_REGISTERS | BUFFER_CNT_REG), 0x19);

        /*sound_device_t *sndDevice = (sound_device_t *)kmalloc(sizeof(sound_device_t));
        register_sound_device(sndDevice);*/ //TODO

        return 0;
    }

    return 1;
}

static int _exit() {
    return 0;
}

MODULE_DEF(sb16, _init, _exit);
