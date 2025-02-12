#include "virtiogpu.h"
#include "../virtio/virtio.h"

#include <tasking/scheduler.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <cpu/cpu.h>

#include <fs/devfs/devfs.h>

#include <utils/lists.h>

static pci_device_t *VirtioGpuDevice = NULL;
static volatile virtio_gpu_config_t *VirtioGpuConfig = NULL;

static event_t virtiogpu_event = 0;

static bool virtiogpu_event_handler(irq_regs_t *regs, size_t queue, size_t descriptor) {
    if(queue == -1 || descriptor == -1) {
        __asm__ __volatile__ ("" ::: "memory");
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        if(VirtioGpuConfig->events_read) {
            __asm__ __volatile__ ("" ::: "memory");
            __atomic_thread_fence(__ATOMIC_SEQ_CST);
            VirtioGpuConfig->events_clear = VirtioGpuConfig->events_read;

            //printf("Event received\n");

            event_notify(&virtiogpu_event);

            return true;
        }
    }

    return false;
}

static size_t send_command_response(const void *cmd, size_t cmdLength, void **_res, size_t resLength) {
    uint16_t i = 0;
    virtio_queue_descriptor_t *descriptor = virtio_allocate_descriptors(VirtioGpuDevice, 0, 2, &i);

    descriptor->address = (uint64_t)((size_t)cmd - MEM_PHYS_OFFSET);
    descriptor->length = cmdLength;
    descriptor->flags |= VIRTQ_DESC_F_NEXT;

    uint16_t next = descriptor->next;
    descriptor = virtio_get_next_descriptor(VirtioGpuDevice, 0, i);

    void *res = kmalloc(resLength);
    *_res = res;

    descriptor->address = (uint64_t)((size_t)res - MEM_PHYS_OFFSET);
    descriptor->length = resLength;
    descriptor->flags |= VIRTQ_DESC_F_WRITE;

    //printf("Sending\n");
    virtio_submit_descriptor(VirtioGpuDevice, 0, i, next);
    virtio_free_descriptor(VirtioGpuDevice, 0, next);

    return 0;
}

static size_t send_command_response_cursor(const void *cmd, size_t cmdLength, void **_res, size_t resLength) {
    uint16_t i = 0;
    virtio_queue_descriptor_t *descriptor = virtio_allocate_descriptors(VirtioGpuDevice, 1, 2, &i);

    descriptor->address = (uint64_t)((size_t)cmd - MEM_PHYS_OFFSET);
    descriptor->length = cmdLength;
    descriptor->flags |= VIRTQ_DESC_F_NEXT;

    uint16_t next = descriptor->next;
    descriptor = virtio_get_next_descriptor(VirtioGpuDevice, 1, i);

    void *res = kmalloc(resLength);
    *_res = res;

    descriptor->address = (uint64_t)((size_t)res - MEM_PHYS_OFFSET);
    descriptor->length = resLength;
    descriptor->flags |= VIRTQ_DESC_F_WRITE;

    memcpy(res, cmd, resLength);

    //printf("Sending\n");
    virtio_submit_descriptor(VirtioGpuDevice, 1, i, next);
    virtio_free_descriptor(VirtioGpuDevice, 1, next);

    return 0;
}

static volatile uint32_t *cframebuffer = NULL;
static void virtiogpu_change_handler(void *arg) {
    while(1) {
        if(event_await(&virtiogpu_event)) {
        }
    }
}

static virtgpu_node_reference_t *gpu_get_reference(vfs_node_t *node) {
    if(!node->data) return NULL;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    ktid_t tid = CPULocals[CurrentCPU].currentTid;
    spinlock_unlock(&SchedulerLock);

    for(size_t i = 0; i < ((list_t *)node->data)->length; i++) {
        virtgpu_node_reference_t *reference = list_get_indexed(node->data, i)->value;
        if(reference->pid == pid && reference->tid == tid) {
            return reference;
        }
    }

    return NULL;
}

