#include "fat32.h"
#include <fs/devfs/devfs.h>
#include <utils/spinlock.h>

typedef struct {
    vfs_node_t *deviceNode;

    uint16_t sector_length;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint32_t sectors_per_fat;
    uint32_t root_dir_cluster;

    uint32_t begin_cluster;

    spinlock_t lock;
} fat32fs_custom_data_t;

typedef struct {
    uint8_t order;
    uint8_t name1[10];
    uint8_t attribute;
    uint8_t entry_type;
    uint8_t checksum;
    uint8_t name2[12];
    uint16_t zero;
    uint8_t name3[4];
} __attribute__((packed)) fat32_long_file_name_t;

typedef struct {
    uint8_t name[11];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t create_tenth_seconds_time;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_date;
    uint16_t first_cluster_hi;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t first_cluster_lo;
    uint32_t size;
} __attribute__((packed)) fat32_directory_t;

typedef struct {
    union {
        fat32_directory_t directory;
        fat32_long_file_name_t long_file_name;
    };
} __attribute__((packed)) fat32_entry_t;

typedef struct {
    fat32fs_custom_data_t *customData;

    fat32_entry_t entry;
} fat32fs_device_t;

#define SECTOR_TO_OFFSET(x) ((x) << 9)
#define CLUSTER_TO_OFFSET(x, base, sectors) (SECTOR_TO_OFFSET((uint64_t)(base) + ((uint64_t)(x) - 2) * (uint64_t)(sectors)))

static size_t read_device_offset(vfs_node_t *node, uint64_t offset, void *buffer, size_t size) {
    vfs_lseek(node, offset, SEEK_SET);
    return vfs_read(node, (char *)buffer, size);
}

static bool fat32_read_fat_entry(fat32fs_custom_data_t *data, uint32_t cluster, size_t index, fat32_entry_t *entry) {
    if(cluster < 2 || cluster >= 0x0FFFFFF8) return false;

    uint64_t offset = CLUSTER_TO_OFFSET(cluster, data->begin_cluster,
        data->sectors_per_cluster) + index * 32;

    read_device_offset(data->deviceNode, offset, entry, 32);

    /*if(cluster > 3) {
        size_t x = 0;
        while(x < 32) {
            for(int i = 0; i < 16; i++) {
                if(i == 15)
                    printf("%02.2X\n", ((uint8_t *)entry)[x] & 0xFF);
                else
                    printf("%02.2X, ", ((uint8_t *)entry)[x] & 0xFF);
                x++;
            }
        }
    }*/

    if(!entry->directory.name[0]) return false;

    return true;
}

static uint32_t fat32_get_next_cluster(uint32_t cluster, uint32_t *fat, uint32_t fat_length) {
    if(cluster >= (fat_length / 4)) return 0;
    if(fat[cluster] >= 0x0FFFFFF8) return 0;
    return fat[cluster];
}

#define min(a, b) ((a) > (b) ? (b) : (a))
static size_t fat32_strlen_special(char *s) {
    char name[11];
    memset(name, 0, 11);
    memset(name, ' ', 10);

    size_t len = 0;

    bool extension = false;
    for(int i = 0; i < 3; i++) {
        if(s[8+i] && s[8+i] != 0x20) extension = true;
    }

    if (extension) {
        for(int i = 0; i < 8; i++) {
            if(s[i] && s[i] != 0x20) name[len++] = s[i];
        }
        name[len++] = '.';
        for(int i = 0; i < 3; i++) {
            if(s[8+i] && s[8+i] != 0x20) name[len++] = s[8+i];
        }
    } else {
        for(int i = 0; i < 8; i++) {
            if(s[i] && s[i] != 0x20) name[len++] = s[i];
        }
    }

    memcpy(s, name, len);
    return len;
}

