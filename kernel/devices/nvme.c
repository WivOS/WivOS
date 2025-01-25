#include "nvme.h"

#include "pci.h"

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <fs/devfs/devfs.h>
#include <utils/spinlock.h>

typedef struct {
    volatile uint64_t Capabilities;
    volatile uint32_t Version;
    volatile uint32_t InterruptMaskSet;
    volatile uint32_t InterruptMaskClear;
    volatile uint32_t ControllerConfig;
    volatile uint32_t Reserved1;
    volatile uint32_t ControllerStatus;
    volatile uint32_t Reserved2;
    volatile uint32_t Aqa;
    volatile uint64_t Asq;
    volatile uint64_t Acq;
} __attribute__((packed)) nvme_regs_t;

typedef struct {
    volatile nvme_command_t *command;
    volatile nvme_result_t *result;
    volatile uint32_t *command_db;
    volatile uint32_t *result_db;
    uint16_t size;
    uint16_t cq_vector;
    uint16_t sq_head;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint8_t cq_phase;
    uint16_t qid;
    uint32_t command_id;

    uint64_t *prps;
} nvme_queue_t;

typedef struct {
    uint8_t *cache;
    size_t block;
    size_t end_block;
    uint8_t status;
} cached_block_t;

#define CACHE_NOT_READY 0
#define CACHE_READY 1
#define CACHE_DIRTY 2
#define MAX_CACHED_BLOCKS 512

typedef struct {
    volatile nvme_regs_t *registers;
    size_t stride;
    size_t queue_slots;
    nvme_queue_t queues[2];
    size_t max_prps;
    size_t max_transfer_shift;
    size_t lba_count;
    size_t lba_size;
    size_t cache_block_size;
    size_t cache_overwrite_count;
    cached_block_t *cache;

    volatile spinlock_t lock;
} nvme_device_t;

enum {
    NVME_CONFIG_ENABLE      = 1 << 0,
    NVME_CONFIG_CSS_NVM     = 0 << 4,
    NVME_CONFIG_ARB_RR      = 0 << 11,
    NVME_CONFIG_ARB_WRRU    = 1 << 11,
    NVME_CONFIG_ARB_VS      = 7 << 11,
    NVME_CONFIG_SHN_NONE    = 0 << 14,
    NVME_CONFIG_SHN_NORMAL  = 1 << 14,
    NVME_CONFIG_SHN_ABRUPT  = 2 << 14,
    NVME_CONFIG_SHN_MASK    = 3 << 14,
    NVME_CONFIG_IOSQES      = 6 << 16,
    NVME_CONFIG_IOCQES      = 4 << 20,
    NVME_STATUS_READY       = 1 << 0,
    NVME_STATUS_FATAL_STATUS = 1 << 1,

    NVME_NS_FLBAS_LBA_MASK = 0xF
};

nvme_device_t *NVMEDevices;

static void nvme_initialize_queue(int device, nvme_queue_t *queue, size_t slots, size_t qid) {
    queue->command = (volatile nvme_command_t *)kmalloc(sizeof(nvme_command_t) * slots);
    queue->result = (volatile nvme_result_t *)kmalloc(sizeof(nvme_result_t) * slots);
    queue->command_db = (volatile uint32_t *)((size_t)NVMEDevices[device].registers + PAGE_SIZE + (2 * qid * (4 << NVMEDevices[device].stride)));
    queue->result_db = (volatile uint32_t *)((size_t)NVMEDevices[device].registers + PAGE_SIZE + ((2 * qid + 1) * (4 << NVMEDevices[device].stride)));
    queue->size = slots;
    queue->cq_vector = 0;
    queue->sq_head = 0;
    queue->sq_tail = 0;
    queue->cq_head = 0;
    queue->cq_phase = 1;
    queue->qid = qid;
    queue->command_id = 0;

    queue->prps = kmalloc(NVMEDevices[device].max_prps * slots * sizeof(uint64_t));
}

void nvme_send_command(nvme_queue_t *queue, nvme_command_t command) {
    uint16_t tail = queue->sq_tail;
    queue->command[tail] = command;
    tail++;
    if(tail == queue->size) tail = 0;

    *queue->command_db = tail;
    queue->sq_tail = tail;
}

