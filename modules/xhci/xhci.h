#pragma once

#ifndef _USERMODE_

#include <utils/common.h>
#include <modules/modules.h>
#include <devices/pci.h>

#include <tasking/scheduler.h>

#else

typedef size_t kpid_t;
typedef size_t ktid_t;

#endif

typedef struct {
    volatile uint8_t capLength;
    volatile uint8_t rsvd;
    volatile uint16_t hciVersion;
    volatile uint32_t hcsParams1;
    volatile uint32_t hcsParams2;
    volatile uint32_t hcsParams3;
    volatile uint32_t hccParams1;
    volatile uint32_t doorbellOffset;
    volatile uint32_t runtimeSpaceOffset;
    volatile uint32_t hccParams2;
} __attribute__((packed)) xhci_capability_registers_t;

typedef struct {
    volatile uint32_t portStatusControl;
    volatile uint32_t protPwrMgmStatusControl;
    volatile uint32_t portLinkInfo;
    volatile uint32_t portHardwareLPMControl;
} __attribute__((packed, aligned(4))) xhci_usb_port_registers_t;

typedef struct {
    volatile uint32_t usbCommand;
    volatile uint32_t usbStatus;
    volatile uint32_t pageSize;
    volatile uint64_t rsvd0;
    volatile uint32_t deviceNotificationControl;
    volatile uint64_t commandRingControl;
    volatile uint8_t rsvd1[0x10];
    volatile uint64_t deviceContextBaseAddressArrayPointer;
    volatile uint32_t config;
    volatile uint8_t rsvd3[0x400-0x3C];
    volatile xhci_usb_port_registers_t portRegisters[0x100]; //Max 0x100
    //TODO
} __attribute__((packed, aligned(8))) xhci_operational_registers_t;

typedef struct {
    volatile uint32_t interruptManagement;
    volatile uint32_t interruptModeration;
    volatile uint32_t eventRingSegmentTableSize;
    volatile uint32_t rsvd;
    volatile uint64_t eventRingSegmentTableBaseAddress;
    volatile uint64_t eventRingDequeuePointer;
} __attribute__((packed, aligned(8))) xhci_interrupt_registers_t;

typedef struct {
    volatile uint32_t microframeIndex;
    volatile uint8_t rsvd[0x1C];
    volatile xhci_interrupt_registers_t ir[1024];
} __attribute__((packed, aligned(8))) xhci_runtime_registers_t;

typedef struct {
    volatile uint32_t doorbell[0x100];
} __attribute__((packed, aligned(4))) xhci_doorbell_registers_t;

typedef struct {
    volatile uint32_t usbLegacyCapability;
    volatile uint32_t usbLegacyStatus;
} __attribute__((packed)) xhci_legacy_capability_registers_t;

#define HCS1_MAX_DEVICE_SLOTS(register_value) ((register_value) & 0xFF)
#define HCS1_MAX_INTERRUPTERS(register_value) (((register_value) >> 8) & 0x7FF)
#define HCS1_MAX_PORTS(register_value) (((register_value) >> 24) & 0xFF)

#define HCS2_IST(register_value) ((register_value) & 0xF)
#define HCS2_ERST_MAX(register_value) (((register_value) >> 4) & 0xF)
#define HCS2_MAX_SCRATCHPAD_BUFS(register_value) ((((register_value) >> 27) & 0x1F) | ((((register_value) >> 21) & 0x1F) << 5))
#define HCS2_SPR(register_value) (((register_value) >> 26) & 0x1)

#define HCS3_U1_DEVICE_EXIT_LATENCY(register_value) ((register_value) & 0xFF)
#define HC31_U2_DEVICE_EXIT_LATENCY(register_value) (((register_value) >> 8) & 0xFF)

#define HCC1_XECP(register_value) (((register_value) >> 16) & 0xFFFF)
#define HCC1_CONTEXT_SIZE(register_value) ((((register_value) >> 1) & 0x1) ? 64 : 32)

