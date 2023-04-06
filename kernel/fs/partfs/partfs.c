#include "partfs.h"

#include <utils/lists.h>
#include <fs/devfs/devfs.h>

typedef struct {
    vfs_node_t *deviceNode;
    uint64_t first_lba;
    uint64_t lba_count;

    volatile spinlock_t lock;
    bool offsetDirty;

    vfs_functions_t functions;
} partfs_entry_t;

typedef struct {
    uint8_t signature[8];
    uint32_t revision;
    uint32_t header_size;
    uint32_t header_checksum;
    uint32_t reserved;
    uint64_t header_lba;
    uint64_t alternate_header_lba;
    uint64_t first_usable_lba;
    uint64_t last_usable_lba;
    uint8_t guid[16];
    uint64_t start_lba;
    uint32_t count;
    uint32_t entry_length;
    uint32_t parition_table_checksum;
} __attribute__((packed)) gpt_header_t;

typedef struct {
    uint8_t type_guid[16];
    uint8_t part_guid[16];
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    uint8_t name[72];
} __attribute__((packed)) gpt_partition_header_t;

size_t partfs_read(struct vfs_node *node, char *buffer, size_t size) {
    partfs_entry_t *entry = (partfs_entry_t *)node->data;

    spinlock_lock(&entry->lock);

    if(node->offset + size >= (entry->lba_count * 0x200)) {
        size = entry->lba_count * 0x200 - node->offset;
    }

    if(entry->offsetDirty) {
        vfs_lseek(entry->deviceNode, entry->first_lba * 0x200 + node->offset, SEEK_SET);
        entry->offsetDirty = false;
    }

    size_t count = vfs_read(entry->deviceNode, buffer, size);
    if(count == -1) {
        spinlock_unlock(&entry->lock);
        return -1;
    }

    node->offset += count;

    spinlock_unlock(&entry->lock);

    return count;
}

size_t partfs_write(struct vfs_node *node, char *buffer, size_t size) {
    partfs_entry_t *entry = (partfs_entry_t *)node->data;

    spinlock_lock(&entry->lock);

    if(node->offset + size >= (entry->lba_count * 0x200)) {
        size = entry->lba_count * 0x200 - node->offset;
    }

    if(entry->offsetDirty) {
        vfs_lseek(entry->deviceNode, entry->first_lba * 0x200 + node->offset, SEEK_SET);
        entry->offsetDirty = false;
    }

    size_t count = vfs_write(entry->deviceNode, buffer, size);
    if(count == -1) {
        spinlock_unlock(&entry->lock);
        return -1;
    }

    node->offset += count;

    spinlock_unlock(&entry->lock);

    return count;
}

size_t partfs_lseek(vfs_node_t *node, size_t offset, size_t type) {
    partfs_entry_t *entry = (partfs_entry_t *)node->data;

    spinlock_lock(&entry->lock);

    size_t length = ((entry->first_lba + entry->lba_count) * 0x200);

    switch(type) {
        case SEEK_SET:
            if(offset >= length) goto def;
            node->offset = offset;
            break;
        case SEEK_END:
            if(((length + offset) < 0) || offset >= 0) goto def;
            node->offset = length + offset;
            break;
        case SEEK_CUR:
            if((node->offset + offset) >= length || (node->offset + offset) < 0) goto def;
            node->offset += offset;
            break;
        default:
        def:
            spinlock_unlock(&entry->lock);
            return -1;
    }

    entry->offsetDirty = true;

    spinlock_unlock(&entry->lock);
    return node->offset;
}

static bool partfs_enumerate_gpt(char *deviceName, vfs_node_t *deviceNode) {
    vfs_lseek(deviceNode, 0x200, SEEK_SET);

    gpt_header_t header;
    vfs_read(deviceNode, (char *)&header, sizeof(gpt_header_t));

    uint32_t savedCrc = header.header_checksum;
    header.header_checksum = 0;
    uint32_t headerCrc = crc32_calculate_buffer((uint8_t *)&header, sizeof(gpt_header_t));
    header.header_checksum = savedCrc;

    if(headerCrc != savedCrc) {
        printf("[PartFS] GPT Header Checksum mismatch");
        return false;
    }

    size_t entry_stride = header.entry_length / sizeof(gpt_partition_header_t);
    size_t paritionArraySize = sizeof(gpt_partition_header_t) * entry_stride * header.count;
    gpt_partition_header_t *partitionArray = kmalloc(paritionArraySize);

    vfs_lseek(deviceNode, header.start_lba * 0x200, SEEK_SET);
    vfs_read(deviceNode, (char *)partitionArray, paritionArraySize);

    uint32_t paritionArrayCrc = crc32_calculate_buffer((uint8_t *)partitionArray, paritionArraySize);

    if(paritionArrayCrc != header.parition_table_checksum) {
        printf("[PartFS] GPT Parition Array Checksum mismatch");
        kfree(partitionArray);
        return false;
    }

    //All validated, start indexing the partitions;

    for(size_t i = 0; i < header.count; i++) {
        gpt_partition_header_t *parition = &partitionArray[i * entry_stride];

        uint64_t firstGuidPart = *(uint64_t *)&parition->type_guid[0];
        uint64_t secondGuidPart = *(uint64_t *)&parition->type_guid[8];
        if(!firstGuidPart && !secondGuidPart) continue;

        //Found valid entry
        partfs_entry_t *entry = kmalloc(sizeof(partfs_entry_t));
        entry->deviceNode = deviceNode;
        entry->first_lba = parition->start_lba;
        entry->lba_count = parition->end_lba - parition->start_lba;
        entry->offsetDirty = true;
        entry->lock = INIT_SPINLOCK();

        devfs_node_t *node = kmalloc(sizeof(devfs_node_t));
        char name[256];
        sprintf(name, "%sp%d", deviceName, i);
        strcpy(node->name, "PartFS");
        node->data = entry;
        node->functions.read = partfs_read;
        node->functions.write = partfs_write;
        node->functions.lseek = partfs_lseek;
        devfs_mount(name, node);

        printf("Part %s: %d -> %d\n", node->name, entry->first_lba, entry->lba_count);
    }

    kfree(partitionArray);
    return true;
}

void partfs_init(char *device) {
    char *devicePath = kmalloc(1024);
    sprintf(devicePath, "/dev/%s", device);
    vfs_node_t *deviceNode = kopen(devicePath, 0);

    kfree(devicePath);

    if(deviceNode == NULL) {
        return;
    }

    if(!partfs_enumerate_gpt(device, deviceNode)) {
        printf("[PartFS] No GPT parititions found\n");
        kfree(devicePath);
    }
}