uint16_t nvme_send_command_and_wait(nvme_queue_t *queue, nvme_command_t command) {
    uint16_t head = queue->cq_head;
    uint16_t phase = queue->cq_phase;

    command.common.command_id = queue->command_id++;
    nvme_send_command(queue, command);

    uint16_t status = 0;
    while(true) {
        status = queue->result[queue->cq_head].status;
        if((status & 0x1) == phase) break;
    }

    status >>= 1;
    if(status) {
        printf("[NVME] Command Error 0x%x\n", status);
        return status;
    }

    head++;
    if(head == queue->size) {
        head = 0;
        queue->cq_phase = !queue->cq_phase;
    }
    *queue->result_db = head;
    queue->cq_head = head;

    return status;
}

bool nvme_identify(int device, nvme_id_ctrl_t *id) {
    size_t length = sizeof(nvme_id_ctrl_t);

    nvme_command_t command = {0};
    command.identify.opcode = 0x06;
    command.identify.nsid = 0;
    command.identify.cns = 1;
    command.identify.prp1 = (size_t)id - MEM_PHYS_OFFSET;

    size_t offset = (size_t)id & (PAGE_SIZE - 1);
    length -= PAGE_SIZE - offset;
    if(length <= 0) command.identify.prp2 = 0;
    else {
        command.identify.prp2 = (size_t)id + (PAGE_SIZE - offset);
    }

    uint16_t status = nvme_send_command_and_wait(&NVMEDevices[device].queues[0], command);
    if(status != 0) return false;

    size_t shift = 12 + (NVMEDevices[device].registers->Capabilities >> 48) & 0xF;

    NVMEDevices[device].max_transfer_shift = id->mdts ? (shift + id->mdts) : 20;

    return true;
}

bool nvme_get_ns_info(int device, int ns, nvme_id_ns_t *id_ns) {
    size_t length = sizeof(nvme_id_ctrl_t);

    nvme_command_t command = {0};
    command.identify.opcode = 0x06;
    command.identify.nsid = ns;
    command.identify.cns = 0;
    command.identify.prp1 = (size_t)id_ns - MEM_PHYS_OFFSET;

    uint16_t status = nvme_send_command_and_wait(&NVMEDevices[device].queues[0], command);
    if(status != 0) return false;

    return true;
}

bool nvme_set_queue_count(int device, int count) {
    nvme_command_t command = {0};
    command.features.opcode = 0x09;
    command.features.fid = 0x7; //Number of queues
    command.features.prp1 = 0;
    command.features.dword11 = ((count - 1) << 16) | (count - 1);

    uint16_t status = nvme_send_command_and_wait(&NVMEDevices[device].queues[0], command);
    if(status != 0) return false;

    return true;
}

bool nvme_create_queue_pair(int device, uint16_t qid) {
    if(!nvme_set_queue_count(device, 4)) return false;
    nvme_initialize_queue(device ,&NVMEDevices[device].queues[1], NVMEDevices[device].queue_slots, 1);

    nvme_command_t cq_command = {0};
    cq_command.create_cq.opcode = 0x05;
    cq_command.create_cq.prp1 = ((size_t)(NVMEDevices[device].queues[1].result)) - MEM_PHYS_OFFSET;
    cq_command.create_cq.cqid = qid;
    cq_command.create_cq.qsize = NVMEDevices[device].queue_slots - 1;
    cq_command.create_cq.cq_flags = 0x1;
    cq_command.create_cq.irq_vector = 0;

    uint16_t status = nvme_send_command_and_wait(&NVMEDevices[device].queues[0], cq_command);
    if(status != 0) return false;

    nvme_command_t sq_command = {0};
    sq_command.create_sq.opcode = 0x01;
    sq_command.create_sq.prp1 = ((size_t)(NVMEDevices[device].queues[1].command)) - MEM_PHYS_OFFSET;
    sq_command.create_sq.sqid = qid;
    sq_command.create_sq.cqid = qid;
    sq_command.create_sq.qsize = NVMEDevices[device].queue_slots - 1;
    sq_command.create_sq.sq_flags = 0x1 | 0x4;

    status = nvme_send_command_and_wait(&NVMEDevices[device].queues[0], sq_command);
    if(status != 0) return false;

    return true;
}

