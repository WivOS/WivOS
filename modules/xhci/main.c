#include "xhci.h"

#include <tasking/scheduler.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <cpu/cpu.h>

#include <fs/devfs/devfs.h>

#include <utils/lists.h>

#include <devices/timer.h>

#include "../usb/usb.h"

#define _DEBUG 0

typedef struct {
    uint8_t type;
    uint32_t size;
    uint8_t *address;
} __attribute__((packed, aligned(64))) xhci_context_t;

typedef struct {
    void *controller;
    size_t slotID;
    xhci_context_t inContext;
    xhci_context_t outContext;
    xhci_segment_t control;
    uint32_t contextSize;
    xhci_segment_t *epSegments[16];
} xhci_device_t;

typedef struct {
    pci_device_t *pci_device;
    xhci_capability_registers_t *capabilities;
    xhci_operational_registers_t *operational;
    xhci_runtime_registers_t *runtime;
    volatile xhci_doorbell_registers_t *doorbell;
    uint32_t *extendedCapabilities;

    volatile uint64_t *deviceContextBaseAddressArrayPointer;
    volatile uint64_t *scratchpadBufferArray;

    xhci_segment_t commandRingSegment;
    xhci_segment_t eventRing;
    xhci_erst_t erst;

    size_t protocolCount;
    xhci_port_protocol_t portProtocols[255];

    size_t slotID;

    xhci_device_t **devices;

    size_t pci_device_idx;
} xhci_controller_t;

static usb_controller_t *xhciController;

static uint64_t read_aligned_config(void *address, size_t size) {
    size_t alignment = (size_t)address & 0x3;
    address = (void *)((size_t)address & ~alignment);
    switch(size) {
        case 0x1:
            {
                uint32_t value = *(uint32_t *)address;
                return (value >> (alignment * 8)) & 0xFF;
            }
        case 0x2:
            {
                if(alignment <= 2) {
                    uint32_t value = *(uint32_t *)address;
                    return (value >> (alignment * 8)) & 0xFFFF;
                } else {
                    uint32_t value1 = *(uint32_t *)address;
                    uint32_t value2 = *(uint32_t *)((size_t)address + 0x4);
                    return ((value2 & 0xFF) << 24) | ((value1 >> 24) & 0xFF);
                }
            }
        case 0x4:
            if(alignment == 0) {
                return (uint64_t)*(uint32_t *)address;
            }
            return -1;
        case 0x8:
            if(alignment == 0) {
                uint64_t value0 = (uint64_t)*(uint32_t *)address;
                uint64_t value1 = (uint64_t)*(uint32_t *)(address + 0x4);
                return (value1 << 32) | value0;
            }
            return -1;
    }

    return -1;
}

static void write_aligned_config(void *address, uint64_t value, size_t size) {
    size_t alignment = (size_t)address & 0x3;
    address = (void *)((size_t)address & ~alignment);
    switch(size) {
        case 0x1:
            {
                uint32_t value0 = *(uint32_t *)address;
                *(uint32_t *)address = (value0 & (0xFFFFFFFF << ((alignment + 1) * 8))) | ((value & ~(size * 8)) << (alignment * 8));
            }
            break;
        case 0x2:
            {
                if(alignment == 0) {
                    uint32_t value0 = *(uint32_t *)address;
                    *(uint32_t *)address = (value0 & 0xFFFF0000) | (value & 0xFFFF);
                } else {
                    uint32_t value0 = *(uint32_t *)address;
                    *(uint32_t *)address = (value0 & 0xFFFF) | ((value & 0xFFFF) << 16);
                }
            }
            break;
        case 0x4:
            if(alignment == 0) {
                *(uint32_t *)address = value;
            }
            break;
        case 0x8:
            if(alignment == 0) {
                *(uint32_t *)address = value;
                *(uint32_t *)((size_t)address + 0x4) = value >> 32;
            }
            break;
    }
}

#define write_aligned_qword(address, value) write_aligned_config(address, value, 0x8)

list_t *xhci_list = NULL;

