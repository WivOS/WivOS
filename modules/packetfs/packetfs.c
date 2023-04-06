#include <modules/modules.h>
#include <utils/common.h>

#include <fs/devfs/devfs.h>
#include <fs/pipe/pipe.h>

#include "packetfs.h"

//#define _DEBUG_PACKETFS_

static size_t packetfs_receive_packet(vfs_node_t *socket, pmgr_packet_t **out) {
    pmgr_packet_t temp;
    size_t retValue = vfs_read(socket, (char *)&temp, sizeof(pmgr_packet_t));
    if((int64_t)retValue < 0) return -1;

    *out = (pmgr_packet_t *)kmalloc(temp.size + sizeof(pmgr_packet_t));
    memcpy((void *)*out, &temp, sizeof(pmgr_packet_t));

    if(temp.size) retValue = vfs_read(socket, (char *)(*out)->data, temp.size);

    if((int64_t)retValue < 0) return -1;

    return sizeof(pmgr_packet_t) + temp.size;
}

static size_t packetfs_send_to_server(pmgr_instance_t *instance, pmgr_client_t *client, size_t size, char *data) {
    size_t packetSize = size + sizeof(pmgr_packet_t);
    pmgr_packet_t *packet = (pmgr_packet_t *)kmalloc(packetSize);

    packet->source = client;
    packet->size = size;

    if(size) memcpy(packet->data, data, size);

    size_t ret = vfs_write(instance->server_pipe, (char *)packet, packetSize);
    kfree(packet);

    return ret;
}

static size_t packetfs_send_to_client(pmgr_instance_t *instance, pmgr_client_t *client, size_t size, char *data) {
    size_t packetSize = size + sizeof(pmgr_packet_t);

    pmgr_packet_t *packet = (pmgr_packet_t *)kmalloc(packetSize);

    memcpy(packet->data, data, size);
    packet->source = NULL;
    packet->size = size;

    size_t ret = vfs_write(client->pipe, (char *)packet, packetSize);
    kfree(packet);

    return ret;
}

size_t packetfs_server_read(vfs_node_t *node, char *buffer, size_t size) {
    pmgr_instance_t *instance = (pmgr_instance_t *)node->data;
    if(!instance) return -1;

    pmgr_packet_t *packet;
    if((int64_t)packetfs_receive_packet(instance->server_pipe, &packet) < 0) {
        printf("[PaketFS] Failed to read Packet: Generic Error\n");
        return -1;
    }

    if((packet->size + sizeof(pmgr_packet_t)) > size) {
        printf("[PaketFS] Failed to read Packet: Size mismatch\n");
        return -1;
    }

    memcpy(buffer, packet, packet->size + sizeof(pmgr_packet_t));
    size_t out = packet->size + sizeof(pmgr_packet_t);

    //Send to server
#ifdef _DEBUG_PACKETFS_
    printf("[PaketFS] Server received packet of size %d, maximum awaited: %d\n", packet->size, size - sizeof(pmgr_packet_t));
#endif
    kfree(packet);

    return out;
}

size_t packetfs_server_write(vfs_node_t *node, char *buffer, size_t size) {
    pmgr_instance_t *instance = (pmgr_instance_t *)node->data;
    if(!instance || !buffer) return -1;

    pmgr_header_t *header = (pmgr_header_t *)buffer;
    if((size - sizeof(pmgr_header_t)) > MAX_PACKET_SIZE) return -1;

    if(header->target == NULL) {
#ifdef _DEBUG_PACKETFS_
        printf("[PaketFS] Sending packet of size %d to clients\n", size);
#endif
        spinlock_lock(&instance->lock);

        foreach(cnode, instance->clients) {
            packetfs_send_to_client(instance, (pmgr_client_t *)cnode->value, size - sizeof(pmgr_header_t), (char *)header->data);
        }

        spinlock_unlock(&instance->lock);
        return size;
    } else if(header->target->parent != instance) {
        return -1;
    }

#ifdef _DEBUG_PACKETFS_
    printf("[PaketFS] Sending packet of size %d to client 0x%llX\n", size, header->target);
#endif
    return packetfs_send_to_client(instance, header->target, size - sizeof(pmgr_header_t), (char *)header->data);
}

size_t packetfs_client_read(vfs_node_t *node, char *buffer, size_t size) {
    pmgr_client_t *client = (pmgr_client_t *)node->internIndex;
    pmgr_instance_t *instance = (pmgr_instance_t *)node->data;
    if(!client || client->parent != instance) return -1;

    pmgr_packet_t *packet;
    if((int64_t)packetfs_receive_packet(client->pipe, &packet) < 0) return -1;

    if(packet->size > size) return -1;

    memcpy(buffer, packet->data, packet->size);
    size_t out = packet->size;
    kfree(packet);

    //Send to server
#ifdef _DEBUG_PACKETFS_
    printf("[PaketFS] Client received packet of size %d\n", out);
#endif

    return out;
}

