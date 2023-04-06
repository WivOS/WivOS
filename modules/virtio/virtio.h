#pragma once

#include <utils/common.h>
#include <modules/modules.h>
#include <devices/pci.h>
#include <cpu/idt.h>

typedef struct {
    volatile uint32_t deviceFeaturesSel;
    volatile uint32_t deviceFeatures;
    volatile uint32_t guestFeaturesSel;
    volatile uint32_t guestFeatures;
    volatile uint16_t msixConfig;
    volatile uint16_t numQueues;
    volatile uint8_t deviceStatus;
    volatile uint8_t configGeneration;

    volatile uint16_t queueSelect;
    volatile uint16_t queueSize;
    volatile uint16_t queueMsixVector;
    volatile uint16_t queueEnable;
    volatile uint16_t queueNotify;
    volatile uint64_t queueDesc;
    volatile uint64_t queueGuest;
    volatile uint64_t queueDevice;
    volatile uint16_t queueNotifyData;
    volatile uint16_t queueReset;
} virtio_registers_t;

#define VIRTIO_DEVICE_ACKNOWLEDGED 0x1
#define VIRTIO_DRIVER_LOADED 0x2
#define VIRTIO_DRIVER_READY 0x4
#define VIRTIO_FEATURES_OK 0x8
#define VIRTIO_DEVICE_ERROR 0x40
#define VIRTIO_DRIVER_FAILED 0x80

typedef struct {
    volatile uint64_t address;
    volatile uint32_t length;
    volatile uint16_t flags;
    volatile uint16_t next;
} virtio_queue_descriptor_t;

typedef struct {
    volatile uint16_t flags;
    volatile uint16_t index;
    volatile uint16_t rings[];
} virtio_queue_available_t;

typedef struct {
    volatile uint32_t index;
    volatile uint32_t length;
} virtio_queue_used_element_t;

typedef struct {
    volatile uint16_t flags;
    volatile uint16_t index;
    volatile virtio_queue_used_element_t rings[];
} virtio_queue_used_t;

typedef struct {
    volatile virtio_queue_descriptor_t *descriptors;
    volatile virtio_queue_available_t *available;
    volatile virtio_queue_used_t *used;
    volatile uint16_t freeList;
    volatile uint16_t freeCount;
    uint16_t queueSize;
    uint16_t queueMask;
    volatile uint16_t lastUsed;

    volatile uint16_t *notify;

    volatile spinlock_t *descriptorsLock;
} virtio_queue_virtqueue_t;

#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4

#define VIRTQ_USED_F_NO_NOTIFY 1

typedef bool (*virtio_event_handler_t)(irq_regs_t *regs, size_t queue, size_t startDescriptor);

void virtio_device_add(pci_device_t *virtioDevice, uint64_t features, virtio_event_handler_t eventHandler);
virtio_registers_t *virtio_get_registers(pci_device_t *virtioDevice);
void *virtio_get_config(pci_device_t *virtioDevice);
void virtio_setup_queue(pci_device_t *virtioDevice, size_t index);

virtio_queue_descriptor_t *virtio_allocate_descriptors(pci_device_t *virtioDevice, size_t queue, size_t count, uint16_t *firstDescriptorIndex);
void virtio_free_descriptor(pci_device_t *virtioDevice, size_t queue, uint16_t descriptorIdx);
void virtio_submit_descriptor(pci_device_t *virtioDevice, size_t queueIdx, uint16_t descriptor, size_t waitDescriptor);

virtio_queue_descriptor_t *virtio_get_next_descriptor(pci_device_t *virtioDevice, size_t queue, uint16_t descriptor);