typedef struct {
    uint64_t address;
    uint32_t status;
    uint32_t flags;
} __attribute__((packed)) xhci_event_trb_t;

typedef struct {
    volatile xhci_event_trb_t trb;
    event_t event;
} xhci_event_t;

typedef struct {
    uint64_t address;
    uint32_t length;
    uint32_t flags;
} __attribute__((packed)) xhci_transfer_trb_t;

typedef struct {
    uint32_t field[4];
} __attribute__((packed)) xhci_command_trb_t;

typedef struct {
    uint64_t address;
    uint32_t field2;
    uint32_t field3;
} __attribute__((packed)) xhci_link_trb_t;

typedef union {
    xhci_event_trb_t event;
    xhci_transfer_trb_t transfer;
    xhci_command_trb_t command;
    xhci_link_trb_t link;
} xhci_trb_t;

typedef enum {
    TYPE_CTRL = 0,
    TYPE_BULK,
    TYPE_COMMAND,
    TYPE_EVENT,
} xhci_seg_type_t;

typedef struct xhci_segment {
    xhci_trb_t *trbs;
    uint64_t enqueue;
    uint64_t dequeue;
    uint32_t size;
    uint32_t cycle_state;
    xhci_seg_type_t type;
    xhci_event_t **events;

    struct xhci_segment *next;
    spinlock_t lock;
} xhci_segment_t;

#define TRB_NORMAL           1
#define TRB_SETUP_STAGE      2
#define TRB_DATA_STAGE       3
#define TRB_STATUS_STAGE     4
#define TRB_ISOCH            5
#define TRB_LINK             6
#define TRB_EVENT_DATA       7
#define TRB_NOOP             8
#define TRB_ENABLE_SLOT      9
#define TRB_DISABLE_SLOT     10
#define TRB_ADDRESS_DEV      11
#define TRB_CONFIG_EP        12
#define TRB_EVAL_CNTX        13
#define TRB_TRANSFER_EVENT   32
#define TRB_CMD_COMPLETION   33
#define TRB_PORT_STATUS      34
#define TRB_COMMAND_TYPE(value) ((value & 0x3F) << 10)
#define TRB_STATUS(value) (((value) & (0xFF << 24)) >> 24)
#define TRB_SLOT_ID(value) (((value) & (0xFF << 24)) >> 24)
#define TRB_TYPE(value) (((value) & (0x3F << 10)) >> 10)

typedef struct {
    uint64_t address;
    uint32_t size;
    uint32_t reserved;
} __attribute__((packed, aligned(8))) xhci_erst_entry_t;

typedef struct {
    xhci_erst_entry_t *entries;
    uint32_t segmentCount;
} xhci_erst_t;

typedef struct {
    uint8_t value;
    uint8_t exponent;
    uint8_t type;
    bool fullDuplex;
    uint8_t link_protocol;
    uint16_t mantissa;
} xhci_port_speed_id_t;

typedef struct {
    size_t minor;
    size_t major;
    char name[5];

    uint8_t compatiblePortOffset;
    uint8_t compatiblePortCount;
    uint8_t protocolSpecific;
    uint8_t protocolSlotType;

    xhci_port_speed_id_t speeds[16];
} xhci_port_protocol_t;

typedef struct {
    uint32_t dropFlags;
    uint32_t addFlags;
    uint32_t rsvd[6];
} __attribute__((packed)) xhci_control_context_t;

typedef struct {
    uint32_t field0;
    uint32_t field1;
    uint32_t field2;
    uint32_t field3;
    uint32_t rsvd[4];
} __attribute__((packed)) xhci_slot_context_t;

typedef struct {
    uint32_t field0;
    uint32_t field1;
    uint64_t dequeueAddress;
    uint32_t field3;
    uint32_t rsvd[3];
} __attribute__((packed)) xhci_endpoint_context_t;

typedef struct {
    xhci_segment_t segment;
    uint32_t xEpno;
    spinlock_t lock;
} xhci_endpoint_t;