static void fat32_index_directories(fat32fs_custom_data_t *customData, uint32_t *fat, uint32_t cluster, uint32_t parent_cluster, uint32_t parentOffset) {
    size_t fat_length = customData->sectors_per_fat * 512;

    static char list_entry_name[512]; //TODO
    bool ready_long_name = false;

    fat32_entry_t entry;
    for(size_t i = 0; fat32_read_fat_entry(customData, cluster, i, &entry); i++) {
        if(i == customData->sectors_per_cluster * 16) {
            cluster = fat32_get_next_cluster(cluster, fat, fat_length);
            i = -1;
            continue;
        }

        if (entry.directory.name[0] == 0xE5) continue;

        if(entry.directory.attribute == 0x0F) {
            //Handle long file names

            uint8_t index = (entry.long_file_name.order & 0x1F) - 1;
            if(entry.long_file_name.order & 0x40) {
                list_entry_name[index * 14] = 0;
                ready_long_name = true;
            }

            for(int j = 0; j < 5; j++) {
                list_entry_name[index * 13 + j] = entry.long_file_name.name1[j*2];
            }
            for(int j = 0; j < 6; j++) {
                list_entry_name[index * 13 + 5 + j] = entry.long_file_name.name2[j*2];
            }
            for(int j = 0; j < 2; j++) {
                list_entry_name[index * 13 + 11 + j] = entry.long_file_name.name3[j*2];
            }
        } else {
            size_t length = 0;
            for(int j = 0; j < parentOffset; j++) printf(" ");
            if(ready_long_name) {
                if(entry.directory.size)
                    printf("%s(size:%d)\n", list_entry_name, entry.directory.size);
                else
                    printf("%s(DIR)\n", list_entry_name);
                length = strlen(list_entry_name);
            } else {
                if (entry.directory.name[0] == 0x05) entry.directory.name[0] = 0xE5;
                length = fat32_strlen_special((char *)entry.directory.name);
                if(entry.directory.size)
                    printf("%*.*s(size:%d)\n", length, length, entry.directory.name, entry.directory.size);
                else
                    printf("%*.*s(DIR)\n", length, length, entry.directory.name);
            }

            ready_long_name = false;

            uint16_t newCluster = ((uint16_t)entry.directory.first_cluster_hi << 16) | entry.directory.first_cluster_lo;
            if(entry.directory.attribute & 0x10 && newCluster != cluster && newCluster != parent_cluster) {
                fat32_index_directories(customData, fat, newCluster, cluster, parentOffset + length + 1);
            }
        }
    }
}

static int fat32_compare_fs_ent_and_path(char *real_name, char *path) {
    char *ext = strchrnul(path, '.');

    char name[12];
    memset(name, 0, 12);
    memset(name, ' ', 11);

    if (*(ext - 1) == '.') {
        memcpy(name, path, min(ext - path - 1, 8));
        memcpy(name + 8, ext, min(strlen(ext), 3));
    } else {
        memcpy(name, path, min(strlen(path), 8));
    }

    for (size_t i = 0; i < 11; i++) {
        char c = name[i];
        if(c >= 'a' && c <= 'z') c -= ('a' - 'A');
        name[i] = c;
    }

    return memcmp(real_name, name, 11);
}

size_t fat32_read(struct vfs_node *node, char *buffer, size_t size) {
    fat32fs_device_t *device = (fat32fs_device_t *)node->data;

    spinlock_lock(&device->customData->lock);

    size_t fat_length = device->customData->sectors_per_fat * 512;

    uint32_t *fat = kmalloc(fat_length);
    read_device_offset(device->customData->deviceNode, SECTOR_TO_OFFSET(device->customData->reserved_sectors), fat, fat_length);

    size_t read_size = size;
    if(node->offset + size >= device->entry.directory.size)
        read_size = device->entry.directory.size - node->offset;

    size_t clusterSize = device->customData->sectors_per_cluster * 512;
    size_t clusterLength  = ((read_size + clusterSize - 1) / clusterSize) * clusterSize;
    size_t clusterOffset = ROUND_DOWN(node->offset, clusterSize) / clusterSize;
    size_t cluster = ((uint16_t)device->entry.directory.first_cluster_hi << 16) | device->entry.directory.first_cluster_lo;
    while(clusterOffset--) cluster = fat32_get_next_cluster(cluster, fat, fat_length);

    size_t savedOffset = node->offset;

    size_t index = 0;
    while(index < read_size) {
        size_t offset = CLUSTER_TO_OFFSET(cluster, device->customData->begin_cluster, device->customData->sectors_per_cluster) + (savedOffset % clusterSize);

        size_t readCount = min(clusterSize - (savedOffset % clusterSize), read_size - index);
        savedOffset = 0;

        read_device_offset(device->customData->deviceNode, offset, (buffer + index), readCount);

        index += readCount;

        if(index < read_size) {
            cluster = fat32_get_next_cluster(cluster, fat, fat_length);
        }
    }

    kfree(fat);
    node->offset += read_size;

    spinlock_unlock(&device->customData->lock);

    return read_size;
}