static void xhci_take_controll_from_bios(xhci_controller_t *device) {
    uint32_t *epc = device->extendedCapabilities;
    if(HCC1_XECP(device->capabilities->hccParams1) == 0) return;

    while(true) {
        uint32_t legacyCapability = *epc;
        if(legacyCapability == 0xFFFFFFFF) break;

        uint8_t capabilityID = legacyCapability & 0xFF;
        if(capabilityID == 0) break;

        if(capabilityID == 1) {
            volatile uint8_t *bios_sem = (volatile uint8_t *)((size_t)epc + 0x2);
            if(*bios_sem) {
                volatile uint8_t *os_sem = (volatile uint8_t *)((size_t)epc + 0x3);
                *os_sem = 1;
                while(true) {
                    if(*bios_sem == 0) {
                        printf("[XHCI] Not bios owned\n");
                        break;
                    }
                    ksleep(100);
                }
            }
        }

        uint32_t *oldEpc = epc;
        epc = (uint32_t *)((size_t)epc + (((legacyCapability >> 8) & 0xFF) << 2));
        if(oldEpc == epc) break;
    }
}

static void xhci_init_segment(xhci_segment_t *segment, size_t size, xhci_seg_type_t type) {
    segment->trbs = (xhci_trb_t *)kmalloc(size);

    xhci_link_trb_t *link;

    segment->type = type;
    segment->size = size / 16;
    segment->enqueue = (uint64_t)segment->trbs;
    segment->dequeue = (uint64_t)segment->trbs;
    segment->cycle_state = 1;
    segment->next = NULL;
    segment->lock = INIT_SPINLOCK();

    memset((void *)segment->trbs, 0, size);

    if(type != TYPE_EVENT) {
        link = (xhci_link_trb_t *)(segment->trbs + segment->size - 1);
        link->address = (uint64_t)segment->trbs - MEM_PHYS_OFFSET;
        link->field2 = 0;
        link->field3 = 0x1 | TRB_COMMAND_TYPE(TRB_LINK);
        segment->events = kmalloc(sizeof(xhci_event_t) * 4096);
    }
}

bool xhci_irq_handler(irq_regs_t *regs, void *data) {
    xhci_controller_t *device = (xhci_controller_t *)data;

    if(device->operational->usbStatus & (1 << 3)) {
        //Taking interrupt
        uint64_t dequeue = 0;
        volatile xhci_event_trb_t *event = (volatile xhci_event_trb_t *)device->eventRing.dequeue;
        while((event->flags & 0x1) == device->eventRing.cycle_state) {
            if(_DEBUG) printf("[XHCI] Event Dequeue type: 0x%X status: 0x%X\n", TRB_TYPE(event->flags), event->status);

            switch(TRB_TYPE(event->flags)) {
                case 33:
                    {
                        uint64_t index = (event->address - ((uint64_t)device->commandRingSegment.trbs - MEM_PHYS_OFFSET)) / 0x10;
                        if(_DEBUG) printf("[XHCI] Command completion at index 0x%X\n", index);

                        if(device->commandRingSegment.events[index]) {
                            device->commandRingSegment.events[index]->trb = *event;
                            event_notify(&device->commandRingSegment.events[index]->event);
                            device->commandRingSegment.events[index] = NULL;
                        }
                    }
                    break;
                case 0x20:
                    {
                        size_t slotID = (event->flags >> 24) & 0xFF;
                        size_t epid = (event->flags >> 16) & 0x1F;
                        xhci_segment_t *tSegment = NULL;
                        if(epid == 1) tSegment = &device->devices[slotID]->control;
                        else tSegment = device->devices[slotID]->epSegments[epid];

                        uint64_t index = (event->address - ((uint64_t)tSegment->trbs - MEM_PHYS_OFFSET)) / 0x10;
                        if(index == 0xFE) index = (uint64_t)(int64_t)-1;
                        if(_DEBUG) printf("[XHCI] Transfer completion at index %x\n", index + 1);
                        if(tSegment->events[index + 1]) {
                            tSegment->events[index + 1]->trb = *event;
                            event_notify(&tSegment->events[index + 1]->event);
                            tSegment->events[index + 1] = NULL;
                        }
                    }
                    break;
                case 34:
                    {
                        if(_DEBUG) printf("[XHCI] Port 0x%X changed state\n", event->address);
                        if((event->address >> 24) < HCS1_MAX_DEVICE_SLOTS(device->capabilities->hcsParams1)) {
                            //TODO: Implement port notification
                        }
                    }
                    break;
            }

            device->eventRing.dequeue = (uint64_t)(event + 1);
            uint64_t index = device->eventRing.dequeue - (uint64_t)device->eventRing.trbs;
            dequeue = ((uint64_t)device->eventRing.trbs - MEM_PHYS_OFFSET) + (index % 4096);
            if(!(index % 4096)) {
                device->eventRing.dequeue = (uint64_t)device->eventRing.trbs;
                device->eventRing.cycle_state = !device->eventRing.cycle_state;
            }

            event = (xhci_event_trb_t *)device->eventRing.dequeue;
        }

        device->operational->usbStatus |= (1 << 3);
        device->runtime->ir[0].interruptManagement |= 0x1;
        write_aligned_qword((void *)&device->runtime->ir[0].eventRingDequeuePointer, dequeue | (1 << 3));
        return true;
    } else
        return false;
}

