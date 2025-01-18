#include "virtio.h"

#include <mem/pmm.h>
#include <mem/vmm.h>

typedef struct {
    pci_device_t *pci_device;
    virtio_registers_t *registers;
    volatile uint32_t *isr_status;
    volatile void *notify;
    void *device_config;

    uint32_t notify_multiplier;

    virtio_queue_virtqueue_t **virtual_queues;
    uint64_t activeQueues;

    uint8_t irq;

    virtio_event_handler_t event_handler;
} virtio_device_t;

list_t *virtioDevices;

static virtio_device_t *virtio_get_device(pci_device_t *pciDevice) {
    virtio_device_t *device = NULL;

    spinlock_lock(&virtioDevices->lock);
    foreach(node, virtioDevices) {
        device = (virtio_device_t *)node->value;
        if(device->pci_device->bus == pciDevice->bus &&
           device->pci_device->seg == pciDevice->seg &&
           device->pci_device->dev == pciDevice->dev &&
           device->pci_device->func == pciDevice->func) {
            break;
        }
        device = NULL;
    }
    spinlock_unlock(&virtioDevices->lock);

    return device;
}

void virtio_setup_queue(pci_device_t *virtioDevice, size_t index) {
    virtio_device_t *device = virtio_get_device(virtioDevice);
    if(device == NULL || index >= device->registers->numQueues) return;

    virtio_queue_virtqueue_t *virtqueue = kmalloc(sizeof(virtio_queue_virtqueue_t));

    device->registers->queueSelect = index;
    uint32_t queueSize = device->registers->queueSize;

    size_t descriptorsSize = (sizeof(virtio_queue_descriptor_t) * queueSize);
    size_t availableSize = (3 * sizeof(uint16_t)) + (queueSize * sizeof(uint16_t));
    size_t usedSize = (3 * sizeof(uint16_t)) + (queueSize * sizeof(virtio_queue_used_element_t));
    __attribute__((unused)) size_t pageCount = ROUND_UP((descriptorsSize + availableSize), PAGE_SIZE) + ROUND_UP(usedSize, PAGE_SIZE);

    virtqueue->descriptors = (virtio_queue_descriptor_t *)((uint64_t)pmm_alloc(ROUND_UP(descriptorsSize, PAGE_SIZE) / PAGE_SIZE) + MEM_PHYS_OFFSET);
    virtqueue->available = (virtio_queue_available_t *)((uint64_t)pmm_alloc(ROUND_UP(availableSize, PAGE_SIZE) / PAGE_SIZE) + MEM_PHYS_OFFSET);
    virtqueue->used = (virtio_queue_used_t *)((uint64_t)pmm_alloc(ROUND_UP(usedSize, PAGE_SIZE) / PAGE_SIZE) + MEM_PHYS_OFFSET);

    for(size_t i = 0; i < queueSize; i++) {
        virtqueue->descriptors[i].next = virtqueue->freeList;
        virtqueue->freeList = i;
        virtqueue->freeCount++;
    }

    virtqueue->queueSize = queueSize;
    virtqueue->queueMask = (1 << ((sizeof(queueSize) * 8) - 1 - __builtin_clz(queueSize))) - 1;
    virtqueue->notify = (volatile uint16_t *)(device->notify + device->registers->queueNotify * device->notify_multiplier);
    virtqueue->descriptorsLock = (spinlock_t *)kcalloc(sizeof(spinlock_t), queueSize);
    for(size_t i = 0; i < queueSize; i++) {
        virtqueue->descriptorsLock[i] = INIT_SPINLOCK();
        spinlock_lock(&virtqueue->descriptorsLock[i]); //Lock the descriptor, so when locking again we need to wait until the response is received
    }

    device->registers->queueDesc = ((uint64_t)virtqueue->descriptors - MEM_PHYS_OFFSET);
    device->registers->queueGuest = ((uint64_t)virtqueue->available - MEM_PHYS_OFFSET);
    device->registers->queueDevice = ((uint64_t)virtqueue->used - MEM_PHYS_OFFSET);
    device->registers->queueMsixVector = 0xFFFF;
    device->registers->queueEnable = 0x1;

    device->activeQueues |= (1 << index);

    device->virtual_queues[index] = virtqueue;
}

