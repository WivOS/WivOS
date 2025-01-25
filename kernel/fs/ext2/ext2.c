#include "ext2.h"
#include <fs/devfs/devfs.h>
#include <utils/spinlock.h>

typedef struct {
    uint32_t inodes;
    uint32_t blocks;
    uint32_t reserved_blocks;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t start_block;
    uint32_t log2_block_size;
    uint32_t log2_fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t mount_time;
    uint32_t written_time;
    uint16_t mounts_after_check;
    uint16_t mounts_allowed;
    uint16_t signature;
    uint16_t state;
    uint16_t error_algorithm;
    uint16_t minor_version;
    uint32_t check_time;
    uint32_t time_between_checks;
    uint32_t operating_system;
    uint32_t major_version;
    uint16_t user_id;
    uint16_t group_id;
} ext2_superblock_t;

typedef struct {
    uint32_t non_reserved_inode;
    uint16_t inode_structure_size;
    uint16_t block_group_backup;
    uint32_t optional_features;
    uint32_t required_features;
    uint32_t features_readonly;
    uint8_t filesystem_id[16];
    uint8_t volume_name[16];
    uint8_t path_mounted[64];
    uint32_t compression_algorithms;
    uint8_t file_blocks_preallocate;
    uint8_t directory_blocks_preallocate;
    uint16_t unused0;
    uint8_t journal_id[16];
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_inode_list_head;
    uint8_t unused1[788];
} ext2_extended_superblock_t;

typedef struct {
    uint32_t block_usage_address;
    uint32_t inode_usage_address;
    uint32_t inode_table_start_block;
    uint16_t unallocated_blocks;
    uint16_t unallocated_inodes;
    uint16_t directory_count;
    uint16_t pad;
    uint8_t unused[12];
} ext2_block_group_descriptor_t;

typedef struct {
    uint16_t type_and_perms;
    uint16_t user_id;
    uint32_t size_lo;
    uint32_t access_time;
    uint32_t creation_time;
    uint32_t modification_time;
    uint32_t deletion_time;
    uint16_t group_id;
    uint16_t hard_links;
    uint32_t disk_sectors;
    uint32_t flags;
    uint32_t os_value1;
    uint32_t direct_block_pointers[12];
    uint32_t singly_indirect_block_pointer;
    uint32_t doubly_indirect_block_pointer;
    uint32_t triply_indirect_block_pointer;
    uint32_t generation_number;
    uint32_t ext_attribute_block;
    uint32_t size_hi;
    uint32_t fragment_address;
    uint8_t os_value2[12];
} ext2_inode_data_t;

typedef struct {
    uint32_t inode;
    uint16_t size;
    uint8_t size_lo;
    uint8_t type;
    uint8_t name[];
} ext2_directory_entry_t;

typedef struct {
    uint32_t block_group;
    uint32_t inode_table_block_offset;
    uint32_t inode_block_address_offset;
} ext2_inode_addr_info_t;

typedef struct {
    vfs_node_t *deviceNode;

    ext2_superblock_t superblock;
    ext2_extended_superblock_t extended_superblock;

    uint32_t blockSize;
    uint32_t fragmentSize;
    uint32_t groupCount;

    uint32_t singlyStartBlockIndex;
    uint32_t doublyStartBlockIndex;
    uint32_t triplyStartBlockIndex;

    uint8_t *tempBlockData;

    spinlock_t lock;
} ext2fs_custom_data_t;

typedef struct {
    ext2fs_custom_data_t *data;

    uint32_t inode;
    ext2_inode_data_t inodeData;

    uint32_t cluster;
} ext2fs_device_t;

static void temp_print_buffer(const char *buffer, size_t size) {
    for(int i = 0; i < size; i++) {
        uint8_t value = *((uint8_t *)((uintptr_t)buffer + i));
        if((i & 0xF) == 0xF || i == (size - 1)) {
            printf("%02X\n", value);
        } else {
            printf("%02X, ", value);
        }
    }
}

static size_t read_device_offset(vfs_node_t *node, uint64_t offset, void *buffer, size_t size) {
    vfs_lseek(node, offset, SEEK_SET);
    return vfs_read(node, (char *)buffer, size);
}

static size_t read_device_block(ext2fs_custom_data_t *data, uint64_t block, void *buffer, size_t size) {
    vfs_lseek(data->deviceNode, block << (10 + data->superblock.log2_block_size), SEEK_SET);
    return vfs_read(data->deviceNode, (char *)buffer, size);
}