static void *xhci_get_trb(volatile xhci_segment_t *segment) {
    uint64_t enqueue = segment->enqueue;
    uint32_t index = (enqueue - (uint64_t)segment->trbs) / 0x10 + 1;
    xhci_link_trb_t *link;

    if(index == (segment->size - 1)) {
        segment->enqueue = (uint64_t)segment->trbs;
        segment->cycle_state ^= segment->cycle_state;
        link = (xhci_link_trb_t *)(segment->trbs + segment->size - 1);
        link->address = ((uint64_t)segment->trbs - MEM_PHYS_OFFSET);
        link->field2 = 0;
        link->field3 = 0x1 | TRB_COMMAND_TYPE(TRB_LINK);
    } else
        segment->enqueue += 16;

    return (void *)enqueue;
}

static void xhci_fill_trb(volatile xhci_command_trb_t *command, uint32_t field0, uint32_t field1, uint32_t field2, uint32_t field3, bool first) {
    command->field[0] = field0;
    command->field[1] = field1;
    command->field[2] = field2;

    if(!first)
        command->field[3] = ((command->field[3] & 0x1) ? 0 : 1) | (field3 & ~0x1);
    else
        command->field[3] = field3;
}

static void xhci_send_command(xhci_controller_t *device, uint32_t field0, uint32_t field1, uint32_t field2, uint32_t field3, xhci_event_t *event) {
    uint32_t index = (device->commandRingSegment.enqueue - (uint64_t)device->commandRingSegment.trbs) / 0x10;

    xhci_command_trb_t *command = xhci_get_trb(&device->commandRingSegment);
    xhci_fill_trb(command, field0, field1, field2, field3, false);
    device->commandRingSegment.events[index] = event;

    *((volatile uint32_t *)&device->doorbell->doorbell[0]) = 0;
}

static bool xhci_enable_slot(xhci_controller_t *device) {
    xhci_event_t event = {0};
    xhci_send_command(device, 0, 0, 0, TRB_COMMAND_TYPE(TRB_ENABLE_SLOT), &event);
    event_await(&event.event);
    if(TRB_STATUS(event.trb.status)) {
        device->slotID = TRB_SLOT_ID(event.trb.flags);
        return true;
    } else {
        printf("[XHCI] Failed to enable slot\n");
        device->slotID = 0;
        return false;
    }
}

static bool xhci_send_address_device(xhci_controller_t *device, uint32_t slotID, uint64_t dmaInContext, bool bsr) {
    xhci_event_t event = {0};
    xhci_send_command(device,
        dmaInContext & ~0xF,
        dmaInContext >> 32,
        0,
        TRB_COMMAND_TYPE(TRB_ADDRESS_DEV) | (slotID << 24) | (bsr << 9),
        &event
    );
    event_await(&event.event);
    if((event.trb.status >> 24) == 1) {
        return true;
    } else {
        printf("[XHCI] Failed to address device: 0x%X\n", TRB_STATUS(event.trb.status));
        return false;
    }
}

static bool xhci_configure_endpoint(xhci_controller_t *device, uint32_t slotID, uint64_t dmaInContext) {
    xhci_event_t event = {0};
    xhci_send_command(device,
        dmaInContext & ~0xF,
        dmaInContext >> 32,
        0,
        TRB_COMMAND_TYPE(TRB_CONFIG_EP) | (slotID << 24),
        &event
    );
    event_await(&event.event);
    if((event.trb.status >> 24) == 1) {
        return true;
    } else {
        printf("[XHCI] Failed to configure endpoint: 0x%X\n", TRB_STATUS(event.trb.status));
        return false;
    }
}

static xhci_control_context_t *xhci_get_control_context(xhci_context_t *context) {
    if(context->type == 0x2)
        return (xhci_control_context_t *)context->address;
    return NULL;
}