bool virtio_irq_handler(irq_regs_t *regs, void *_data) {
    foreach(node, virtioDevices) {
        virtio_device_t *device = (virtio_device_t *)node->value;
        if(device->irq == regs->int_no) {
            //Try with this one
            uint32_t irqStatus = *device->isr_status;
            if(irqStatus & 0x1) { //We are handling this one
                for(size_t q = 0; q < device->registers->numQueues; q++) {
                    virtio_queue_virtqueue_t *queue = device->virtual_queues[q];
                    if(queue == NULL || (device->activeQueues & (1 << q)) == 0) continue;

                    size_t startIndex = -1;
                    uint16_t currentIndex = queue->used->index;
                    for(uint16_t i = queue->lastUsed; i != (currentIndex & queue->queueMask); i = (i + 1) & queue->queueMask) {
                        volatile virtio_queue_used_element_t *usedElement = (volatile virtio_queue_used_element_t *)&queue->used->rings[i];

                        uint16_t j = usedElement->index;
                        if(startIndex < 1) startIndex = j;
                        for(;;) {
                            int64_t next;
                            volatile virtio_queue_descriptor_t *descriptor = &queue->descriptors[j];

                            if(descriptor->flags & VIRTQ_DESC_F_NEXT) next = descriptor->next;
                            else next = -1;

                            if(*((volatile uint32_t *)&queue->descriptorsLock[j].lock) == 0) {
                                spinlock_unlock(&queue->descriptorsLock[j]); //Unlock the descriptor to inform

                                if(next < 0) break;

                                queue->descriptors[j].next = queue->freeList;
                                queue->freeList = j;
                                queue->freeCount++;

                                j = next;
                                continue;
                            }

                            queue->descriptors[j].next = queue->freeList;
                            queue->freeList = j;
                            queue->freeCount++;

                            if(next < 0) break;
                            j = next;
                        }

                        queue->lastUsed = (queue->lastUsed + 1) & queue->queueMask;
                    }

                    device->event_handler(regs, q, startIndex);
                }

                return true;
            } else if(device->event_handler(regs, -1, -1)) {
                return true;
            }
        }
    }

    return false;
}

static void virtio_submit_chain(virtio_queue_virtqueue_t *queue, uint16_t descriptor) {
    volatile virtio_queue_available_t *available = queue->available;

    available->rings[available->index & queue->queueMask] = descriptor;
    __asm__ __volatile__ ("" ::: "memory");
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    available->index++;
}

