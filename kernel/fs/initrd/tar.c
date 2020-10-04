#include "tar.h"
#include <fs/vfs.h>

static tar_header_t **headers;
static size_t numHeaders = 30;

static size_t getTarSize(const char *in)
{
 
    size_t size = 0;
    size_t j;
    size_t count = 1;
 
    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);
 
    return size;
 
}

void initrd_open() {
    return;
}

size_t initrd_write(vfs_node_t *file, char *buffer, size_t size) {
    return -1;
}

size_t initrd_read(vfs_node_t *file, char *buffer, size_t size) {
    size_t *index = file->device;
    tar_header_t *header = headers[*index];
    size_t tarSize = getTarSize(header->size);
    if((size + file->offset) <= tarSize) {
        memcpy(buffer, (void *)((uint64_t)header + 512 + file->offset), size);
        file->offset += size;
        return size;
    } else {
        memcpy(buffer, (void *)((uint64_t)header + 512 + file->offset), (tarSize - file->offset));
        file->offset = tarSize;
        return (tarSize - file->offset);
    }
}

size_t initrd_fstat(vfs_node_t *file, stat_t *statPtr) {
    size_t *index = file->device;
    tar_header_t *header = headers[*index];
    size_t tarSize = getTarSize(header->size);

    statPtr->st_dev = 1;
    if(file->flags & FS_DIRECTORY) {
        statPtr->st_ino = 1;
    } else {
        statPtr->st_ino = (ino_t)(*index);
    }
    statPtr->st_nlink = 1;
    statPtr->st_uid = 0;
    statPtr->st_gid = 0;
    statPtr->st_rdev = 0;
    statPtr->st_size = tarSize;
    statPtr->st_blksize = 512;
    statPtr->st_blocks = (tarSize + 512 - 1) / 512;
    statPtr->st_atim.tv_sec = 0;
    statPtr->st_atim.tv_nsec = 0;
    statPtr->st_mtim.tv_sec = 0;
    statPtr->st_mtim.tv_nsec = 0;
    statPtr->st_ctim.tv_sec = 0;
    statPtr->st_ctim.tv_nsec = 0;
    statPtr->st_mode = 0;
    if(file->flags & FS_DIRECTORY) {
        statPtr->st_mode |= 0x04000;
    } else {
        statPtr->st_mode |= 0x06000;
    }

    return 0;
}

size_t initrd_lseek(vfs_node_t *file, off_t offset, int type) {
    size_t *index = file->device;
    tar_header_t *header = headers[*index];
    size_t tarSize = getTarSize(header->size);

    if(file->flags & FS_DIRECTORY) {
        return -1;
    }

    switch(type) {
        case SEEK_SET:
            if(offset >= tarSize || offset < 0) goto def;
            file->offset = offset;
            break;
        case SEEK_END:
            if(tarSize + offset > tarSize || tarSize + offset < 0) goto def;
            file->offset = tarSize + offset;
            break;
        case SEEK_CUR:
            if(file->offset + offset >= tarSize || file->offset + offset < 0) goto def;
            file->offset += offset;
            break;
        default:
        def:
            return -1;
    }

    size_t ret = file->offset;
    return ret;
}

vfs_node_t *initrd_finddir(vfs_node_t *parent, char *name) {
    for(size_t i = 0; i < numHeaders; i++) {
        tar_header_t *header = headers[i];

        if(!strcmp(header->filename, name)) {
            vfs_node_t *ret = kcalloc(sizeof(vfs_node_t), 1);

            memcpy(ret->name, name, strlen(name));
            ret->flags |= FS_FILE;
            ret->device = (void *)kmalloc(sizeof(size_t));
            *((size_t *)ret->device) = i; // Set entry to access later

            ret->functions.open = initrd_open;
            ret->functions.write = initrd_write;
            ret->functions.read = initrd_read;
            ret->functions.fstat = initrd_fstat;
            ret->functions.lseek = initrd_lseek;

            return ret;
        }
    }
    return NULL;
}

size_t parseTarInitrd(void *initrd) {
    uint64_t address = (uint64_t)initrd;

    headers = kcalloc(sizeof(tar_header_t **), numHeaders);

    size_t i;
    for(i = 0; ; i++) {
        if(i == numHeaders) {
            numHeaders += 30;
            headers = krealloc((void *)headers, sizeof(tar_header_t **) * numHeaders);
        }
        tar_header_t *header = (tar_header_t *)address;

        if(header->filename[0] == '\0')
            break;

        //printf("%s\n", header->filename);

        size_t size = getTarSize(header->size);
    
        address += ((size / 512) + 1) * 512;

        if(size % 512)
            address += 512;
        
        headers[i] = (tar_header_t *)kmalloc((address - (uint64_t)header));
        memcpy((void *)headers[i], (void *)header, (address - (uint64_t)header));
    }

    numHeaders = i;

    vfs_node_t *rootNode = (vfs_node_t *)kcalloc(sizeof(vfs_node_t *), 1);
    strcpy(rootNode->name, "Initrd");
    rootNode->functions.open = initrd_open;
    rootNode->functions.read = initrd_read;
    rootNode->functions.write = initrd_write;
    rootNode->functions.finddir = initrd_finddir;
    rootNode->functions.fstat = initrd_fstat;
    rootNode->functions.lseek = initrd_lseek;
    rootNode->flags |= FS_DIRECTORY;
    vfs_mount("/", rootNode);

    return i;
}