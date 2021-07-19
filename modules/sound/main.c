#include "sound.h"
#include <acpi/apic.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <proc/smp.h>
#include <fs/devfs/devfs.h>
#include <util/list.h>

static list_t *soundDeviceList;
static list_t *clients;

static int64_t sound_get_buffer_available(dspNode_t *node) {
    if(node->readPos == node->writePos) return INTERNALBUFFER_SIZE - 1;
    if(node->readPos > node->writePos) return (INTERNALBUFFER_SIZE - node->readPos) + node->writePos;
    else return node->writePos - node->readPos;
}

static void sound_write_buffer(dspNode_t *node, uint8_t value) {
    node->internalBuffer[node->writePos];
    node->writePos++;
    if(node->writePos >= INTERNALBUFFER_SIZE) node->writePos = 0;
}

static void sound_open(vfs_node_t *file, uint32_t flags) {

}

static size_t sound_read(vfs_node_t *file, char *buffer, size_t size) {
    return -1;
}

static size_t sound_write(vfs_node_t *file, char *buffer, size_t size) {
    dspNode_t *node = file->device;
    
    //Spinlock here
    size_t written = 0;
    while(written < size) {
        while(sound_get_buffer_available(node) > 0 && written < size) {
            sound_write_buffer(node, buffer[written]);
            written++;
        }

        //We ran out of buffer, wait for it to be higher than 0
        while(sound_get_buffer_available(node) <= 0) {
            yield(); //yield thread until we have more buffer available
        }
    }
    //Spinlock end here

    return -1;
}

static size_t nextId;

void register_sound_device(sound_device_t *device) {
    device->id = nextId++;

    list_insert_back(soundDeviceList, device);
}

size_t request_sound_buffer(uint8_t *buffer, size_t size) {
    foreach(client, clients) {
        dspNode_t *node = client->val;
        uint32_t byteAvailableForRead = INTERNALBUFFER_SIZE - sound_get_buffer_available(node);
        byteAvailableForRead = (byteAvailableForRead > size) ? size : byteAvailableForRead;
        while(byteAvailableForRead) {

        }
    }
}

static int _init() {
    soundDeviceList = list_create();
    clients = list_create();
    
    devfs_node_t *sound = (devfs_node_t *)kcalloc(sizeof(devfs_node_t *), 1);
    strcpy(sound->name, "BadSoundSystem");
    sound->functions.write = sound_write;
    sound->functions.read = sound_read;
    sound->functions.ioctl = gpu_ioctl;
    sound->functions.open = sound_open;
    sound->flags |= FS_FILE;

    sound->device = list_create();

    devfs_mount("dsp", sound);

    return 0;
}

static int _exit() {
    return 0;
}

MODULE_DEF(sound, _init, _exit);