static size_t gpu_open(vfs_node_t *node, uint32_t flags) {
    list_t *gpuList = (list_t *)node->data;

    if(gpu_get_reference(node) != NULL) {
        printf("RRR GPU\n");
        return -1;
    }

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    ktid_t tid = CPULocals[CurrentCPU].currentTid;
    spinlock_unlock(&SchedulerLock);

    virtgpu_node_reference_t *reference = kmalloc(sizeof(virtgpu_node_reference_t));
    reference->pid = CPULocals[CurrentCPU].currentPid;
    reference->tid = CPULocals[CurrentCPU].currentTid;
    reference->resources = list_create();
    reference->ctxs = list_create();
    reference->ctxAllocator = 1;

    virtgpu_ctx_t *default_ctx = kmalloc(sizeof(virtgpu_ctx_t));
    default_ctx->id = 0;
    default_ctx->resourceAllocator = 15;
    list_push_back(reference->ctxs, default_ctx);
    reference->currentCtx = default_ctx;

    list_push_back(gpuList, reference);

    return 0;
}

static uint32_t contextAllocator = 1;

static size_t gpu_ioctl(vfs_node_t *node, size_t request, void *arg) {
    virtgpu_node_reference_t *reference = gpu_get_reference(node);
    if(!reference) return -1;

    switch(request) {
        case VIRTGPU_IOCTL_CREATE_RESOURCE_2D:
            {
                if(!arg || !reference->currentCtx) return -1;

                virtgpu_create_resource_2d_t *gpuResource = (virtgpu_create_resource_2d_t *)arg;

                virtio_gpu_resource_create_2d_t *req = (virtio_gpu_resource_create_2d_t *)kmalloc(sizeof(virtio_gpu_resource_create_2d_t));
                req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
                req->resourceID = reference->currentCtx->resourceAllocator++;
                req->format = gpuResource->format;
                req->width = gpuResource->width;
                req->height = gpuResource->height;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_resource_create_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                virtgpu_resource_t *resource = (virtgpu_resource_t *)kmalloc(sizeof(virtgpu_resource_t));
                resource->id = req->resourceID;
                resource->createCommand = *req;
                list_push_back(reference->resources, resource);

                kfree(res);
                kfree(req);
                return reference->resources->length - 1; //Last length was the index
            }
        case VIRTGPU_IOCTL_GET_DISPLAY_INFO:
            {
                if(!arg) return -1;

                virtio_mode_t *modesArray = (virtio_mode_t *)arg;

                virtio_gpu_ctrl_hdr_t *header = (virtio_gpu_ctrl_hdr_t *)kmalloc(sizeof(virtio_gpu_ctrl_hdr_t));
                header->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;

                virtio_display_info_t *info = NULL;
                send_command_response(header, sizeof(virtio_gpu_ctrl_hdr_t), (void **)&info, sizeof(virtio_display_info_t));

                if(info->header.type != VIRTIO_GPU_RESP_OK_DISPLAY_INFO) {
                    kfree(info);
                    kfree(header);
                    return -1;
                }

                memcpy(modesArray, info->modes, sizeof(virtio_mode_t) * 16);

                kfree(info);
                kfree(header);

                return 16;
            }
        case VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING:
            {
                if(!arg || !reference->currentResource) return -1;

                virtgpu_attach_backing_t *virtioAttachBacking = (virtgpu_attach_backing_t *)arg;

                struct {
                    virtio_gpu_resource_attach_backing_t req;
                    virtio_gpu_mem_entry_t mem;
                } *req = kmalloc(sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t));

                req->req.header.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
                req->req.resourceID = reference->currentResource->id;
                req->req.nrEntries = 1;

                kpid_t pid = scheduler_get_current_pid();

                if(!pid) {
                    req->mem.address = (virtioAttachBacking->address - MEM_PHYS_OFFSET);
                } else {
                    req->mem.address = (uint64_t)vmm_get_phys((void *)SchedulerProcesses[pid]->page_table, (void *)virtioAttachBacking->address);
                }
                req->mem.length = virtioAttachBacking->length;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                reference->currentResource->attachedBacking = req->mem;

                kfree(res);
                kfree(req);
                return 0;
            }
        case VIRTGPU_IOCTL_SET_SCANOUT:
            {
                if(!arg || !reference->currentResource) return -1;

                virtgpu_set_scanout_t *virtioScanout = (virtgpu_set_scanout_t *)arg;

                virtio_gpu_set_scanout_t *req = (virtio_gpu_set_scanout_t *)kmalloc(sizeof(virtio_gpu_set_scanout_t));
                req->header.type = VIRTIO_GPU_CMD_SET_SCANOUT;
                req->rect.x = req->rect.y = 0;
                req->rect.width = virtioScanout->width;
                req->rect.height = virtioScanout->height;
                req->scanoutID = virtioScanout->scanoutID;
                req->resourceID = reference->currentResource->id;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_set_scanout_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                reference->currentResource->currentScanout = *virtioScanout;

                kfree(res);
                kfree(req);
                return 0;
            }
        case VIRTGPU_IOCTL_TRANSFER_AND_FLUSH:
            {
                if(!arg || !reference->currentResource || !reference->currentCtx) return -1;

                virtgpu_transfer_and_flush_t *virtioTransferAndFlush = (virtgpu_transfer_and_flush_t *)arg;
                if(!virtioTransferAndFlush->notTransfer)
                {
                    virtio_gpu_transfer_to_host_2d_t *req = (virtio_gpu_transfer_to_host_2d_t *)kmalloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
                    req->header.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
                    req->rect.x = req->rect.y = 0;
                    req->rect.width = virtioTransferAndFlush->width;
                    req->rect.height = virtioTransferAndFlush->height;
                    req->offset = 0;
                    req->resourceID = reference->currentResource->id;
                    req->header.ctxID = reference->currentCtx->id;

                    virtio_gpu_ctrl_hdr_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_transfer_to_host_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                    if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                        kfree(res);
                        kfree(req);
                        return -1;
                    }

                    kfree(res);
                    kfree(req);
                }

                if(!virtioTransferAndFlush->notFlush)
                {
                    virtio_gpu_resource_flush_t *req = (virtio_gpu_resource_flush_t *)kmalloc(sizeof(virtio_gpu_resource_flush_t));
                    req->header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
                    req->rect.x = req->rect.y = 0;
                    req->rect.width = virtioTransferAndFlush->width;
                    req->rect.height = virtioTransferAndFlush->height;
                    req->resourceID = reference->currentResource->id;
                    req->header.ctxID = reference->currentCtx->id;

                    virtio_gpu_ctrl_hdr_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_resource_flush_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                    if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                        kfree(res);
                        kfree(req);
                        return -1;
                    }

                    kfree(res);
                    kfree(req);

                    return 0;
                }
            }
        case VIRTGPU_IOCTL_SET_RESOURCE_ID:
            {
                if(!arg) return -1;

                uint32_t *index = arg;

                list_node_t *resourceNode = list_get_indexed(reference->resources, *index);
                if(!resourceNode) return -1;

                reference->currentResource = (virtgpu_resource_t *)resourceNode->value;

                return 0;
            }
        case VIRTGPU_IOCTL_SET_CONTEXT_ID:
            {
                if(!arg) return -1;

                uint32_t *index = arg;

                list_node_t *ctxNode = list_get_indexed(reference->resources, *index);
                if(!ctxNode) return -1;

                reference->currentCtx = (virtgpu_ctx_t *)ctxNode->value;

                return 0;
            }
        case VIRTGPU_IOCTL_UPDATE_CURSOR:
            {
                if(!arg || !reference->currentResource) return -1;

                virtgpu_update_cursor_t *virtioUpdateCursor = (virtgpu_update_cursor_t *)arg;

                list_node_t *resourceNode = list_get_indexed(reference->resources, virtioUpdateCursor->resourceID);
                if(!resourceNode) return -1;

                virtio_gpu_update_cursor_req_t *req = (virtio_gpu_update_cursor_req_t *)kmalloc(sizeof(virtio_gpu_update_cursor_req_t));
                req->header.type = VIRTIO_GPU_CMD_UPDATE_CURSOR;
                req->position.scanoutID = reference->currentResource->currentScanout.scanoutID;
                req->position.x = virtioUpdateCursor->x;
                req->position.y = virtioUpdateCursor->y;
                req->hotX = virtioUpdateCursor->hotX;
                req->hotY = virtioUpdateCursor->hotY;
                req->resourceID = ((virtgpu_resource_t *)resourceNode->value)->id;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response_cursor(req, sizeof(virtio_gpu_update_cursor_req_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                kfree(res);
                kfree(req);
                return 0;
            }
        default:
            return -1;
    }
}