bool nvme_execute_read_write(int device, void *buffer, size_t startLBA, size_t LBACount, bool write) {
    bool usePrpList = false;
    bool usePrp2 = false;

    if((startLBA + LBACount) >= NVMEDevices[device].lba_count) {
        LBACount -= (startLBA - LBACount) - NVMEDevices[device].lba_count;
    }

    uint32_t cmdId = NVMEDevices[device].queues[1].command_id;
    if((LBACount * NVMEDevices[device].lba_size) > PAGE_SIZE) {
        if((LBACount * NVMEDevices[device].lba_size) > (PAGE_SIZE * 2)) {
            size_t prpNum = ((LBACount - 1) * NVMEDevices[device].lba_size) / PAGE_SIZE;
            if(prpNum > NVMEDevices[device].max_prps) {
                printf("[NVME] Max prps exceeded\n");
                return -1;
            }
            for(size_t i = 0; i < prpNum; i++)
                NVMEDevices[device].queues[1].prps[i + cmdId * NVMEDevices[device].max_prps] =
                    ((size_t)(buffer - MEM_PHYS_OFFSET - ((size_t)buffer & (PAGE_SIZE - 1))) + PAGE_SIZE + i * PAGE_SIZE);
            usePrpList = true;
        } else usePrp2 = true;
    }

    nvme_command_t command = {0};
    command.rw.opcode = write ? 0x1 : 0x2;
    command.rw.flags = 0;
    command.rw.nsid = 1;
    command.rw.control = 0;
    command.rw.dsmgm = 0;
    command.rw.eilbrt = 0;
    command.rw.elbat = 0;
    command.rw.elbatm = 0;
    command.rw.metadata = 0;
    command.rw.startLBA = startLBA;
    command.rw.length = LBACount - 1;
    if(usePrpList) {
        command.rw.prp1 = (size_t)((size_t)buffer - MEM_PHYS_OFFSET);
        command.rw.prp2 = (size_t)(&NVMEDevices[device].queues[1].prps[cmdId * NVMEDevices[device].max_prps]) - MEM_PHYS_OFFSET;
    } else if(usePrp2) command.rw.prp2 = (size_t)((size_t)buffer + PAGE_SIZE - MEM_PHYS_OFFSET);
    else command.rw.prp1 = (size_t)((size_t)buffer - MEM_PHYS_OFFSET);

    uint16_t status = nvme_send_command_and_wait(&NVMEDevices[device].queues[1], command);
    if(status != 0) return false;

    return true;
}

static size_t nvme_find_block(int device, size_t block) {
    for(size_t i = 0; i < MAX_CACHED_BLOCKS; i++) {
        if((NVMEDevices[device].cache[i].block == block) && NVMEDevices[device].cache[i].status)
            return i;
    }

    return -1;
}

static size_t nvme_cache_block(int deviceId, size_t block) {
    nvme_device_t *device = &NVMEDevices[deviceId];

    size_t index = 0;
    for(index = 0; index < MAX_CACHED_BLOCKS; index++)
        if(!device->cache[index].status) goto found;

    if(device->cache_overwrite_count == MAX_CACHED_BLOCKS) device->cache_overwrite_count = 0;

    index = device->cache_overwrite_count++;

    if(device->cache[index].status == CACHE_DIRTY) //We are using this now, flush this cache
        if(!nvme_execute_read_write(deviceId, device->cache[index].cache, ((device->cache_block_size / device->lba_size) * device->cache[index].block),
            (device->cache_block_size / device->lba_size), true))
                return -1;

found:
    if(device->cache[index].cache == NULL) device->cache[index].cache = (uint8_t *)kmalloc(device->cache_block_size);

    if(!nvme_execute_read_write(deviceId, device->cache[index].cache, ((device->cache_block_size / device->lba_size) * block),
        (device->cache_block_size / device->lba_size), false))
            return -1;

    device->cache[index].block = block;
    device->cache[index].status = CACHE_READY;

    return index;
}