static xhci_slot_context_t *xhci_get_slot_context(xhci_context_t *context, uint32_t contextSize) {
    if(context->type == 0x2)
        return (xhci_slot_context_t *)((uint64_t)context->address + contextSize);
    return (xhci_slot_context_t *)(context->address);
}

static xhci_endpoint_context_t *xhci_get_endpoint0_context(xhci_context_t *context, uint32_t contextSize) {
    if(context->type == 0x2)
        return (xhci_endpoint_context_t *)((uint64_t)context->address + contextSize * 2);
    return (xhci_endpoint_context_t *)((uint64_t)context->address + contextSize);
}

static xhci_endpoint_context_t *xhci_get_endpoint_context(xhci_context_t *context, uint32_t contextSize, uint32_t epno) {
    if(context->type == 0x2)
        return (xhci_endpoint_context_t *)((uint64_t)context->address + contextSize * (epno + 1));
    return (xhci_endpoint_context_t *)((uint64_t)context->address + contextSize * epno);
}

bool xhci_send_control(usb_device_t *usbDevice, void *data, usb_request_t request, bool toDevice) {
    xhci_controller_t *device = (xhci_controller_t *)usbDevice->data;
    xhci_device_t *xhciDevice = device->devices[usbDevice->controllerDevNumber];

    xhci_command_trb_t *command = xhci_get_trb(&xhciDevice->control);
    xhci_command_trb_t *first = command;
    uint32_t cycle_state = (command->field[3] & 0x1) ? 0 : 1;

    xhci_fill_trb(
        command,
        request.type | (request.request << 8) | (request.value << 16),
        (request.length << 16) | request.index,
        8,
        ((toDevice ? 2 : 3) << 16) | (1 << 6) | (TRB_SETUP_STAGE << 10) | (1 << 5),
        true
    );

    if(request.length) {
        command = xhci_get_trb(&xhciDevice->control);
        uint64_t address = ((uint64_t)data) - MEM_PHYS_OFFSET;

        xhci_fill_trb(
            command,
            address,
            address >> 32,
            request.length,
            (1 << 2) | (!toDevice << 16) | (TRB_DATA_STAGE << 10) | (1 << 5),
            false
        );
    }

    command = xhci_get_trb(&xhciDevice->control);
    xhci_fill_trb(
        command,
        0,
        0,
        0,
        (1 << 5) | (TRB_STATUS_STAGE << 10),
        false
    );

    uint32_t index = (xhciDevice->control.enqueue - (uint64_t)xhciDevice->control.trbs) / 0x10;
    xhci_event_t event = {0};

    xhciDevice->control.events[index] = &event;
    first->field[3] |= cycle_state;

    device->doorbell->doorbell[xhciDevice->slotID] = 1;

    event_await(&event.event);
    return (event.trb.status >> 24) == 1;
}

bool xhci_send_bulk(usb_device_t *usbDevice, char *data, size_t size, uint32_t epno, uint32_t out) {
    (void)out;

    xhci_controller_t *device = (xhci_controller_t *)usbDevice->data;
    xhci_device_t *xhciDevice = device->devices[usbDevice->controllerDevNumber];

    xhci_segment_t *segment = xhciDevice->epSegments[epno];
    spinlock_lock(&segment->lock);

    xhci_transfer_trb_t *transfer = xhci_get_trb(segment);
    xhci_fill_trb(
        (volatile xhci_command_trb_t *)transfer,
        (uint32_t)((uint64_t)data - MEM_PHYS_OFFSET),
        (uint32_t)(((uint64_t)data - MEM_PHYS_OFFSET) >> 32),
        size,
        TRB_COMMAND_TYPE(TRB_NORMAL) | (1 << 2) | (1 << 5) | (0 << 4),
        false
    );

    uint32_t index = (segment->enqueue - (uint64_t)segment->trbs) / 0x10;
    xhci_event_t event = {0};

    segment->events[index] = &event;
    device->doorbell->doorbell[usbDevice->controllerDevNumber] = epno;

    event_await(&event.event);
    transfer->address = 0;
    transfer->length = 0;
    transfer->flags = 0;

    spinlock_unlock(&segment->lock);
    return (event.trb.status >> 24) == 1;
}

