#include "pci.h"

#include <mem/pmm.h>

#include <lai/helpers/pci.h>

#include <utils/lists.h>
#include "nvme.h"

#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

typedef struct pci_extended_bus_holder {
    uint16_t size;
    pci_config_alloc_t entries[];
} pci_extended_bus_holder_t;

static mcfg_t *MCFGTable = NULL;
static pci_extended_bus_holder_t *PCIEHolder = NULL;

static list_t *PCIDevices;

void pci_preinit() {
    MCFGTable = acpi_get_table("MCFG", 0);
    if(MCFGTable) {
        int n = (MCFGTable->sdt.length - sizeof(sdt_t) - 8) / 16;
        PCIEHolder = (pci_extended_bus_holder_t *)kmalloc(sizeof(pci_config_alloc_t) * n + sizeof(pci_extended_bus_holder_t));
        PCIEHolder->size = n;

        for(int i = 0; i < n; i++) {
            pci_config_alloc_t *alloc = &MCFGTable->configs[i];
            PCIEHolder->entries[alloc->seg] = *alloc;
        }
    }
}

static void pci_select_address(uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset) {
    uint32_t address = 0x80000000 | ((uint64_t)bus << 16) |
                       ((uint32_t)(dev & 0x1F) << 11) |
                       ((uint32_t)(func & 0x7) << 8) |
                       (uint32_t)(offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
}

void pci_write_config(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset,
                      uint32_t value, uint8_t writeSize) {
    if(PCIEHolder) {
        for(int i = 0; i < PCIEHolder->size; i++) {
            pci_config_alloc_t *entry = &PCIEHolder->entries[i];
            if(entry->seg == seg && entry->startBus <= bus && entry->endBus >= bus) {
                void *virtAddr = (void *)(MEM_PHYS_OFFSET + entry->base + ((uint64_t)(bus - entry->startBus) << 20 | dev << 15 | func << 12) + offset);
                switch(writeSize) {
                    case 1: *((uint8_t *)virtAddr) = (uint8_t)value; break;
                    case 2: *((uint16_t *)virtAddr) = (uint16_t)value; break;
                    case 4: *((uint32_t *)virtAddr) = (uint32_t)value; break;
                }

                return;
            }
        }

        printf("PCI Device not found\n");
        return;
    }
    pci_select_address(bus, dev, func, offset);

    uint32_t currentData = (writeSize != 4) ? inl(PCI_CONFIG_DATA) : 0;

    switch(writeSize) {
        case 1:
            currentData &= ~(0xFF << ((offset & 0x3) * 8));
            currentData |= (value & 0xFF) << ((offset & 0x3) * 8);
            break;
        case 2:
            currentData &= ~(0xFFFF << ((offset & 0x2) * 8));
            currentData |= (value & 0xFFFF) << ((offset & 0x2) * 8);
            break;
        case 4:
            currentData = value;
            break;
    }

    outl(PCI_CONFIG_DATA, currentData);
}

uint32_t pci_read_config(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset,
                         uint8_t readSize) {
    if(PCIEHolder) {
        for(int i = 0; i < PCIEHolder->size; i++) {
            pci_config_alloc_t *entry = &PCIEHolder->entries[i];
            if(entry->seg == seg && entry->startBus <= bus && entry->endBus >= bus) {
                void *virtAddr = (void *)(MEM_PHYS_OFFSET + entry->base + ((uint64_t)(bus - entry->startBus) << 20 | dev << 15 | func << 12) + offset);
                switch(readSize) {
                    case 1: return *((uint8_t *)virtAddr);
                    case 2: return *((uint16_t *)virtAddr);
                    case 4: return *((uint32_t *)virtAddr);
                }
            }
        }

        printf("PCI Device not found or read size undefined\n");
        return 0;
    }
    pci_select_address(bus, dev, func, offset);

    uint32_t currentData = inl(PCI_CONFIG_DATA);

    switch(readSize) {
        case 1:
            currentData >>= (offset & 0x3) * 8;
            currentData &= 0xFF;
            break;
        case 2:
            currentData >>= (offset & 0x2) * 8;
            currentData &= 0xFFFF;
            break;
    }

    return currentData;
}

void pci_device_write_config(pci_device_t *device, uint16_t offset,
                      uint32_t value, uint8_t writeSize) {
    pci_write_config(device->seg, device->bus, device->dev, device->func, offset, value, writeSize);
}
uint32_t pci_device_read_config(pci_device_t *device, uint16_t offset,
                         uint8_t readSize) {
    return pci_read_config(device->seg, device->bus, device->dev, device->func, offset, readSize);
}

static void pci_check_bus(uint16_t seg, uint8_t bus, size_t parent);

static lai_nsnode_t *pci_determine_root_bus_node(uint16_t seg, uint8_t bus, lai_state_t *state) {
    LAI_CLEANUP_VAR lai_variable_t pciPnpId = LAI_VAR_INITIALIZER;
    LAI_CLEANUP_VAR lai_variable_t pciePnpId = LAI_VAR_INITIALIZER;
    lai_eisaid(&pciPnpId, "PNP0A03");
    lai_eisaid(&pciePnpId, "PNP0A08");

    lai_nsnode_t *sbHandle = lai_resolve_path(NULL, "\\_SB_");

    struct lai_ns_child_iterator iterator = LAI_NS_CHILD_ITERATOR_INITIALIZER(sbHandle);
    lai_nsnode_t *currentNode;
    while((currentNode = lai_ns_child_iterate(&iterator))) {
        if(lai_check_device_pnp_id(currentNode, &pciPnpId, state) && lai_check_device_pnp_id(currentNode, &pciePnpId, state)) {
            continue;
        }

        LAI_CLEANUP_VAR lai_variable_t busNumber = LAI_VAR_INITIALIZER;
        uint64_t bbnResult = 0;
        lai_nsnode_t *bbnHandle = lai_resolve_path(currentNode, "_BBN");
        if(bbnHandle) {
            if(lai_eval(&busNumber, bbnHandle, state)) continue;
            lai_obj_get_integer(&busNumber, &bbnResult);
        }

        LAI_CLEANUP_VAR lai_variable_t segNumber = LAI_VAR_INITIALIZER;
        uint64_t segResult = 0;
        lai_nsnode_t *segHandle = lai_resolve_path(currentNode, "_SEG");
        if(segHandle) {
            if(lai_eval(&segNumber, segHandle, state)) continue;
            lai_obj_get_integer(&segNumber, &segResult);
        }

        if(segResult == seg && bbnResult == bus) return currentNode;
    }

    return NULL;
}

static void pci_determine_acpi_node(pci_device_t *device, lai_state_t *state) {
    if(device->acpiNode) return;

    lai_nsnode_t *node = NULL;
    if(device->parent != -1) { //Not the bus
        pci_device_t *parent = (pci_device_t *)list_get_indexed(PCIDevices, device->parent)->value;
        if(!parent) return;

        if(!parent->acpiNode) pci_determine_acpi_node(parent, state);

        node = parent->acpiNode;
    } else {
        node = pci_determine_root_bus_node(device->seg, device->bus, state);
    }

    if(!node) return;

    device->acpiNode = lai_pci_find_device(node, device->dev, device->func, state);
}

static void pci_check_function(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t func, size_t parent) {
    pci_device_t *device = (pci_device_t *)kmalloc(sizeof(pci_device_t));
    device->seg = seg;
    device->bus = bus;
    device->dev = dev;
    device->func = func;

    uint32_t deviceAndVendorId = pci_read_config(seg, bus, dev, func, 0x0, sizeof(uint32_t));
    if((deviceAndVendorId & 0xFFFF) == 0xFFFF) {
        kfree(device);
        return;
    }

    uint32_t config8 = pci_device_read_config(device, 0x8, sizeof(uint32_t));
    uint32_t configC = pci_device_read_config(device, 0xC, sizeof(uint32_t));
    uint32_t config3C = pci_device_read_config(device, 0x3C, sizeof(uint32_t));

    device->parent = parent;
    device->device_id = deviceAndVendorId >> 16;
    device->vendor_id = deviceAndVendorId;
    device->rev_id = config8;
    device->subclass = config8 >> 16;
    device->device_class = config8 >> 24;
    device->prog_if = config8 >> 8;
    device->irq_pin = config3C >> 8;

    device->multifunction = configC & 0x800000;

    size_t id = PCIDevices->length;
    list_push_back(PCIDevices, device);

    if(device->device_class == 0x06 && device->subclass == 0x04) {
        LAI_CLEANUP_STATE lai_state_t laiState;
        lai_init_state(&laiState);

        pci_determine_acpi_node(device, &laiState);

        if(device->acpiNode) {
            lai_nsnode_t *prtHandle = lai_resolve_path(device->acpiNode, "_PRT");

            if(prtHandle) device->has_prt = !lai_eval(&device->acpiPrt, prtHandle, &laiState);
        }

        uint32_t config18 = pci_device_read_config(device, 0x18, sizeof(uint32_t));
        pci_check_bus(seg, config18 >> 8, id);
    }
}

static void pci_check_bus(uint16_t seg, uint8_t bus, size_t parent) {
    for(uint8_t dev = 0; dev < 32; dev++) {
        for(uint8_t func = 0; func < 8; func++) {
            pci_check_function(seg, bus, dev, func, parent);
        }
    }
}

void pci_enable_busmastering(pci_device_t *device) {
    uint32_t cfg = pci_device_read_config(device, 0x4, sizeof(uint32_t));
    if(!(cfg & (1 << 2))) {
        pci_device_write_config(device, 0x4, cfg | (1 << 2), sizeof(uint32_t));
    }
}

void pci_set_interrupts(pci_device_t *device, bool enable) {
    uint32_t cfg = pci_device_read_config(device, 0x4, sizeof(uint32_t));
    if(enable)
        pci_device_write_config(device, 0x4, cfg & ~(1 << 10), sizeof(uint32_t));
    else
        pci_device_write_config(device, 0x4, cfg | (1 << 10), sizeof(uint32_t));
}

void pci_enable_mmio(pci_device_t *device) {
    pci_device_write_config(device, 0x4, pci_device_read_config(device, 0x4, sizeof(uint32_t)) | (1 << 1), sizeof(uint32_t));
}

pci_device_t *pci_get_device(uint8_t class, uint8_t subclass, uint8_t prog_if, size_t index) {
    size_t currentIndex = 0;
    spinlock_lock(&PCIDevices->lock);
    foreach(deviceNode, PCIDevices) {
        pci_device_t *device = (pci_device_t *)deviceNode->value;

        if(device->device_class == class && device->subclass == subclass && device->prog_if == prog_if) {
            if(index == currentIndex) {
                spinlock_unlock(&PCIDevices->lock);
                return device;
            }
            currentIndex++;
        }
    }
    spinlock_unlock(&PCIDevices->lock);

    return NULL;
}

pci_device_t *pci_get_device_by_vendor(uint16_t vendorID, uint16_t deviceID, size_t index) {
    size_t currentIndex = 0;
    spinlock_lock(&PCIDevices->lock);
    foreach(deviceNode, PCIDevices) {
        pci_device_t *device = (pci_device_t *)deviceNode->value;

        if(device->vendor_id == vendorID && device->device_id == deviceID) {
            if(index == currentIndex) {
                spinlock_unlock(&PCIDevices->lock);
                return device;
            }
            currentIndex++;
        }
    }
    spinlock_unlock(&PCIDevices->lock);

    return NULL;
}

bool pci_read_bar(pci_device_t *device, uint8_t bar, pci_bar_t *out) {
    if(bar > 5) return false;

    uint8_t offset = 0x10 + bar * 0x4;
    uint64_t barAddressLow = pci_device_read_config(device, offset, sizeof(uint32_t));
    if(!barAddressLow) return false;

    bool isMmio = !(barAddressLow & 0x1);
    bool isPrefetchable = isMmio && barAddressLow & (1 << 3);
    bool isLong = isMmio && (barAddressLow & 0x6) == 0x4;

    uint64_t barAddressHigh = 0;
    if(isLong) barAddressHigh = pci_device_read_config(device, offset + 0x4, sizeof(uint32_t));

    uint64_t barAddress = (barAddressHigh << 32 | barAddressLow) & ~(isMmio ? (uint64_t)0xF : (uint64_t)0x3);

    pci_device_write_config(device, offset, 0xFFFFFFFF, sizeof(uint32_t));
    uint64_t barSizeLow = pci_device_read_config(device, offset, sizeof(uint32_t));
    pci_device_write_config(device, offset, barAddressLow, sizeof(uint32_t));

    uint64_t barSizeHigh = 0;
    if(isLong) {
        pci_device_write_config(device, offset + 0x4, 0xFFFFFFFF, sizeof(uint32_t));
        barSizeHigh = pci_device_read_config(device, offset + 0x4, sizeof(uint32_t));
        pci_device_write_config(device, offset + 0x4, barAddressHigh, sizeof(uint32_t));
    }

    uint64_t size = ((barSizeHigh << 32) | barSizeLow) & ~(isMmio ? (uint64_t)0xF : (uint64_t)0x3);
    size = ~size + 1;

    if(out)
        *out = (pci_bar_t) {
            barAddress, size, isMmio, isPrefetchable
        };

    return true;
}

void pci_init() {
    PCIDevices = list_create();

    if(!(pci_read_config(0, 0, 0, 0, 0xC, sizeof(uint32_t)) & 0x800000))
        pci_check_bus(0, 0, -1);
    else {
        for(size_t func = 0; func < 8; func++) {
            if((pci_read_config(0, 0, 0, func, 0x0, sizeof(uint32_t)) & 0xFFFF) == 0xFFFF)
                continue;

            pci_check_bus(0, func, -1);
        }
    }

    if(PCIEHolder)
        for(uint16_t seg = 1; seg < PCIEHolder->size; seg++) {
            pci_check_bus(PCIEHolder->entries[seg].seg, 0, -1);
        }

    //Route interrupts
    LAI_CLEANUP_STATE lai_state_t laiState;
    lai_init_state(&laiState);

    spinlock_lock(&PCIDevices->lock);
    foreach(deviceNode, PCIDevices) {
        pci_device_t *device = (pci_device_t *)deviceNode->value;

        printf("[%04X] %02X:%02X.%01X", device->seg, device->bus, device->dev, device->func);
        printf(" - %04X:%04X\n", device->vendor_id, device->device_id);

        if(!device->irq_pin) continue;

        uint8_t irqPin = device->irq_pin;

        pci_device_t *currentDevice = device;
        LAI_CLEANUP_VAR lai_variable_t rootPrt = LAI_VAR_INITIALIZER;
        lai_variable_t *prt = NULL;

        while(true) {
            if(currentDevice->parent != -1) { //If the parent is not the bus itself
                spinlock_unlock(&PCIDevices->lock);
                pci_device_t *parent = (pci_device_t *)list_get_indexed(PCIDevices, currentDevice->parent)->value;
                spinlock_lock(&PCIDevices->lock);

                if(!parent->has_prt) irqPin = (((irqPin - 1) + currentDevice->dev % 0x4) % 0x4) + 1;
                else {
                    prt = &parent->acpiPrt; //Found
                    break;
                }
                currentDevice = parent;
            } else {
                lai_nsnode_t *node = pci_determine_root_bus_node(currentDevice->seg, currentDevice->bus, &laiState);

                lai_nsnode_t *prtHandle = lai_resolve_path(node, "_PRT");

                if(prtHandle && lai_eval(&rootPrt, prtHandle, &laiState)) break;

                prt = &rootPrt;
                break;
            }
        }

        if(!prt) {
            printf("[PCI] Failed to get _PTR for this device\n");
            continue;
        }

        lai_api_error_t error;
        struct lai_prt_iterator iterator = LAI_PRT_ITERATOR_INITIALIZER(prt);

        while(!(error = lai_pci_parse_prt(&iterator))) {
            if(iterator.slot == currentDevice->dev && (iterator.function == currentDevice->func || iterator.function == -1) && iterator.pin == (irqPin - 1)) {
                device->gsi = iterator.gsi;
                device->gsi_flags |= (!!iterator.active_low) << 2;
                device->gsi_flags |= (!!iterator.level_triggered) << 8;

                printf("\t - Routed to gsi %d\n", iterator.gsi);
                break;
            }
        }
    }
    spinlock_unlock(&PCIDevices->lock);

    printf("[PCI] Inited\n");

    //Init drivers here
    nvme_init();
}