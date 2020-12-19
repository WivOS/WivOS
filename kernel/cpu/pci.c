#include "pci.h"
#include <lai/helpers/pci.h>

static list_t *pciList = NULL;

static void pci_setAddress(pci_device_t *device, uint32_t offset) {
    uint32_t address = 0x80000000 | (device->bus << 16) | ((device->slot & 0x1F) << 11) | ((device->function & 0x7) << 8) | (offset & ~((uint32_t)(3)));
    outl(PCI_ROOT_INDEX, address);
}

uint8_t pci_read_byte(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inb(PCI_ROOT_DATA + (offset & 3));
}

uint16_t pci_read_word(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inw(PCI_ROOT_DATA + (offset & 3));
}

uint32_t pci_read_dword(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inl(PCI_ROOT_DATA + (offset & 3));
}

void pci_write_byte(pci_device_t *device, uint32_t offset, uint8_t value) {
    pci_setAddress(device, offset);
    outb(PCI_ROOT_DATA + (offset & 3), value);
}

void pci_write_word(pci_device_t *device, uint32_t offset, uint16_t value) {
    pci_setAddress(device, offset);
    outw(PCI_ROOT_DATA + (offset & 3), value);
}

void pci_write_dword(pci_device_t *device, uint32_t offset, uint32_t value) {
    pci_setAddress(device, offset);
    outl(PCI_ROOT_DATA + (offset & 3), value);
}

static uint16_t pci_get_vendorID(pci_device_t *device) {
    return pci_read_word(device, 0x0);
}

static void pci_check_bus(uint8_t bus, int64_t parent);

#define PCI_PNP_ID  "PNP0A03"
#define PCIE_PNP_ID "PNP0A08"

static lai_nsnode_t *pci_determine_root_bus_node(uint8_t bus, lai_state_t *state) {
    LAI_CLEANUP_VAR lai_variable_t pci_pnp_id = LAI_VAR_INITIALIZER;
    LAI_CLEANUP_VAR lai_variable_t pcie_pnp_id = LAI_VAR_INITIALIZER;
    lai_eisaid(&pci_pnp_id, PCI_PNP_ID);
    lai_eisaid(&pcie_pnp_id, PCIE_PNP_ID);

    lai_nsnode_t *sb_handle = lai_resolve_path(NULL, "\\_SB_");
    
    struct lai_ns_child_iterator iter = LAI_NS_CHILD_ITERATOR_INITIALIZER(sb_handle);
    lai_nsnode_t *node;

    while((node = lai_ns_child_iterate(&iter))) {
        if(lai_check_device_pnp_id(node, &pci_pnp_id, state) && lai_check_device_pnp_id(node, &pcie_pnp_id, state)) {
            continue;
        }

        LAI_CLEANUP_VAR lai_variable_t busNumber = LAI_VAR_INITIALIZER;
        uint64_t bbnResult = 0;
        lai_nsnode_t *bbnHandle = lai_resolve_path(node, "_BBN");
        if(bbnHandle) {
            if(lai_eval(&busNumber, bbnHandle, state)) {
                continue;
            }
            lai_obj_get_integer(&busNumber, &bbnResult);
        }

        if(bbnResult == bus)
            return node;
    }

    return NULL;
}

static void pci_determine_acpi_node_for(pci_device_t *device, lai_state_t *state) {
    if(device->acpiNode) return;

    lai_nsnode_t *node = NULL;

    if(device->parent != -1) {
        pci_device_t *parent = (pci_device_t *)list_get_node_by_index(pciList, device->parent)->val;
        if(!parent) return;

        if(!parent->acpiNode) pci_determine_acpi_node_for(parent, state);

        node = parent->acpiNode;
    } else {
        node = pci_determine_root_bus_node(device->bus, state);
    }

    if(!node) return;

    device->acpiNode = lai_pci_find_device(node, device->slot, device->function, state);
}