size_t xhci_setup_endpoint(usb_device_t *usbDevice, uint32_t address, uint32_t attributes, uint32_t maxPacket) {
    xhci_controller_t *device = (xhci_controller_t *)usbDevice->data;
    xhci_device_t *xhciDevice = device->devices[usbDevice->controllerDevNumber];

    xhci_endpoint_t *endpoint = kmalloc(sizeof(xhci_endpoint_t));
    endpoint->lock = INIT_SPINLOCK();

    volatile xhci_slot_context_t *outSlot = xhci_get_slot_context(&xhciDevice->outContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));
    volatile xhci_slot_context_t *inSlot = xhci_get_slot_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));

    inSlot->field0 = outSlot->field0;
    inSlot->field1 = outSlot->field1;
    inSlot->field2 = outSlot->field2;
    inSlot->field3 = outSlot->field3;

    xhci_endpoint_context_t *endpoint0ContextOut = xhci_get_endpoint0_context(&xhciDevice->outContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));
    xhci_endpoint_context_t *endpoint0ContextIn = xhci_get_endpoint0_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));

    endpoint0ContextIn->field0 = endpoint0ContextOut->field0;
    endpoint0ContextIn->field1 = endpoint0ContextOut->field1;
    endpoint0ContextIn->dequeueAddress = endpoint0ContextOut->dequeueAddress;
    endpoint0ContextIn->field3 = endpoint0ContextOut->field3;

    bool out = (address & 0x80) ? false : true;

    xhci_segment_t newSegment;
    xhci_init_segment(&newSegment, 4096, TYPE_BULK);

    uint32_t xEpno = 2 * (address & 0xF) + ((!out) ? 1 : 0);

    inSlot->field0 &= ~(0x1F << 27);
    inSlot->field0 |= (xEpno + 1) << 27;

    uint8_t transferType = attributes & 0x3;

    //TODO: Handle other endpoints
    if(out) {
        xhci_control_context_t *control = xhci_get_control_context(&xhciDevice->inContext);
        xhci_endpoint_context_t *endpointContext = xhci_get_endpoint_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1), xEpno);
        endpointContext->field1 = (transferType << 3) | (3 << 1) | (maxPacket << 16);

        endpoint->segment = newSegment;
        endpoint->xEpno = xEpno;

        endpointContext->dequeueAddress = ((uint64_t)endpoint->segment.trbs - MEM_PHYS_OFFSET) | endpoint->segment.cycle_state;
        endpointContext->field3 = 8;
        control->addFlags = ((1 << xEpno) + 1) | 0x1;
        control->dropFlags = 0;
        xhci_configure_endpoint(device, xhciDevice->slotID, (uint64_t)xhciDevice->inContext.address - MEM_PHYS_OFFSET);

        xhciDevice->epSegments[xEpno] = &endpoint->segment;
        return xEpno;
    } else {
        xhci_control_context_t *control = xhci_get_control_context(&xhciDevice->inContext);
        xhci_endpoint_context_t *endpointContext = xhci_get_endpoint_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1), xEpno);
        endpointContext->field1 = ((transferType + 4) << 3) | (3 << 1) | (maxPacket << 16);

        endpoint->segment = newSegment;
        endpoint->xEpno = xEpno;

        endpointContext->dequeueAddress = ((uint64_t)endpoint->segment.trbs - MEM_PHYS_OFFSET) | endpoint->segment.cycle_state;
        endpointContext->field3 = 8;
        control->addFlags = ((1 << xEpno) + 1) | 0x1;
        control->dropFlags = 0;
        xhci_configure_endpoint(device, xhciDevice->slotID, (uint64_t)xhciDevice->inContext.address - MEM_PHYS_OFFSET);

        xhciDevice->epSegments[xEpno] = &endpoint->segment;
        return xEpno;
    }
}