size_t nvme_read(struct vfs_node *node, char *buffer, size_t size) {
    int deviceId = (int)node->internIndex;
    nvme_device_t *device = &NVMEDevices[deviceId];

    spinlock_lock(&device->lock);

    size_t count = 0;
    while(count < size) {
        size_t sector = (node->offset + count) / device->cache_block_size;
        size_t slot = nvme_find_block(deviceId, sector);
        if(slot == -1) {
            slot = nvme_cache_block(deviceId, sector);
            if(slot == -1) {
                spinlock_unlock(&device->lock);

                return -1;
            }
        }

        size_t chunk = size - count;
        size_t offset = (node->offset + count) % device->cache_block_size;
        if(chunk > (device->cache_block_size - offset))
            chunk = device->cache_block_size - offset;

        memcpy(buffer + count, &device->cache[slot].cache[offset], chunk);
        count += chunk;
    }

    node->offset += size;

    spinlock_unlock(&device->lock);
    return size;
}

size_t nvme_write(struct vfs_node *node, char *buffer, size_t size) {
    int deviceId = (int)node->internIndex;
    nvme_device_t *device = &NVMEDevices[deviceId];

    spinlock_lock(&device->lock);

    size_t count = 0;
    while(count < size) {
        size_t sector = (node->offset + count) / device->cache_block_size;
        size_t slot = nvme_find_block(deviceId, sector);
        if(slot == -1) {
            slot = nvme_cache_block(deviceId, sector);
            if(slot == -1) {
                spinlock_unlock(&device->lock);
                return -1;
            }
        }

        size_t chunk = size - count;
        size_t offset = (node->offset + count) % device->cache_block_size;
        if(chunk > (device->cache_block_size - offset))
            chunk = device->cache_block_size - offset;

        memcpy(&device->cache[slot].cache[offset], buffer + count, chunk);
        device->cache[slot].status = CACHE_DIRTY;
        count += chunk;
    }

    node->offset += size;

    spinlock_unlock(&device->lock);
    return size;
}

size_t nvme_lseek(vfs_node_t *node, size_t offset, size_t type) {
    int deviceId = (int)node->internIndex;
    nvme_device_t *device = &NVMEDevices[deviceId];

    spinlock_lock(&device->lock);

    switch(type) {
        case SEEK_SET:
            if(offset >= (device->lba_count * device->lba_size)) goto def;
            node->offset = offset;
            break;
        case SEEK_END:
            if((((device->lba_count * device->lba_size) + offset) < 0) || offset >= 0) goto def;
            node->offset = (device->lba_count * device->lba_size) + offset;
            break;
        case SEEK_CUR:
            if((node->offset + offset) >= (device->lba_count * device->lba_size) || (node->offset + offset) < 0) goto def;
            node->offset += offset;
            break;
        default:
        def:
            spinlock_unlock(&device->lock);
            return -1;
    }

    spinlock_unlock(&device->lock);
    return node->offset;
}