size_t packetfs_client_write(vfs_node_t *node, char *buffer, size_t size) {
    pmgr_client_t *client = (pmgr_client_t *)node->internIndex;
    pmgr_instance_t *instance = (pmgr_instance_t *)node->data;
    if(!client || client->parent != instance) return -1;

    if(size > MAX_PACKET_SIZE) return -1;

    //Send to server
#ifdef _DEBUG_PACKETFS_
    printf("[PaketFS] Sending packet of size %d to parent\n", size);
#endif
    if((int64_t)packetfs_send_to_server(instance, client, size, buffer) < 0) {
        return -1;
    }

    return size;
}

size_t packetfs_open(vfs_node_t *node, uint32_t flags) {
    pmgr_instance_t *instance = (pmgr_instance_t *)node->data;

    if((flags & O_CREAT) && instance->new) {
        instance->new = false;

        node->functions.read = packetfs_server_read;
        node->functions.write = packetfs_server_write;
        //TODO IOCTL

#ifdef _DEBUG_PACKETFS_
        printf("[PaketFS] Server opened: %s\n", instance->name);
#endif
    } else if(!(flags & O_CREAT)) {
        pmgr_client_t *client = (pmgr_client_t *)kmalloc(sizeof(pmgr_client_t));
        client->parent = instance;

        vfs_node_t *pipes[2];
        pipe_create(pipes);

        client->pipe = pipes[0];
        kfree(pipes[1]);

        node->internIndex = (size_t)client;
        node->functions.read = packetfs_client_read;
        node->functions.write = packetfs_client_write;
        //TODO IOCTL, CLOSE

        list_push_back(instance->clients, client);

#ifdef _DEBUG_PACKETFS_
        printf("[PaketFS] Client opened: %s:0x%llX\n", instance->name, node->internIndex);
#endif
    } else {
        return -1;
    }

    return 0;
}

static vfs_node_t *packetfs_finddir(vfs_node_t *node, char *name, char **path) {
    if(!name) return NULL;

    pmgr_t *pmgr = (pmgr_t *)node->data;

    spinlock_lock(&pmgr->lock);

    foreach(fnode, pmgr->filesystem) {
        pmgr_instance_t *instance_t = (pmgr_instance_t *)fnode->value;

        if(!strcmp(name, instance_t->name)) {
            vfs_node_t *inode = (vfs_node_t *)kmalloc(sizeof(vfs_node_t *));
            strcpy(inode->name, instance_t->name);
            inode->functions.open = packetfs_open; //Next is defined on open
            inode->data = (void *)instance_t;

            spinlock_unlock(&pmgr->lock);
            return inode;
        }
    }

    spinlock_unlock(&pmgr->lock);

    return NULL;
}

static vfs_node_t *packetfs_create(vfs_node_t *node, char *name, uint32_t flags) {
    if(!name) return NULL;

    pmgr_t *pmgr = (pmgr_t *)node->data;

    spinlock_lock(&pmgr->lock);

    foreach(fnode, pmgr->filesystem) {
        pmgr_instance_t *instance_t = (pmgr_instance_t *)fnode->value;

        if(!strcmp(name, instance_t->name)) {
            spinlock_unlock(&pmgr->lock);
            return NULL; //What
        }
    }

    pmgr_instance_t *instance = (pmgr_instance_t *)kmalloc(sizeof(pmgr_instance_t));
    instance->name = strdup(name);
    instance->new = true;
    instance->clients = list_create();

    vfs_node_t *pipes[2];
    pipe_create(pipes);

    instance->server_pipe = pipes[0];
    kfree(pipes[1]);

    instance->lock = INIT_SPINLOCK();

    vfs_node_t *inode = (vfs_node_t *)kmalloc(sizeof(vfs_node_t *));
    strcpy(inode->name, instance->name);
    inode->functions.open = packetfs_open; //Next is defined on open
    inode->data = (void *)instance;

    list_push_back(pmgr->filesystem, instance);

    spinlock_unlock(&pmgr->lock);

    return inode;
}

static size_t _init() {
    vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(devfs_node_t));
    strcpy(node->name, "Packet manager");
    node->flags |= VFS_DIRECTORY;
    node->functions.create = packetfs_create;
    node->functions.finddir = packetfs_finddir;

    pmgr_t *pmgr = (pmgr_t *)kmalloc(sizeof(pmgr_t));
    pmgr->filesystem = list_create();
    pmgr->lock = INIT_SPINLOCK();
    node->data = (void *)pmgr;

    vfs_mount("/dev/pmgr", node);

    /*vfs_node_t *testNode = kopen("/dev/pmgr/test", O_CREAT);
    vfs_node_t *testNode2 = kopen("/dev/pmgr/test", 0);

    uint8_t testBuffer[1024];
    pmgr_header_t *testHeader = (pmgr_header_t *)testBuffer;

    testHeader->target = NULL;
    strcpy((char *)testHeader->data, "Hola\0");
    vfs_write(testNode, (char *)testBuffer, sizeof(pmgr_header_t) + 5);

    size_t size = vfs_read(testNode2, (char *)testBuffer, 1024);
    printf("[PaketFS] Test Received: %.*s\n", size, (char *)testBuffer);*/
    //Yeah

    return 0;
}

static size_t _exit() {
    return 0;
}

MODULE_DEF(packetfs, _init, _exit);