size_t fat32_lseek(vfs_node_t *node, size_t offset, size_t type) {
    fat32fs_device_t *device = (fat32fs_device_t *)node->data;

    spinlock_lock(&device->customData->lock);

    size_t length = device->entry.directory.size;

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
            spinlock_unlock(&device->customData->lock);
            return -1;
    }

    spinlock_unlock(&device->customData->lock);
    return node->offset;
}

vfs_node_t *fat32_finddir(vfs_node_t *node_dev, char *name, char **last_path) {
    fat32fs_custom_data_t *customData = (fat32fs_custom_data_t *)node_dev->data;

    spinlock_lock(&customData->lock);

    char path[1024];
    if(strlen(*last_path)) {
        sprintf(path, "%s/%s", name, *last_path);
    } else {
        strcpy(path, name);
    }
    char *pathPtr = (char *)path;

    uint32_t cluster = customData->root_dir_cluster;
    size_t fat_length = customData->sectors_per_fat * 512;

    uint32_t *fat = kmalloc(fat_length);
    read_device_offset(customData->deviceNode, SECTOR_TO_OFFSET(customData->reserved_sectors), fat, fat_length);

    char entry_name[512];
    bool ready_long_name = false;

    size_t counter = 0x40;

    fat32_entry_t entry;
    for(size_t i = 0; fat32_read_fat_entry(customData, cluster, i, &entry); i++) {
        if(i == customData->sectors_per_cluster * 16) {
            cluster = fat32_get_next_cluster(cluster, fat, fat_length);
            i = -1;
            continue;
        }

        if (entry.directory.name[0] == 0xE5) continue;

        if(entry.directory.attribute == 0x0F) {
            //Handle long file names

            uint8_t index = (entry.long_file_name.order & 0x1F) - 1;
            if(entry.long_file_name.order & 0x40) {
                entry_name[index * 14] = 0;
                ready_long_name = true;
            }

            for(int j = 0; j < 5; j++) {
                entry_name[index * 13 + j] = entry.long_file_name.name1[j*2];
            }
            for(int j = 0; j < 6; j++) {
                entry_name[index * 13 + 5 + j] = entry.long_file_name.name2[j*2];
            }
            for(int j = 0; j < 2; j++) {
                entry_name[index * 13 + 11 + j] = entry.long_file_name.name3[j*2];
            }
        } else {
            /*if(ready_long_name) {
                printf("%s %d %x\n", entry_name, cluster, entry.directory.attribute);
            } else {
                if (entry.directory.name[0] == 0x05) entry.directory.name[0] = 0xE5;
                printf("%11.11s %d %x\n", entry.directory.name, cluster, entry.directory.attribute);
            }*/

            char *slash = strchrnul(pathPtr, '/');

            char subPath[1024] = {0};
            bool pathIsAFile = false;
            if(*slash == '\0') {
                memcpy(subPath, pathPtr, strlen(pathPtr));
                pathIsAFile = true;
            } else {
                memcpy(subPath, pathPtr, slash - pathPtr);
                subPath[slash - pathPtr] = '\0';
            }

            bool match = false;
            if(ready_long_name) {
                match = !strcmp(entry_name, subPath);
            } else {
                match = !fat32_compare_fs_ent_and_path((char *)entry.directory.name, subPath);
            }

            uint32_t newCluster = ((uint32_t)entry.directory.first_cluster_hi << 16) | entry.directory.first_cluster_lo;
            if(entry.directory.attribute & 0x20 && pathIsAFile && match) {
                /*if(ready_long_name) {
                    printf("Found %s\n", entry_name);
                } else {
                    printf("Found %.8s.%.3s\n", entry.directory.name, &entry.directory.name[8]);
                }*/

                vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
                fat32fs_device_t *device = (fat32fs_device_t *)kmalloc(sizeof(fat32fs_device_t));
                device->customData = customData;
                device->entry = entry;

                memcpy(node->name, path, 1024);
                node->flags |= VFS_FILE;
                node->data = (void *)device;
                node->functions.read = fat32_read;
                node->functions.lseek = fat32_lseek;

                kfree(fat);

                *last_path = NULL; //Null it out, to indicate we handled the directory finding

                spinlock_unlock(&customData->lock);
                return node;
            } else if(entry.directory.attribute & 0x10 && !pathIsAFile && match) {
                cluster = newCluster;
                i = 0;
                pathPtr = slash + 1;
            }

            ready_long_name = false;
            memset(entry_name, 0, 512);
        }
    }


    kfree(fat);

    spinlock_unlock(&customData->lock);
    return NULL;
}