static size_t _init() {
    VirtioGpuDevice = pci_get_device_by_vendor(0x1AF4, 0x1050, 0);
    if(VirtioGpuDevice) {
        printf("VirtioGPU Starting\n");

        virtio_device_add(VirtioGpuDevice, 0x100000001, virtiogpu_event_handler);

        virtio_setup_queue(VirtioGpuDevice, 0);
        virtio_setup_queue(VirtioGpuDevice, 1);

        VirtioGpuConfig = (volatile virtio_gpu_config_t *)virtio_get_config(VirtioGpuDevice);

        __attribute__((unused)) virtio_mode_t currentMode = {0};
        __attribute__((unused)) uint8_t currentModeNumber = 0;

        {
            virtio_gpu_ctrl_hdr_t *header = (virtio_gpu_ctrl_hdr_t *)kmalloc(sizeof(virtio_gpu_ctrl_hdr_t));
            header->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;

            virtio_display_info_t *info = NULL;
            send_command_response(header, sizeof(virtio_gpu_ctrl_hdr_t), (void **)&info, sizeof(virtio_display_info_t));

            for(size_t i = 0; i < 16; i++) {
                if(info->modes[i].enabled) {
                    printf("%u: x %u, y %u, w %u, h %u, flags 0x%x\n", i, info->modes[i].rect.x, info->modes[i].rect.y, info->modes[i].rect.width, info->modes[i].rect.height, info->modes[i].flags);
                    currentMode = *(&info->modes[i]);
                    currentModeNumber = i;
                }
            }

            kfree(header);
            kfree(info);
        }

        { // Cursor
            virtio_gpu_resource_create_2d_t *req = (virtio_gpu_resource_create_2d_t *)kmalloc(sizeof(virtio_gpu_resource_create_2d_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
            req->resourceID = 2;
            req->format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
            req->width = currentMode.rect.width;
            req->height = currentMode.rect.height;
            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

            printf("Type: %x\n", res->type);
            kfree(req);
            kfree(res);
        }
        volatile uint32_t *cframebuffer = NULL;
        {
            struct {
                virtio_gpu_resource_attach_backing_t req;
                virtio_gpu_mem_entry_t mem;
            } __attribute__((packed)) *req = kmalloc(sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t));

            req->req.header.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
            req->req.resourceID = 2;
            req->req.nrEntries = 1;
            size_t pageslength = currentMode.rect.width * currentMode.rect.height * 16;
            cframebuffer = (uint32_t *)kmalloc(pageslength);

            req->mem.address = ((uint64_t)cframebuffer - MEM_PHYS_OFFSET);
            req->mem.length = pageslength;
            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

            printf("Type: %x\n", res->type);
            kfree(req);
            kfree(res);
        }

        for(size_t i = 0; i < currentMode.rect.height; i++) {
            for(size_t j = 0; j < currentMode.rect.width; j++) {
                volatile uint32_t *pixel = &cframebuffer[j + i * currentMode.rect.width];
                *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
            }
        }

        {
            virtio_gpu_set_scanout_t *req = (virtio_gpu_set_scanout_t *)kmalloc(sizeof(virtio_gpu_set_scanout_t));
            req->header.type = VIRTIO_GPU_CMD_SET_SCANOUT;
            req->rect.x = req->rect.y = 0;
            req->rect.width = currentMode.rect.width;
            req->rect.height = currentMode.rect.height;
            req->resourceID = 2;
            req->scanoutID = currentModeNumber;
            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_set_scanout_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

            printf("Type: %x\n", res->type);
            kfree(req);
            kfree(res);
        }

        {
            virtio_gpu_transfer_to_host_2d_t *req = (virtio_gpu_transfer_to_host_2d_t *)kmalloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
            req->header.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
            req->rect.x = req->rect.y = 0;
            req->rect.width = currentMode.rect.width;
            req->rect.height = currentMode.rect.height;
            req->offset = 0;
            req->resourceID = 2;
            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_transfer_to_host_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

            printf("Type: %x\n", res->type);
            kfree(req);
            kfree(res);
        }

        {
            virtio_gpu_resource_flush_t *req = (virtio_gpu_resource_flush_t *)kmalloc(sizeof(virtio_gpu_resource_flush_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
            req->rect.x = req->rect.y = 0;
            req->rect.width = currentMode.rect.width;
            req->rect.height = currentMode.rect.height;
            req->resourceID = 2;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_flush_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

            printf("Type: %x\n", res->type);
            kfree(req);
            kfree(res);
        }

        printf("Capsets supported: %d\n", VirtioGpuConfig->num_caspsets);
        {
            uint32_t version = 2;
            uint32_t size = 0;
            for(uint32_t i = 0; i < VirtioGpuConfig->num_caspsets; i++) {
                {
                    virtio_gpu_get_capset_info_req_t *req = (virtio_gpu_get_capset_info_req_t *)kmalloc(sizeof(virtio_gpu_get_capset_info_req_t));
                    req->header.type = VIRTIO_GPU_CMD_GET_CAPSET_INFO;
                    req->capsetIndex = i;

                    virtio_gpu_get_capset_info_res_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_get_capset_info_req_t), (void **)&res, sizeof(virtio_gpu_get_capset_info_res_t));

                    if(res->capsetID == 2) {
                        size = res->capsetMaxSize;
                    }

                    printf("Capset #%u: id 0x%X, maxVersion 0x%X, maxSize 0x%X\n", i, res->capsetID, res->capsetMaxVersion, res->capsetMaxSize);
                    kfree(req);
                    kfree(res);
                }
            }

            {
                virtio_gpu_get_capset_t *req = (virtio_gpu_get_capset_t *)kmalloc(sizeof(virtio_gpu_get_capset_t));
                req->header.type = VIRTIO_GPU_CMD_GET_CAPSET;
                req->capsetIndex = 2;
                req->capsetVersion = version;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_get_capset_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t) + size);

                if(res->type == VIRTIO_GPU_RESP_OK_CAPSET) {
                    uint8_t *data = (uint8_t *)((uint64_t)res + sizeof(virtio_gpu_ctrl_hdr_t));
                    size_t x = 0;
                    while(x < size) {
                        for(int i = 0; i < 16; i++) {
                            if(x >= size) {
                                printf("\n");
                                break;
                            }
                            if(i == 15)
                                printf("%02.2X\n", data[x] & 0xFF);
                            else
                                printf("%02.2X, ", data[x] & 0xFF);
                            x++;
                        }
                    }
                }

                printf("Type: %x\n", res->type);
                kfree(req);
                kfree(res);
            }
        }

        //kpid_t pid = scheduler_get_current_pid();
        //ktid_t tid = thread_create(pid, thread_parameter_exec, thread_entry_data((void *)virtiogpu_change_handler, NULL));
        //scheduler_add_task(pid, tid);

        //while(*(volatile uint32_t *)VirtioGpuConfig == 0); -> TODO: Create

        devfs_node_t *gpuNode = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
        strcpy(gpuNode->name, "VirtIOGPU");
        gpuNode->functions.ioctl = gpu_ioctl;
        gpuNode->functions.open = gpu_open;
        gpuNode->flags |= VFS_FILE;
        gpuNode->data = list_create();

        devfs_mount("gpu", gpuNode);

        printf("Inited virtiogpu\n");
    }
    return 1;
}

static size_t _exit() {
    printf("Goodbye :D\n");

    return 1;
}

MODULE_DEPENDS(virtio);
MODULE_DEF(virtiogpu, _init, _exit);