void nvme_init() {
    int NVMEDeviceCount = 0;
    while(true) {
        pci_device_t *pciDeviceTemp = pci_get_device(0x1, 0x8, 0x2, NVMEDeviceCount);
        if(pciDeviceTemp == NULL) break;
        NVMEDeviceCount++;
    }

    NVMEDevices = (nvme_device_t *)kcalloc(sizeof(nvme_device_t), NVMEDeviceCount);

    for(int deviceIndex = 0; deviceIndex < NVMEDeviceCount; deviceIndex++) {
        pci_device_t *pciDevice = pci_get_device(0x1, 0x8, 0x2, deviceIndex);
        if(pciDevice == NULL) continue;

        pci_bar_t bar;
        if(!pci_read_bar(pciDevice, 0, &bar)) {
            printf("[NVME] Error on BAR #0\n");
            return;
        }

        nvme_device_t device = {0};

        pci_enable_busmastering(pciDevice);

        nvme_regs_t *registers = (nvme_regs_t *)(bar.base + MEM_PHYS_OFFSET);
        vmm_map_if_not_mapped(KernelPml4, (void *)registers, (void *)bar.base, ROUND_UP(bar.size, PAGE_SIZE) / PAGE_SIZE, 0x3);

        pci_enable_mmio(pciDevice);

        device.registers = registers;

        printf("[NVME] Version 0x%X\n", registers->Version);

        //Disable the controller
        if(registers->ControllerConfig & NVME_CONFIG_ENABLE) {
            registers->ControllerConfig &= ~NVME_CONFIG_ENABLE;
        }
        while(registers->ControllerStatus & NVME_STATUS_READY);

        device.queue_slots = registers->Capabilities & 0xFFFF;
        device.stride = (registers->Capabilities >> 32) & 0xF;
        NVMEDevices[deviceIndex] = device;

        nvme_initialize_queue(deviceIndex, &NVMEDevices[deviceIndex].queues[0], NVMEDevices[deviceIndex].queue_slots, 0);

        uint32_t aqa = NVMEDevices[deviceIndex].queue_slots - 1;
        aqa |= aqa << 16;
        aqa |= aqa << 16;
        registers->Aqa = aqa;
        registers->Asq = (size_t)NVMEDevices[deviceIndex].queues[0].command - MEM_PHYS_OFFSET;
        registers->Acq = (size_t)NVMEDevices[deviceIndex].queues[0].result - MEM_PHYS_OFFSET;
        registers->ControllerConfig = NVME_CONFIG_CSS_NVM | NVME_CONFIG_ARB_RR | NVME_CONFIG_SHN_NONE |
                                    NVME_CONFIG_IOSQES | NVME_CONFIG_IOCQES | NVME_CONFIG_ENABLE;

        while(true) {
            uint32_t status = registers->ControllerStatus;
            if(status & NVME_STATUS_READY) break;
            else if(status & NVME_STATUS_FATAL_STATUS) {
                printf("[NVME] Fatal error\n");
                return;
            }
        }

        printf("[NVME] Controller restarted\n");

        nvme_id_ctrl_t *id = (nvme_id_ctrl_t *)kmalloc(sizeof(nvme_id_ctrl_t));
        if(!nvme_identify(deviceIndex, id)) {
            printf("[NVME] Failed to identify device\n");
            return;
        }

        printf("[NVME] Serial number: %.20s\n", id->sn);
        printf("[NVME] Model number: %.40s\n", id->mn);

        kfree(id);

        nvme_id_ns_t *id_ns = (nvme_id_ns_t *)kmalloc(sizeof(nvme_id_ns_t));
        if(!nvme_get_ns_info(deviceIndex, 1, id_ns)) {
            printf("[NVME] Failed to get Namespace info\n");
            return;
        }

        size_t lbaShift = id_ns->lbaf[id_ns->flbas & NVME_NS_FLBAS_LBA_MASK].ds;
        size_t maxLBAs = 1 << (NVMEDevices[deviceIndex].max_transfer_shift - lbaShift);
        NVMEDevices[deviceIndex].max_prps = (maxLBAs * (1 << lbaShift)) / PAGE_SIZE;
        NVMEDevices[deviceIndex].cache_block_size = (maxLBAs * (1 << lbaShift));

        if(!nvme_create_queue_pair(deviceIndex, 1)) {
            printf("[NVME] Failed to create the queue pair\n");
            return;
        }

        NVMEDevices[deviceIndex].lba_size = 1 << id_ns->lbaf[id_ns->flbas & NVME_NS_FLBAS_LBA_MASK].ds;
        NVMEDevices[deviceIndex].lba_count = id_ns->nsize;
        NVMEDevices[deviceIndex].cache = kmalloc(sizeof(cached_block_t) * MAX_CACHED_BLOCKS);
        NVMEDevices[deviceIndex].cache_overwrite_count = 0;
        NVMEDevices[deviceIndex].lock = INIT_SPINLOCK();

        printf("[NVME] Namespace 1 -> LBA Count: %d, LBA Size: %d\n", NVMEDevices[deviceIndex].lba_count, NVMEDevices[deviceIndex].lba_size);

        static const char *devBaseName = "nvme";
        char *devName = kmalloc(strlen((char *)devBaseName) + 2); // 2 just o be sure
        sprintf(devName, "%s%d", devBaseName, deviceIndex);

        devfs_node_t *nvmeNode = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
        strcpy(nvmeNode->name, "NVMEFs");
        nvmeNode->internIndex = deviceIndex;
        nvmeNode->functions.read = nvme_read;
        nvmeNode->functions.write = nvme_write;
        nvmeNode->functions.lseek = nvme_lseek;
        devfs_mount(devName, nvmeNode);

        kfree(devName);
    }
}