static bool xhci_init_device(xhci_controller_t *device, uint32_t port, xhci_port_protocol_t protocol) {
    //Enable
    xhci_enable_slot(device);
    if(!device->slotID) return false;

    xhci_device_t *xhciDevice = kmalloc(sizeof(xhci_device_t));
    xhciDevice->slotID = device->slotID;

    xhciDevice->inContext.address = (uint8_t *)kmalloc(4096);
    xhciDevice->inContext.type = 0x2; //INPUT
    xhciDevice->inContext.size = 4096;

    xhci_control_context_t *control = xhci_get_control_context(&xhciDevice->inContext);
    control->addFlags = 0x3;
    control->dropFlags = 0;

    xhci_slot_context_t *slot = xhci_get_slot_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));
    slot->field0 = (1 << 27) | (0x4 << 20);
    slot->field1 = ((port + 1) & 0xFF) << 16;

    xhci_endpoint_context_t *endpoint0 = xhci_get_endpoint0_context(&xhciDevice->inContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));

    volatile xhci_usb_port_registers_t *portRegisters = &device->operational->portRegisters[port];
    uint8_t portSpeed = (portRegisters->portStatusControl >> 10) & 0xF;
    uint16_t maxPacket = 0;
    switch(portSpeed) {
        case 1:
            printf("[XHCI] Full speed not supported\n");
            return false;
        case 2: maxPacket = 8; break;
        case 3: maxPacket = 64; break;
        case 4:
        case 5:
        case 6:
        case 7: maxPacket = 512; break;
    }

    endpoint0->field1 = (4 << 3) | (0 << 8) | (3 << 1) | (maxPacket << 16);
    endpoint0->field3 = 8;
    xhci_init_segment(&xhciDevice->control, 4096, TYPE_CTRL);
    endpoint0->dequeueAddress = ((uint64_t)xhciDevice->control.trbs - MEM_PHYS_OFFSET) | xhciDevice->control.cycle_state;

    xhciDevice->outContext.address = (uint8_t *)kmalloc(4096);
    xhciDevice->outContext.type = 0x1; //DEVICE
    xhciDevice->outContext.size = 4096;

    device->deviceContextBaseAddressArrayPointer[device->slotID] = (uint64_t)xhciDevice->outContext.address - MEM_PHYS_OFFSET;
    slot = xhci_get_slot_context(&xhciDevice->outContext, HCC1_CONTEXT_SIZE(device->capabilities->hccParams1));

    xhci_send_address_device(device, device->slotID, (uint64_t)xhciDevice->inContext.address - MEM_PHYS_OFFSET, false);
    control->addFlags = 0x1;
    xhci_configure_endpoint(device, device->slotID, (uint64_t)xhciDevice->inContext.address - MEM_PHYS_OFFSET);

    usb_device_t usbDevice = {0};
    usbDevice.maxPacketSize = 512;
    usbDevice.controller = xhciController;
    usbDevice.data = device;
    device->devices[device->slotID] = xhciDevice;

    if(!usb_add_device(usbDevice, device->slotID))
        return false;
    return true;
}

static void xhci_get_port_speeds(xhci_controller_t *device) {
    uint32_t *epc = device->extendedCapabilities;
    if(HCC1_XECP(device->capabilities->hccParams1) == 0) return;

    printf("Speeds: \n");

    while(true) {
        uint32_t capability = *epc;
        if(capability == 0xFFFFFFFF) break;

        uint8_t capabilityID = capability & 0xFF;
        if(capabilityID == 0) break;

        uint32_t *oldEpc = epc;
        if(capabilityID == 2) {
            uint32_t value = *epc++;

            xhci_port_protocol_t protocol = {0};
            protocol.minor = (value >> 16) & 0xFF;
            protocol.major = (value >> 24) & 0xFF;

            value = *epc++;
            protocol.name[0] = (char)(value & 0xFF);
            protocol.name[1] = (char)((value >> 8) & 0xFF);
            protocol.name[2] = (char)((value >> 16) & 0xFF);
            protocol.name[3] = (char)((value >> 24) & 0xFF);
            protocol.name[4] = 0;

            value = *epc++;
            protocol.compatiblePortOffset = value & 0xFF;
            protocol.compatiblePortCount = (value >> 8) & 0xFF;
            protocol.protocolSpecific = (value >> 16) & 0xFF;

            size_t speedIdCount = (value >> 28) & 0xF;

            value = *epc++;
            protocol.protocolSlotType = value & 0xF;

            for(size_t i = 0; i < speedIdCount; i++) {
                value = *epc++;

                protocol.speeds[i].value = value & 0xF;
                protocol.speeds[i].exponent = (value >> 4) & 0x3;
                protocol.speeds[i].type = (value >> 6) & 0x3;
                protocol.speeds[i].fullDuplex = (value >> 8) & 0x1;
                protocol.speeds[i].link_protocol = (value >> 14) & 0x3;
                protocol.speeds[i].mantissa = (value >> 16) & 0xFFFF;
            }

            printf("[XHCI] Speed version %d.%d, name: %s, offset: 0x%X, count = 0x%X\n", protocol.major, protocol.minor, protocol.name, protocol.compatiblePortOffset, protocol.compatiblePortCount);

            epc = oldEpc;
            device->portProtocols[device->protocolCount] = protocol;
            device->protocolCount++;
        }

        epc = (uint32_t *)((size_t)epc + (((capability >> 8) & 0xFF) << 2));
        if(oldEpc == epc) break;
    }
}