static void checkFunction(uint8_t bus, uint8_t slot, uint8_t function, int64_t parent) {
    pci_device_t dev = {0};
    dev.bus = bus;
    dev.slot = slot;
    dev.function = function;

    if(pci_get_vendorID(&dev) == 0xFFFF) {
        return;
    }

    pci_device_t *dev2 = kmalloc(sizeof(pci_device_t));
    dev2->bus = bus;
    dev2->slot = slot;
    dev2->function = function;

    uint32_t config8 = pci_read_dword(dev2, 0x8);
    uint32_t config3C = pci_read_dword(dev2, 0x3C);

    dev2->parent = parent;
    dev2->deviceID = (uint16_t)(pci_read_dword(dev2, 0x0) >> 16);
    dev2->vendorID = pci_get_vendorID(dev2);
    dev2->revID = (uint8_t)config8;
    dev2->subclass = (uint8_t)(config8 >> 16);
    dev2->classCode = (uint8_t)(config8 >> 24);
    dev2->progIF = (uint8_t)(config8 >> 8);
    dev2->irqPin = (uint8_t)(config3C >> 8);

    if(pci_read_dword(dev2, 0xC) & 0x800000) {
        dev2->multifunction = 1;
    } else {
        dev2->multifunction = 0;
    }

    list_insert_back(pciList, dev2);
    size_t id = list_size(pciList) - 1;

    if(dev2->classCode == 0x06 && dev2->subclass == 0x04) {
        LAI_CLEANUP_STATE lai_state_t state;
        lai_init_state(&state);

        pci_determine_acpi_node_for(dev2, &state);

        if(dev2->acpiNode) {
            lai_nsnode_t *prt_handle = lai_resolve_path(dev2->acpiNode, "_PRT");
        
            if(prt_handle) dev2->hasPtr = !lai_eval(&dev2->acpiPtr, prt_handle, &state);
        }

        pci_check_bus((pci_read_dword(dev2, 0x18) >> 8) & 0xFF, id);
    }
}

static void pci_check_bus(uint8_t bus, int64_t parent) {
    for(size_t i = 0; i < 32; i++) {
        for(size_t j = 0; j < 8; j++) {
            checkFunction(bus, i, j, parent);
        }
    }
}

size_t pci_read_bar(pci_device_t *device, int bar, pci_bar_t *out) {
    if(bar > 5) return -1;

    size_t regIndex = 0x10 + bar * 4;
    uint64_t barLow = pci_read_dword(device, regIndex);
    uint64_t barSizeLow = 0;
    uint64_t barHigh = 0;
    uint64_t barSizeHigh = 0;

    if(!barLow) return -1;

    uint64_t base;
    size_t size;

    uint8_t isMmio = !(barLow & 1);
    uint8_t isPrefetchable = isMmio && barLow & (1 << 3);
    uint8_t is64 = isMmio && ((barLow >> 1) & 0x3) == 0x2;

    if(is64) barHigh = pci_read_dword(device, regIndex + 0x4);

    base = ((barHigh << 32 | barLow) & ~(isMmio ? (0xF) : (0x3)));

    pci_write_dword(device, regIndex, 0xFFFFFFFF);
    barSizeLow = pci_read_dword(device, regIndex);
    pci_write_dword(device, regIndex, barLow);

    if(is64) {
        pci_write_dword(device, regIndex + 0x4, 0xFFFFFFFF);
        barSizeHigh = pci_read_dword(device, regIndex + 0x4);
        pci_write_dword(device, regIndex + 0x4, barHigh);
    }

    size = ((barSizeHigh << 32 | barSizeLow) & ~(isMmio ? 0xF : 0x3));
    size = ~size + 1;

    if(out) {
        *out = (pci_bar_t){base, size, isMmio, isPrefetchable};
    }

    return 0;
}

size_t pci_register_msi(pci_device_t *device, uint8_t vector) {
    uint8_t off = 0;

    uint32_t config_4 = pci_read_dword(device, 0x4);
    uint8_t  config_34 = pci_read_byte(device, 0x34);

    if((config_4 >> 16) & (1 << 4)) {
        uint8_t cap_off = config_34;

        while(cap_off) {
            uint8_t cap_id = pci_read_byte(device, cap_off);
            uint8_t cap_next = pci_read_byte(device, cap_off + 1);

            switch(cap_id) {
                case 0x05: {
                    printf("pci: device has msi support\n");
                    off = cap_off;
                    break;
                }
            }
            cap_off = cap_next;
        }
    }

    if(off == 0) {
        printf("pci: device does not support msi\n");
        return 0;
    }

    return 1;
}