static void virtio_kick_queue(virtio_queue_virtqueue_t *queue, uint16_t queueIndex) {
    *queue->notify = queueIndex;

    __asm__ __volatile__ ("" ::: "memory");
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

void virtio_submit_descriptor(pci_device_t *virtioDevice, size_t queueIdx, uint16_t descriptor, size_t waitDescriptor) {
    virtio_device_t *device = virtio_get_device(virtioDevice);
    if(device == NULL || queueIdx >= device->registers->numQueues) return;

    virtio_queue_virtqueue_t *queue = device->virtual_queues[queueIdx];
    if(queue == NULL || (device->activeQueues & (1 << queueIdx)) == 0) return;

    virtio_submit_chain(queue, descriptor);
    virtio_kick_queue(queue, queueIdx);

    if(waitDescriptor >= 0)
        spinlock_lock(&queue->descriptorsLock[waitDescriptor]);
}

virtio_queue_descriptor_t *virtio_allocate_descriptors(pci_device_t *virtioDevice, size_t queueIdx, size_t count, uint16_t *firstDescriptorIndex) {
    virtio_device_t *device = virtio_get_device(virtioDevice);
    if(device == NULL || queueIdx >= device->registers->numQueues) return NULL;

    virtio_queue_virtqueue_t *queue = device->virtual_queues[queueIdx];
    if(queue == NULL || (device->activeQueues & (1 << queueIdx)) == 0 || queue->freeCount < count) return NULL;

    volatile virtio_queue_descriptor_t *last = NULL;
    uint16_t lastIndex = 0;
    while(count > 0) {
        uint16_t i = queue->freeList;
        volatile virtio_queue_descriptor_t *descriptor = &queue->descriptors[i];

        queue->freeList = descriptor->next;
        queue->freeCount--;

        if(last) {
            descriptor->flags = VIRTQ_DESC_F_NEXT;
            descriptor->next = lastIndex;
        } else {
            descriptor->flags = 0;
            descriptor->next = 0;
        }

        last = descriptor;
        lastIndex = i;
        count--;
    }

    if(firstDescriptorIndex) *firstDescriptorIndex = lastIndex;

    return (virtio_queue_descriptor_t *)last;
}

void virtio_free_descriptor(pci_device_t *virtioDevice, size_t queueIdx, uint16_t descriptorIdx) {
    virtio_device_t *device = virtio_get_device(virtioDevice);
    if(device == NULL || queueIdx >= device->registers->numQueues) return;

    virtio_queue_virtqueue_t *queue = device->virtual_queues[queueIdx];
    if(queue == NULL || (device->activeQueues & (1 << queueIdx)) == 0) return;

    queue->descriptors[descriptorIdx].next = queue->freeList;
    queue->freeList = descriptorIdx;
    queue->freeCount++;
}

virtio_queue_descriptor_t *virtio_get_next_descriptor(pci_device_t *virtioDevice, size_t queueIdx, uint16_t descriptor) {
    virtio_device_t *device = virtio_get_device(virtioDevice);
    if(device == NULL || queueIdx >= device->registers->numQueues) return NULL;

    virtio_queue_virtqueue_t *queue = device->virtual_queues[queueIdx];
    if(queue == NULL || (device->activeQueues & (1 << queueIdx)) == 0) return NULL;

    if(queue->descriptors[descriptor].next >= queue->queueSize) return NULL;

    return (virtio_queue_descriptor_t *)&queue->descriptors[queue->descriptors[descriptor].next];
}

void virtio_init_device(virtio_device_t *device, uint64_t features, virtio_event_handler_t eventHandler) {
    pci_device_t *virtioDevice = device->pci_device;

    pci_enable_busmastering(virtioDevice);

    uint32_t firstBarOffset = 0;
    uint32_t secondBarOffset = 0;
    uint8_t firstBarIndex = 0;
    uint8_t secondBarIndex = 0;

    bool capabilitiesEnabled = pci_device_read_config(virtioDevice, 0x6, sizeof(uint16_t)) & (1 << 4);
    if(capabilitiesEnabled) {
        uint8_t capabilityOffset = pci_device_read_config(virtioDevice, 0x34, sizeof(uint8_t));

        while(capabilityOffset) {
            uint8_t capabilityID = pci_device_read_config(virtioDevice, capabilityOffset, sizeof(uint8_t));
            uint8_t capabilityNext = pci_device_read_config(virtioDevice, capabilityOffset + 1, sizeof(uint8_t));

            if(capabilityID == 0x09) {
                uint8_t capabilityType = pci_device_read_config(virtioDevice, capabilityOffset + 3, sizeof(uint8_t));
                //printf("Capability of type 0x%X found\n", capabilityType);

                switch(capabilityType) {
                    case 0x1:
                        firstBarOffset = pci_device_read_config(virtioDevice, capabilityOffset + 0x8, sizeof(uint32_t));
                        firstBarIndex = pci_device_read_config(virtioDevice, capabilityOffset + 0x4, sizeof(uint32_t));
                        break;
                    case 0x2:
                        secondBarOffset = pci_device_read_config(virtioDevice, capabilityOffset + 0x8, sizeof(uint32_t));
                        secondBarIndex = pci_device_read_config(virtioDevice, capabilityOffset + 0x4, sizeof(uint32_t));
                        device->notify_multiplier = pci_device_read_config(virtioDevice, capabilityOffset + 0x10, sizeof(uint32_t));
                        break;
                    case 0x3: {
                            uint32_t isrBarOffset = pci_device_read_config(virtioDevice, capabilityOffset + 0x8, sizeof(uint32_t));
                            uint8_t isrBarIndex = pci_device_read_config(virtioDevice, capabilityOffset + 0x4, sizeof(uint32_t));
                            pci_bar_t isrBar = {0};
                            if(pci_read_bar(virtioDevice, isrBarIndex, &isrBar)) {
                                device->isr_status = (volatile uint32_t *)(isrBar.base + isrBarOffset + MEM_PHYS_OFFSET);
                                vmm_map(KernelPml4, (void *)(isrBar.base + MEM_PHYS_OFFSET), (void *)isrBar.base, ROUND_UP(isrBar.size, PAGE_SIZE) / PAGE_SIZE, 0x03);
                            }
                        }
                        break;
                    case 0x4: {
                            uint32_t configBarOffset = pci_device_read_config(virtioDevice, capabilityOffset + 0x8, sizeof(uint32_t));
                            uint8_t configBarIndex = pci_device_read_config(virtioDevice, capabilityOffset + 0x4, sizeof(uint32_t));
                            pci_bar_t configBar = {0};
                            if(pci_read_bar(virtioDevice, configBarIndex, &configBar)) {
                                device->device_config = (void *)(configBar.base + configBarOffset + MEM_PHYS_OFFSET);
                                vmm_map(KernelPml4, (void *)(configBar.base + MEM_PHYS_OFFSET), (void *)configBar.base, ROUND_UP(configBar.size, PAGE_SIZE) / PAGE_SIZE, 0x03);
                            }
                        }
                        break;
                }
            }

            capabilityOffset = capabilityNext;
        }
    }

    pci_bar_t virtioBar = {0};
    pci_bar_t virtioNotifyBar = {0};
    pci_read_bar(virtioDevice, firstBarIndex, &virtioBar);
    vmm_map(KernelPml4, (void *)(virtioBar.base + MEM_PHYS_OFFSET), (void *)virtioBar.base, ROUND_UP(virtioBar.size, PAGE_SIZE) / PAGE_SIZE, 0x03);
    pci_read_bar(virtioDevice, secondBarIndex, &virtioNotifyBar);
    vmm_map(KernelPml4, (void *)(virtioNotifyBar.base + MEM_PHYS_OFFSET), (void *)virtioNotifyBar.base, ROUND_UP(virtioNotifyBar.size, PAGE_SIZE) / PAGE_SIZE, 0x03);

    device->registers = (virtio_registers_t *)(virtioBar.base + firstBarOffset + MEM_PHYS_OFFSET);
    device->notify = (void *)(virtioNotifyBar.base + secondBarOffset + MEM_PHYS_OFFSET);

    device->registers->deviceStatus = 0;
    while(device->registers->deviceStatus != 0);

    device->registers->deviceStatus |= VIRTIO_DEVICE_ACKNOWLEDGED;
    device->registers->deviceStatus |= VIRTIO_DRIVER_LOADED;

    device->registers->deviceFeaturesSel = 0x00;
    uint64_t deviceFeatures = device->registers->deviceFeatures;
    device->registers->deviceFeaturesSel = 0x01;
    deviceFeatures |= ((uint64_t)device->registers->deviceFeatures << 32);

    printf("Virtio device features supported: 0x%llx\n", deviceFeatures);

    device->registers->guestFeaturesSel = 0x00;
    device->registers->guestFeatures = (uint32_t)features;
    device->registers->guestFeaturesSel = 0x01;
    device->registers->guestFeatures = (uint32_t)(features >> 32);

    device->registers->deviceStatus |= VIRTIO_FEATURES_OK;
    if(!(device->registers->deviceStatus & VIRTIO_FEATURES_OK)) return; //Not initialized: TODO

    device->registers->deviceStatus |= VIRTIO_DRIVER_READY;

    device->virtual_queues = kcalloc(sizeof(virtio_queue_virtqueue_t *), device->registers->numQueues);
    device->activeQueues = 0;

    device->event_handler = eventHandler;

    device->irq = virtioDevice->gsi + 0x90;
    idt_add_pci_handler(virtioDevice->gsi, virtio_irq_handler, virtioDevice->gsi_flags, NULL);

    printf("Virtio device inited\n");
}

void virtio_device_add(pci_device_t *virtioDevice, uint64_t features, virtio_event_handler_t eventHandler) {
    virtio_device_t *device = kmalloc(sizeof(virtio_device_t));
    device->pci_device = virtioDevice;

    printf("Virtio device added\n");

    virtio_init_device(device, features, eventHandler);

    list_push_back(virtioDevices, device);
}

virtio_registers_t *virtio_get_registers(pci_device_t *virtioDevice) {
    virtio_device_t *device = virtio_get_device(virtioDevice);

    if(device != NULL) {
        return device->registers;
    }

    return NULL;
}

void *virtio_get_config(pci_device_t *virtioDevice) {
    virtio_device_t *device = virtio_get_device(virtioDevice);

    if(device != NULL) {
        return device->device_config;
    }

    return NULL;
}

static size_t _init() {
    virtioDevices = list_create();

    return 1;
}

static size_t _exit() {
    printf("Goodbye :D\n");

    return 1;
}

MODULE_DEF(virtio, _init, _exit);