static void xhci_detect_devices(xhci_controller_t *device) {
    xhci_get_port_speeds(device);

    for(size_t i = 0; i < device->protocolCount; i++) {
        for(size_t j = device->portProtocols[i].compatiblePortOffset; (j - device->portProtocols[i].compatiblePortOffset) < device->portProtocols[i].compatiblePortCount; j++) {
            volatile xhci_usb_port_registers_t *portRegisters = &device->operational->portRegisters[j - 1];

            if(!(portRegisters->portStatusControl & (1 << 9))) {
                portRegisters->portStatusControl = 1 << 9;
                ksleep(20);
                if(!(portRegisters->portStatusControl & (1 << 9))) continue;
            }

            portRegisters->portStatusControl = (1 << 9) | ((1 << 17) | (1 << 18) | (1 << 20) | (1 << 21) | (1 << 22));

            if(device->portProtocols[i].major == 2) portRegisters->portStatusControl = (1 << 9) | (1 << 4);
            else portRegisters->portStatusControl = (1 << 9) | (1 << 31);

            size_t timeout = 0;
            bool reset = false;
            while(true) {
                if(portRegisters->portStatusControl & (1 << 21)) {
                    reset = true;
                    break;
                }
                if(timeout++ == 100) break;
                ksleep(1);
            }
            if(!reset) continue;

            ksleep(3);

            if(portRegisters->portStatusControl & (1 << 1)) {
                portRegisters->portStatusControl = (1 << 9) | ((1 << 17) | (1 << 18) | (1 << 20) | (1 << 21) | (1 << 22));

                printf("Initing xhci device on port %d protocol %d\n", j, i);
                xhci_init_device(device, j - 1, device->portProtocols[i]);
            }
        }
    }
}