static size_t read_device_block_offset(ext2fs_custom_data_t *data, uint64_t block, uint64_t offset, void *buffer, size_t size) {
    vfs_lseek(data->deviceNode, (block << (10 + data->superblock.log2_block_size)) + offset, SEEK_SET);
    return vfs_read(data->deviceNode, (char *)buffer, size);
}

static ext2_inode_addr_info_t get_inode_addr_info(ext2fs_custom_data_t *data, uint32_t inode) {
    uint32_t block_group = (inode - 1) / data->superblock.inodes_per_group;
    uint32_t index = (inode - 1) % data->superblock.inodes_per_group;
    uint32_t inode_table_block_offset = (index * data->extended_superblock.inode_structure_size) / data->blockSize;
    uint32_t inode_block_address_offset = (index * data->extended_superblock.inode_structure_size) % data->blockSize;

    return (ext2_inode_addr_info_t) {
        .block_group = block_group,
        .inode_table_block_offset = inode_table_block_offset,
        .inode_block_address_offset = inode_block_address_offset
    };
}

static void ext2_read_inode_struct(ext2fs_custom_data_t *data, uint32_t inode, ext2_inode_data_t *out) {
    ext2_inode_addr_info_t addr_info = get_inode_addr_info(data, inode);

    ext2_block_group_descriptor_t groupDescriptor;
    uint32_t blockNumber = (addr_info.block_group * data->superblock.blocks_per_group) + data->superblock.start_block + 1;
    read_device_block(data, blockNumber, &groupDescriptor, sizeof(ext2_block_group_descriptor_t));

    uint32_t sizeToRead = data->extended_superblock.inode_structure_size;
    if(sizeToRead > sizeof(ext2_inode_data_t)) {
        sizeToRead = sizeof(ext2_inode_data_t);
    }

    blockNumber = groupDescriptor.inode_table_start_block + addr_info.inode_table_block_offset;
    read_device_block_offset(data, blockNumber, addr_info.inode_block_address_offset, out, sizeToRead);
}

static uint32_t getBlockFromFileBlockOffset(ext2fs_custom_data_t *data, ext2_inode_data_t *inode, uint64_t blockOffset) {
    uint32_t maskCommon = ~((uint32_t)((uint32_t)0xFFFFFFFF << (8 + data->superblock.log2_block_size)));

    if(blockOffset < data->singlyStartBlockIndex) {
        return inode->direct_block_pointers[blockOffset];
    } else if(blockOffset < data->singlyStartBlockIndex) {
        blockOffset -= data->singlyStartBlockIndex;
        blockOffset &= maskCommon;

        uint32_t result;
        read_device_block_offset(data, inode->singly_indirect_block_pointer, blockOffset << 2, &result, 2);
        return result;
    } else if(blockOffset < data->doublyStartBlockIndex) {
        blockOffset -= data->doublyStartBlockIndex;
        uint32_t doublyOffset = (blockOffset >> (8 + data->superblock.log2_block_size)) & maskCommon;
        uint32_t singlyOffset = blockOffset & maskCommon;

        uint32_t singlyBlock;
        uint32_t result;
        read_device_block_offset(data, inode->doubly_indirect_block_pointer, doublyOffset << 2, &singlyBlock, 2);
        read_device_block_offset(data, singlyBlock, singlyOffset << 2, &result, 2);
        return result;
    } else {
        blockOffset -= data->triplyStartBlockIndex;
        uint32_t triplyOffset = (blockOffset >> (18 + data->superblock.log2_block_size)) & maskCommon;
        uint32_t doublyOffset = (blockOffset >> (8 + data->superblock.log2_block_size)) & maskCommon;
        uint32_t singlyOffset = blockOffset & maskCommon;

        uint32_t doublyBlock;
        uint32_t singlyBlock;
        uint32_t result;
        read_device_block_offset(data, inode->doubly_indirect_block_pointer, triplyOffset << 2, &doublyBlock, 2);
        read_device_block_offset(data, doublyBlock, doublyOffset << 2, &singlyBlock, 2);
        read_device_block_offset(data, singlyBlock, singlyOffset << 2, &result, 2);
        return result;
    }
}

static uint32_t getBlockFromFileOffset(ext2fs_custom_data_t *data, ext2_inode_data_t *inode, uint64_t offset) {
    uint64_t blockOffset = offset >> (10 + data->superblock.log2_block_size);
    return getBlockFromFileBlockOffset(data, inode, blockOffset);
}