void pci_enable_busmastering(pci_device_t *device) {
    if(!(pci_read_dword(device, 0x4) & (1 << 2))) {
        pci_write_dword(device, 0x4, pci_read_dword(device, 0x4) | (1 << 2) | (1 << 1));
    }
}

pci_device_t *pci_get_device_by_vendor(uint16_t vendor, uint16_t device) {
    for (size_t i = 0; i < list_size(pciList); i++) {
        pci_device_t *dev = (pci_device_t *)((list_get_node_by_index(pciList, i))->val);

        if(dev->vendorID == vendor && dev->deviceID == device) return dev;
    }

    return NULL;
}

pci_device_t *pci_get_device_by_class_subclass(uint8_t class, uint8_t subclass, uint8_t progIF, size_t index) {
    size_t k = 0;
    for (size_t i = 0; i < list_size(pciList); i++) {
        pci_device_t *dev = (pci_device_t *)((list_get_node_by_index(pciList, i))->val);

        if(dev->classCode == class && dev->subclass == subclass && dev->progIF == progIF) {
            if(k == index) {
                return dev;
            }
            k++;
        }
    }

    return NULL;
}

static void pci_route_interrupts() {
    LAI_CLEANUP_STATE lai_state_t state;
    lai_init_state(&state);

    for(size_t i = 0; i < list_size(pciList); i++) {
        pci_device_t *dev = (pci_device_t *)list_get_node_by_index(pciList, i)->val;
    
        if(!dev->irqPin) continue;

        uint8_t irqPin = dev->irqPin;
        pci_device_t *tmp = dev;
        LAI_CLEANUP_VAR lai_variable_t root_prt = LAI_VAR_INITIALIZER;
        lai_variable_t *prt = NULL;

        while(1) {
            if(tmp->parent != -1) {
                pci_device_t *parent = (pci_device_t *)list_get_node_by_index(pciList, tmp->parent)->val;

                if(!parent->hasPtr) {
                    irqPin = (((irqPin - 1) + (tmp->slot % 4)) % 4) + 1;
                } else {
                    prt = &parent->acpiPtr;
                    break;
                }
                tmp = parent;
            } else {
                lai_nsnode_t *node = pci_determine_root_bus_node(tmp->bus, &state);

                lai_nsnode_t *prt_handle = lai_resolve_path(node, "_PRT");
                if(prt_handle) {
                    if(lai_eval(&root_prt, prt_handle, &state)) break;
                }

                prt = &root_prt;
                break;
            }
        }

        if(!prt) {
            printf("failed to get ptr for device\n");
            continue;
        }

        struct lai_prt_iterator iter = LAI_PRT_ITERATOR_INITIALIZER(prt);
        lai_api_error_t err;

        while(!(err = lai_pci_parse_prt(&iter))) {
            if(iter.slot == tmp->slot && (iter.function == tmp->function || iter.function == -1) && iter.pin == (irqPin - 1)) {
                dev->gsi = iter.gsi;
                dev->gsiFlags |= (!!iter.active_low) << 2;
                dev->gsiFlags |= (!!iter.level_triggered) << 8;

                printf("[PCI] Device %02x:%02x.%01x routed to gsi %u\n", dev->bus, dev->slot, dev->function, iter.gsi);
                break;
            }
        }
    }
}

void pci_init() {
    pciList = list_create();

    pci_device_t device = {0};
    if(!(pci_read_dword(&device, 0xC) & 0x800000)) {
        pci_check_bus(0, -1);
    } else {
        for(size_t j = 0; j < 8; j++) {
            device.function = j;
            if(pci_get_vendorID(&device) == 0xFFFF) {
                return;
            }

            pci_check_bus(j, -1);
        }
    }

    for (size_t i = 0; i < list_size(pciList); i++) {
        pci_device_t *dev = (pci_device_t *)((list_get_node_by_index(pciList, i))->val);

        printf("pci:\t%02X : %02X.%01X - %04X : %04X\n", dev->bus, dev->slot, dev->function, dev->vendorID, dev->deviceID);
    }

    pci_route_interrupts();
}