static bool init_xhci_device(xhci_controller_t *device, size_t device_index) {
    printf("[XHCI] Activating XHCI Controller #%d\n", device_index);

    pci_device_t *xhciDevice = device->pci_device;

    pci_enable_busmastering(xhciDevice);

    pci_bar_t bar0 = {0};
    pci_read_bar(xhciDevice, 0, &bar0);

    uint64_t baseConfigAddress = bar0.base;
    baseConfigAddress += MEM_PHYS_OFFSET;
    vmm_map(KernelPml4, (void *)baseConfigAddress, (void *)bar0.base, ROUND_UP(bar0.size, PAGE_SIZE) / PAGE_SIZE, 0x3);

    device->capabilities = (xhci_capability_registers_t *)(baseConfigAddress);
    device->operational = (xhci_operational_registers_t *)(baseConfigAddress + device->capabilities->capLength);
    device->runtime = (xhci_runtime_registers_t *)(baseConfigAddress + device->capabilities->runtimeSpaceOffset);
    device->doorbell = (xhci_doorbell_registers_t *)(baseConfigAddress + device->capabilities->doorbellOffset);
    device->extendedCapabilities = (uint32_t *)(baseConfigAddress + HCC1_XECP(device->capabilities->hccParams1) * 4);

    device->devices = (xhci_device_t **)kmalloc(sizeof(xhci_device_t *) * HCS1_MAX_DEVICE_SLOTS(device->capabilities->hcsParams1));

    printf("[XHCI] Version: %x\n", read_aligned_config((void *)&device->capabilities->hciVersion, sizeof(device->capabilities->hciVersion)));

    xhci_take_controll_from_bios(device);

    //Switch ports
    pci_device_write_config(xhciDevice, 0xD8, pci_device_read_config(xhciDevice, 0xDC, 0x4), 0x4);
    pci_device_write_config(xhciDevice, 0xD0, pci_device_read_config(xhciDevice, 0xD4, 0x4), 0x4);

    //TODO: MSI
    idt_add_pci_handler(xhciDevice->gsi, xhci_irq_handler, xhciDevice->gsi_flags, device);

    //Stop commands
    device->operational->usbCommand |= 0x2;
    ksleep(100);
    while(device->operational->usbCommand & 0x1);
    while(!(device->operational->usbStatus & 0x1));

    device->operational->config = HCS1_MAX_DEVICE_SLOTS(device->capabilities->hcsParams1);

    device->deviceContextBaseAddressArrayPointer = (volatile uint64_t *)kmalloc(2048);
    write_aligned_qword((void *)&device->operational->deviceContextBaseAddressArrayPointer, ((uint64_t)device->deviceContextBaseAddressArrayPointer - MEM_PHYS_OFFSET));

    uint64_t scratchpad_bufs = HCS2_MAX_SCRATCHPAD_BUFS(device->capabilities->hcsParams2);
    if(scratchpad_bufs) {
        device->scratchpadBufferArray = kmalloc(sizeof(uint64_t) * scratchpad_bufs);
        for(size_t i = 0; i < scratchpad_bufs; i++) {
            device->scratchpadBufferArray[i] = (uint64_t)kmalloc(PAGE_SIZE) - MEM_PHYS_OFFSET;
        }
    }

    device->deviceContextBaseAddressArrayPointer[0] = (uint64_t)device->scratchpadBufferArray - MEM_PHYS_OFFSET;

    xhci_init_segment(&device->commandRingSegment, 4096, TYPE_COMMAND);
    xhci_init_segment(&device->eventRing, 4096, TYPE_EVENT);

    write_aligned_qword((void *)&device->operational->commandRingControl, ((uint64_t)device->commandRingSegment.trbs - MEM_PHYS_OFFSET) | 0x1);
    device->erst.entries = (xhci_erst_entry_t *)kmalloc(4096);
    device->erst.segmentCount = 1;
    device->erst.entries->address = ((uint64_t)device->eventRing.trbs - MEM_PHYS_OFFSET);
    device->erst.entries->size = device->eventRing.size;
    device->erst.entries->reserved = 0;

    uint64_t eventRingDequeuePointer = read_aligned_config((void *)&device->runtime->ir[0].eventRingDequeuePointer, 0x8) & 0xFUL;
    eventRingDequeuePointer |= ((uint64_t)device->eventRing.trbs - MEM_PHYS_OFFSET) & ~(uint64_t)0xFUL;
    write_aligned_qword((void *)&device->runtime->ir[0].eventRingDequeuePointer, eventRingDequeuePointer);
    device->runtime->ir[0].interruptManagement = 0x2;

    uint32_t eventRingSegmentTableSize = device->runtime->ir[0].eventRingSegmentTableSize & ~(uint32_t)0xFFFF;
    eventRingSegmentTableSize |= device->erst.segmentCount;
    device->runtime->ir[0].eventRingSegmentTableSize = eventRingSegmentTableSize;

    uint64_t eventRingSegmentTableBaseAddress = read_aligned_config((void *)&device->runtime->ir[0].eventRingSegmentTableBaseAddress, 0x8) & 0x3FUL;
    eventRingSegmentTableBaseAddress |= ((uint64_t)device->erst.entries - MEM_PHYS_OFFSET) & ~(uint64_t)0x3FUL;
    write_aligned_qword((void *)&device->runtime->ir[0].eventRingSegmentTableBaseAddress, eventRingSegmentTableBaseAddress);

    device->operational->usbCommand |= 0x5;

    while(device->operational->usbStatus & 0x1);

    xhci_detect_devices(device);

    printf("[XHCI] Activated XHCI Controller #%d %x\n", device_index);

    return true;
}

static size_t _init() {
    xhci_list = list_create();

    xhciController = kmalloc(sizeof(usb_controller_t));
    xhciController->send_control = xhci_send_control;
    xhciController->send_bulk = xhci_send_bulk;
    xhciController->setup_endpoint = xhci_setup_endpoint;

    size_t idx = 0;
    while(true) {
        pci_device_t *device = pci_get_device(0xC, 0x3, 0x30, idx);
        if(!device) break;

        xhci_controller_t *usbDevice = (xhci_controller_t *)kmalloc(sizeof(xhci_controller_t));
        usbDevice->pci_device = device;
        usbDevice->pci_device_idx = idx;

        if(!init_xhci_device(usbDevice, idx)) {
            kfree(usbDevice);
            break;
        }

        list_push_back(xhci_list, device);

        idx++;
    }
    return 1;
}

static size_t _exit() {
    printf("Goodbye :D\n");

    return 1;
}

MODULE_DEF(xhci, _init, _exit);