size_t ext2_read(struct vfs_node *node, char *buffer, size_t size) {
    ext2fs_device_t *device = (ext2fs_device_t *)node->data;
    ext2fs_custom_data_t *data = device->data;

    spinlock_lock(&device->data->lock);

    size_t fileSize = device->inodeData.size_lo; //TODO: Allow bigger files

    //TODO: Check file size
    uint32_t relativeBlock = node->offset >> (10 + data->superblock.log2_block_size);
    uint32_t maskCommon = ~((uint32_t)((uint32_t)0xFFFFFFFF << (8 + data->superblock.log2_block_size)));
    uint32_t inBlockOffset = node->offset & maskCommon;

    size_t readSize = 0;
    if((inBlockOffset + size) > data->blockSize) {
        size_t dataToRead = data->blockSize - inBlockOffset;
        while(size) {
            if((node->offset + dataToRead) > fileSize) { //Reaching EOF
                dataToRead = fileSize - node->offset;
            }

            uint32_t readBlock = getBlockFromFileBlockOffset(data, &device->inodeData, relativeBlock);
            if(readBlock < 2) { //ERROR
                readSize = -1;
                break;
            }
            read_device_block_offset(data, readBlock, inBlockOffset, buffer, dataToRead);
            readSize += dataToRead;
            size -= dataToRead;
            node->offset += dataToRead;
            if(node->offset >= fileSize) { //EOF reached
                break;
            }

            inBlockOffset = 0;
            dataToRead = (size > data->blockSize) ? data->blockSize : size;

            relativeBlock++;
        }
    } else {
        size_t dataToRead = size;
        if((node->offset + dataToRead) > fileSize) {//Reaching EOF
            dataToRead = fileSize - node->offset;
        }

        uint32_t readBlock = getBlockFromFileBlockOffset(data, &device->inodeData, relativeBlock);
        if(readBlock >= 2) {
            read_device_block_offset(data, readBlock, inBlockOffset, buffer, size);
            readSize = size;
        } else { //ERROR
            readSize = -1;
        }
    }

    spinlock_unlock(&device->data->lock);

    return readSize;
}

size_t ext2_lseek(vfs_node_t *node, size_t offset_, size_t type) {
    ext2fs_device_t *device = (ext2fs_device_t *)node->data;

    spinlock_lock(&device->data->lock);

    size_t length = device->inodeData.size_lo; //TODO: Allow bigger files
    int64_t offset = (int64_t)offset_;

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
            spinlock_unlock(&device->data->lock);
            return -1;
    }

    spinlock_unlock(&device->data->lock);
    return node->offset;
}

size_t ext2_readdir(vfs_node_t *node_dev, vsf_dirent_t *dirent) {
    //TODO
    return -1;
}

uint32_t find_file_inode_recursive(ext2fs_custom_data_t *data, uint32_t inode, char **inPath, bool *should_continue) {
    char *path = *inPath;
    char *slash = strchrnul(path, '/');

    char subPath[1024] = {0};
    char name[1024] = {0};
    bool pathIsAFile = false;
    if(*slash == '\0') {
        memcpy(subPath, path, strlen(path));
        pathIsAFile = true;
    } else {
        memcpy(subPath, path, slash - path);
        subPath[slash - path] = '\0';
    }
    *should_continue = false;

    //TODO: Check if this is always true
    if(!strcmp(subPath, ".")) {
        if(!pathIsAFile) {
            *inPath = slash + 1;
            *should_continue = true;
        }

        return inode;
    }
    //If directries are not the common directories continue

    ext2_inode_data_t inodeData = {0};
    ext2_read_inode_struct(data, inode, &inodeData);
    if(!(inodeData.type_and_perms & EXT2_INODE_DIRECTORY)) return 0; //Should not be called for a file

    uint32_t size = inodeData.size_lo; //TODO: Extended size
    uint32_t dataToRead = size;
    uint32_t offset = 0; //This is valid beacuse directory files entries are not allowed to cross block boundary
    uint32_t current_block = 0;

    uint8_t *tempData = (uint8_t *)kmalloc(size > data->blockSize ? data->blockSize : size);
    read_device_block(data, getBlockFromFileBlockOffset(data, &inodeData, current_block), tempData, size > data->blockSize ? data->blockSize : size);

    ext2_directory_entry_t *entry = (ext2_directory_entry_t *)tempData;
    while((int)dataToRead > 0) {
        ext2_inode_data_t childInodeData = {0};
        ext2_read_inode_struct(data, entry->inode, &childInodeData);

        memcpy(name, entry->name, entry->size_lo);
        name[entry->size_lo] = '\0';
        if(!strcmp(name, subPath)) { //Inode Matches
            if(!pathIsAFile) {
                kfree(tempData);

                *inPath = slash + 1;
                *should_continue = true;
            }

            //If we are not at the end of the path and the file type is not a directory then we have a problem
            if(pathIsAFile || (childInodeData.type_and_perms & EXT2_INODE_DIRECTORY) != 0) {
                return entry->inode;
            }
        }

        dataToRead -= (uint32_t)entry->size;
        offset += entry->size;

        if(offset >= data->blockSize) { //Read new block
            offset = 0;
            current_block++;

            read_device_block(data, getBlockFromFileBlockOffset(data, &inodeData, current_block), tempData, dataToRead > data->blockSize ? data->blockSize : dataToRead);
            entry = (ext2_directory_entry_t *)tempData;
        } else {
            entry = (ext2_directory_entry_t *)((uintptr_t)entry + (uintptr_t)entry->size);
        }
    }

    kfree(tempData);

    return 0; //Not found
}