void fat32_print_tree(struct vfs_node *node, size_t parentOffset) {
    fat32fs_custom_data_t *customData = (fat32fs_custom_data_t *)node->data;

    spinlock_lock(&customData->lock);

    uint32_t cluster = customData->root_dir_cluster;
    size_t fat_length = customData->sectors_per_fat * 512;

    uint32_t *fat = kmalloc(fat_length);
    read_device_offset(customData->deviceNode, SECTOR_TO_OFFSET(customData->reserved_sectors), fat, fat_length);

    fat32_index_directories(customData, fat, cluster, 0, parentOffset);

    kfree(fat);

    spinlock_unlock(&customData->lock);
}

size_t fat32_mount(struct vfs_node *node, char *device, size_t flags, char *path, void *data) {
    vfs_node_t *deviceNode = kopen(device, flags);
    if(deviceNode == NULL)
        return -1;

    fat32fs_custom_data_t *customData = (fat32fs_custom_data_t *)kmalloc(sizeof(fat32fs_custom_data_t));
    customData->deviceNode = deviceNode;

    read_device_offset(deviceNode, 0x0B, &customData->sector_length, 2);
    read_device_offset(deviceNode, 0x0D, &customData->sectors_per_cluster, 1);
    read_device_offset(deviceNode, 0x0E, &customData->reserved_sectors, 2);
    read_device_offset(deviceNode, 0x10, &customData->fat_count, 1);
    read_device_offset(deviceNode, 0x24, &customData->sectors_per_fat, 4);
    read_device_offset(deviceNode, 0x2C, &customData->root_dir_cluster, 4);

    uint16_t signature = 0;
    read_device_offset(deviceNode, 0x1FE, &signature, 2);
    if(signature != 0xAA55) {
        printf("[FAT32] Wrong signature\n");

        kfree(customData);
        vfs_close(deviceNode);
        kfree(deviceNode);

        return -1;
    }

    customData->lock = INIT_SPINLOCK();
    customData->begin_cluster = customData->reserved_sectors + customData->fat_count * customData->sectors_per_fat;

    vfs_node_t *fat32Node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    strcpy(fat32Node->name, "Fat32");
    fat32Node->data = (void *)customData;
    fat32Node->functions.finddir = fat32_finddir;
    fat32Node->functions.print_tree = fat32_print_tree;
    fat32Node->flags |= VFS_DIRECTORY;
    vfs_mount(path, fat32Node);

    return -1;
}

void fat32_init() {
    devfs_node_t *fat32Node = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
    strcpy(fat32Node->name, "Fat32");
    fat32Node->flags |= VFS_FILESYSTEM;
    fat32Node->functions.mount = fat32_mount;
    devfs_mount("fat32", fat32Node);
}