vfs_node_t *ext2_finddir(vfs_node_t *node_dev, char *name, char **last_path) {
    ext2fs_custom_data_t *customData = (ext2fs_custom_data_t *)node_dev->data;

    spinlock_lock(&customData->lock);

    char path[1024];
    if(*last_path && strlen(*last_path)) {
        sprintf(path, "%s/%s", name, *last_path);
    } else {
        strcpy(path, name);
    }
    char *pathPtr = (char *)path;

    uint32_t inode = EXT2_INODE_ROOT;
    if(pathPtr && *pathPtr != '\0') {
        bool should_continue = true;
        while(should_continue) {
            inode = find_file_inode_recursive(customData, inode, &pathPtr, &should_continue);
            if(inode == 0) break;
        }
    }

    vfs_node_t *node = NULL;
    if(inode)
    {
        ext2_inode_data_t inodeData = {0};
        ext2_read_inode_struct(customData, inode, &inodeData);

        node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
        ext2fs_device_t *device = (ext2fs_device_t *)kmalloc(sizeof(ext2fs_device_t));
        device->data = customData;
        device->inode = inode;
        memcpy(&device->inodeData, &inodeData, sizeof(ext2_inode_data_t));

        memcpy(node->name, path, 1024);
        node->data = (void *)device;
        if(inodeData.type_and_perms & EXT2_INODE_DIRECTORY) {
            node->flags |= VFS_FILE;
            node->functions.readdir = ext2_readdir;
        } else {
            //TODO: Add different types of files
            node->flags |= VFS_FILE;
            node->functions.read = ext2_read;
            node->functions.lseek = ext2_lseek;
        }

        *last_path = NULL; //Null it out, to indicate we handled the directory finding
    }

    spinlock_unlock(&customData->lock);
    return node;
}

void parse_and_print_directory_recursive(ext2fs_custom_data_t *data, uint32_t parent_inode, uint32_t inode, size_t parentOffset) {
    ext2_inode_data_t tempInodeData = {0};
    ext2_read_inode_struct(data, inode, &tempInodeData);
    if(!(tempInodeData.type_and_perms & EXT2_INODE_DIRECTORY)) return; //Should not be called for a file

    uint32_t size = tempInodeData.size_lo; //TODO: Extended size
    uint32_t dataToRead = size;
    uint32_t offset = 0; //This is valid beacuse directory files entries are not allowed to cross block boundary
    uint32_t current_block = 0;

    uint8_t *tempData = (uint8_t *)kmalloc(size > data->blockSize ? data->blockSize : size);
    read_device_block(data, getBlockFromFileBlockOffset(data, &tempInodeData, current_block), tempData, size > data->blockSize ? data->blockSize : size);

    ext2_directory_entry_t *entry = (ext2_directory_entry_t *)tempData;
    while((int)dataToRead > 0) {
        if(entry->inode == 0) {
            //Skip
        } else if(entry->inode == parent_inode) {
            //for(int j = 0; j < parentOffset; j++) printf(" ");
            //printf("%*.*s(back inode=%d)\n", entry->size_lo, entry->size_lo, entry->name, entry->inode);
        } else if(entry->inode == inode) {
            //for(int j = 0; j < parentOffset; j++) printf(" ");
            //printf("%*.*s(this inode=%d)\n", entry->size_lo, entry->size_lo, entry->name, entry->inode);
        } else {
            ext2_inode_data_t childInodeData = {0};
            ext2_read_inode_struct(data, entry->inode, &childInodeData);
            if(childInodeData.type_and_perms & EXT2_INODE_DIRECTORY) {
                //Directory
                for(int j = 0; j < parentOffset; j++) printf(" ");
                printf("%*.*s(dir inode=%d)\n", entry->size_lo, entry->size_lo, entry->name, entry->inode);

                //Do the recursive parse on that directory too
                parse_and_print_directory_recursive(data, inode, entry->inode, parentOffset + entry->size_lo + 1);
            } else {
                //Other file
                for(int j = 0; j < parentOffset; j++) printf(" ");
                printf("%*.*s(file inode=%d)\n", entry->size_lo, entry->size_lo, entry->name, entry->inode);
            }
        }

        dataToRead -= (uint32_t)entry->size;
        offset += entry->size;

        if(offset >= data->blockSize) { //Read new block
            offset = 0;
            current_block++;

            read_device_block(data, getBlockFromFileBlockOffset(data, &tempInodeData, current_block), tempData, dataToRead > data->blockSize ? data->blockSize : dataToRead);
            entry = (ext2_directory_entry_t *)tempData;
        } else {
            entry = (ext2_directory_entry_t *)((uintptr_t)entry + (uintptr_t)entry->size);
        }
    }

    kfree(tempData);
}

void ext2_print_tree(struct vfs_node *node, size_t parentOffset) {
    ext2fs_custom_data_t *customData = (ext2fs_custom_data_t *)node->data;

    spinlock_lock(&customData->lock);

    parse_and_print_directory_recursive(customData, EXT2_INODE_ROOT, EXT2_INODE_ROOT, parentOffset);

    spinlock_unlock(&customData->lock);
}

size_t ext2_mount(struct vfs_node *node, char *device, size_t flags, char *path, void *data) {
    vfs_node_t *deviceNode = kopen(device, flags);
    if(deviceNode == NULL)
        return -1;

    ext2fs_custom_data_t *customData = (ext2fs_custom_data_t *)kmalloc(sizeof(ext2fs_custom_data_t));
    customData->deviceNode = deviceNode;

    uint16_t signature;
    read_device_offset(deviceNode, 0x438, &signature, 2);
    if(signature != EXT2_SIGNATURE) {
        printf("[EXT2] Wrong signature 0x%x\n", signature);

        kfree(customData);
        vfs_close(deviceNode);
        kfree(deviceNode);

        return -1;
    }

    read_device_offset(deviceNode, 0x400, &customData->superblock, sizeof(ext2_superblock_t));

    memset(&customData->extended_superblock, 0, sizeof(ext2_extended_superblock_t));
    if(customData->superblock.major_version >= 1) {
        read_device_offset(deviceNode, 0x454, &customData->extended_superblock, sizeof(ext2_extended_superblock_t));
    } else {
        customData->extended_superblock.non_reserved_inode = 11;
        customData->extended_superblock.inode_structure_size = 128;
    }

    customData->blockSize = 0x400 << customData->superblock.log2_block_size;
    if((int32_t)customData->superblock.log2_fragment_size < 0)
        customData->fragmentSize = 0x400 >> (-(int32_t)customData->superblock.log2_fragment_size);
    else
        customData->fragmentSize = 0x400 << customData->superblock.log2_fragment_size;


    {
        uint32_t groupCount0 = customData->superblock.blocks / customData->superblock.blocks_per_group;
        if(customData->superblock.blocks % customData->superblock.blocks_per_group) groupCount0++;

        uint32_t groupCount1 = customData->superblock.inodes / customData->superblock.inodes_per_group;
        if(customData->superblock.inodes % customData->superblock.inodes_per_group) groupCount1++;

        if(groupCount1 != groupCount0) {
            printf("[EXT2] Group count mismatch\n");

            kfree(customData);
            vfs_close(deviceNode);
            kfree(deviceNode);

            return -1;
        }
        customData->groupCount = groupCount0;
    }

    customData->singlyStartBlockIndex = 12;
    customData->doublyStartBlockIndex = customData->singlyStartBlockIndex + customData->blockSize / sizeof(uint32_t);
    customData->triplyStartBlockIndex = customData->doublyStartBlockIndex + (customData->blockSize / sizeof(uint32_t)) * (customData->blockSize / sizeof(uint32_t));
    customData->lock = INIT_SPINLOCK();
    customData->tempBlockData = (uint8_t *)kmalloc(customData->blockSize);

    vfs_node_t *ext2Node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    strcpy(ext2Node->name, "Ext2");
    ext2Node->data = (void *)customData;
    ext2Node->functions.finddir = ext2_finddir;
    ext2Node->functions.print_tree = ext2_print_tree;
    ext2Node->flags |= VFS_DIRECTORY;
    vfs_mount(path, ext2Node);

    return -1;
}

void ext2_init() {
    devfs_node_t *ext2Node = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
    strcpy(ext2Node->name, "Ext2Dev");
    ext2Node->flags |= VFS_FILESYSTEM;
    ext2Node->functions.mount = ext2_mount;
    devfs_mount("ext2", ext2Node);
}