#include "virtiogpu.h"
#include <acpi/apic.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <proc/smp.h>
#include <fs/devfs/devfs.h>

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

static queue_virtqueue_t **vqs = NULL;
static pci_device_t *virtioDevice = NULL;
static virtio_registers_t *virtioRegs = NULL;
static volatile uint32_t *isrStatus = NULL;
static volatile uint64_t notifyQueue0 = 0;
static volatile uint64_t notifyQueue1 = 0;
static void *addrtemp = NULL;
static volatile uint16_t activeQueue = 0;
static volatile virtio_gpu_config_t *gpuConfig = NULL;
static volatile void *framebuffer = NULL;

static volatile spinlock_t ioEvent = INIT_LOCK();

#define atomic_st_rel(value, rhs) \
    __atomic_store_n((value), (rhs), __ATOMIC_RELEASE)

typedef struct {
    uint32_t pid;
    uint32_t tid;
    uint32_t resourceID;
    uint32_t ctxID;
    size_t commandCount;
    virtgpu_3d_command_t *commandQueue;
} virtgpu_node_reference_t;

static void init_queue(uint16_t index) {
    queue_virtqueue_t *vq = kmalloc(sizeof(queue_virtqueue_t));
    vqs[index] = vq;

    *virtioRegs->queueSelect = index;
    uint32_t queueSize = *virtioRegs->queueSize;

    printf("Queue size: %x\n", queueSize);
    
    uint32_t sizeofDescriptors = (sizeof(queue_descriptor_t) * queueSize);
    uint32_t sizeofQueueAvailable = (3 * sizeof(uint16_t)) + (queueSize * sizeof(uint16_t));
    uint32_t sizeofQueueUsed = (3 * sizeof(uint16_t)) + (queueSize * sizeof(queue_used_element_t));
    uint32_t queuePageCount = ROUND_UP((sizeofDescriptors + sizeofQueueAvailable), PAGE_SIZE) + ROUND_UP(sizeofQueueUsed, PAGE_SIZE);

    vq->descriptors = (queue_descriptor_t *)((size_t)pmm_alloc(2) + VIRT_PHYS_BASE);
    vq->available = (queue_available_t *)((size_t)pmm_alloc(2) + VIRT_PHYS_BASE);
    vq->used = (queue_used_t *)((size_t)pmm_alloc(2) + VIRT_PHYS_BASE);

    for(size_t i = 0; i < queueSize; i++) {
        vq->descriptors[i].next = vq->freeList;
        vq->freeList = i;
        vq->freeCount++;
    }

    vq->queueSize = queueSize;
    vq->queueMask = 0xFF;//(1 << ((2 * 8) - 1 - __builtin_clz(queueSize))) - 1;

    *virtioRegs->queueDesc = ((uint64_t)vq->descriptors - VIRT_PHYS_BASE);
    *virtioRegs->queueGuest = ((uint64_t)vq->available - VIRT_PHYS_BASE);
    *virtioRegs->queueDevice = ((uint64_t)vq->used - VIRT_PHYS_BASE);
    *virtioRegs->queueMsixVector = 0xFFFF;
    *virtioRegs->queueEnable = 1;

    activeQueue |= (1 << index);
}

static queue_descriptor_t *virtio_alloc_desc_chain(queue_virtqueue_t *queue, size_t count, uint16_t *start_index) {
    if(queue->freeCount < count) return NULL;

    volatile queue_descriptor_t *last = NULL;
    uint16_t lastIndex = 0;
    while(count > 0) {
        uint16_t i = queue->freeList;
        volatile queue_descriptor_t *desc = &queue->descriptors[i];

        queue->freeList = desc->next;
        queue->freeCount--;

        if(last) {
            desc->flags = VIRTQ_DESC_F_NEXT;
            desc->next = lastIndex;
        } else {
            desc->flags = 0;
            desc->next = 0;
        }
        last = desc;
        lastIndex = i;
        count--;
    }

    if(start_index) *start_index = lastIndex;

    return (queue_descriptor_t *)last;
}

static void virtio_submit_chain(queue_virtqueue_t *queue, uint16_t descIndex) {
    volatile queue_available_t *avail = queue->available;

    avail->rings[avail->index & queue->queueMask] = descIndex;
    __asm__ __volatile__ ("" ::: "memory");
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    avail->index++;
}


static void virtio_kick(queue_virtqueue_t *queue, uint64_t notifyQueue, uint16_t queueIndex) {
    (void)queue;
    *((volatile uint16_t *)notifyQueue) = queueIndex;
    __asm__ __volatile__ ("" ::: "memory");
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

static size_t send_command_response(const void *cmd, size_t cmdLen, void **_res, size_t resLen) {
    uint16_t i = 0;
    queue_descriptor_t *desc = virtio_alloc_desc_chain(vqs[0], 2, &i);

    desc->address = (uint64_t)((size_t)cmd - VIRT_PHYS_BASE); // TODO: Hacked value
    desc->length = cmdLen;
    desc->flags |= VIRTQ_DESC_F_NEXT;

    desc = (queue_descriptor_t *)&vqs[0]->descriptors[desc->next];

    void *res = kmalloc(resLen);
    memset(res, 0, resLen);
    *_res = res;
    addrtemp = (void *)res;

    desc->address = (uint64_t)((size_t)res - VIRT_PHYS_BASE); // TODO: Hacked value
    desc->length = resLen;
    desc->flags = VIRTQ_DESC_F_WRITE;

    virtio_submit_chain(vqs[0], i);

    virtio_kick(vqs[0], notifyQueue0, 0);

    spinlock_lock(&ioEvent); // TODO: implement proper way to wait to events, if two request are send at the same time one needs to wait to another

    //TODO: Await

    return 0;
}

static size_t send_command_response_cursor(const void *cmd, size_t cmdLen, void **_res, size_t resLen) {
    uint16_t i = 0;
    queue_descriptor_t *desc = virtio_alloc_desc_chain(vqs[1], 2, &i);

    //printf("%x\n", i);

    desc->address = (uint64_t)((size_t)cmd - VIRT_PHYS_BASE); // TODO: Hacked value
    desc->length = cmdLen;
    desc->flags |= VIRTQ_DESC_F_NEXT;

    desc = (queue_descriptor_t *)&vqs[1]->descriptors[desc->next];

    void *res = kmalloc(resLen);
    memset(res, 0, resLen);
    *_res = res;
    addrtemp = (void *)res;

    desc->address = (uint64_t)((size_t)res - VIRT_PHYS_BASE); // TODO: Hacked value
    desc->length = resLen;
    desc->flags = VIRTQ_DESC_F_WRITE;

    virtio_submit_chain(vqs[1], i);

    virtio_kick(vqs[1], notifyQueue1, 1);

    spinlock_lock(&ioEvent); // TODO: implement proper way to wait to events, if two request are send at the same time one needs to wait to another

    //TODO: Await

    return 0;
}

extern idt_fn_t irq_functions[0x40];

static void virtio_irq_driver_callback(uint16_t vq, volatile queue_used_element_t *usedElem) {
    uint16_t i = usedElem->index;
    for(;;) {
        int next;
        volatile queue_descriptor_t *desc = &vqs[vq]->descriptors[i];

        if(desc->flags & VIRTQ_DESC_F_NEXT) {
            next = desc->next;
        } else {
            next = -1;
        }

        //printf("Er: %x\n", i);

        vqs[vq]->descriptors[i].next = vqs[vq]->freeList;
        vqs[vq]->freeList = i;
        vqs[vq]->freeCount++;

        if(next < 0) break;
        i = next;
    }
}

static void irq_handler(irq_regs_t *regs) {
    uint32_t irqStatus = *isrStatus;
    //printf("Status: %x\n", irqStatus);

    if(irqStatus & 0x1) { // Used ring update
        for(size_t r = 0; r < 2; r++) {
            if((activeQueue & (1 << r)) == 0) continue;

            uint16_t currentIndex = vqs[r]->used->index;
            //printf("Er1: %x\n", currentIndex);
            for(uint16_t i = vqs[r]->lastUsed; i != (currentIndex & vqs[r]->queueMask); i = (i + 1) & vqs[r]->queueMask) {
                //printf("Er2: %x\n", i);
                volatile queue_used_element_t *usedElem = (queue_used_element_t *)&vqs[r]->used->rings[i];

                virtio_irq_driver_callback(r, usedElem);

                vqs[r]->lastUsed = (vqs[r]->lastUsed + 1) & vqs[r]->queueMask;
            }
        }
        spinlock_unlock(&ioEvent);
    }

    __asm__ __volatile__ ("" ::: "memory");
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    if(gpuConfig->events_read) {
        __asm__ __volatile__ ("" ::: "memory");
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        gpuConfig->events_clear = gpuConfig->events_read;
    }
}

static size_t gpu_write(vfs_node_t *file, char *buffer, size_t size) {
    return -1;
}

static size_t gpu_read(vfs_node_t *file, char *buffer, size_t size) {

    return -1;
}

static virtgpu_node_reference_t *gpu_get_reference(vfs_node_t *file) {
    if(!file->device) return NULL;

    for(size_t i = 0; i < list_size(file->device); i++) {
        virtgpu_node_reference_t *reference = list_get_node_by_index(file->device, i)->val;
        if(reference->pid == cpuLocals[current_cpu].currentProcess && reference->tid == cpuLocals[current_cpu].currentThread) {
            return reference;
        }
    }

    return NULL;
}

static void gpu_open(vfs_node_t *file, uint32_t flags) {
    list_t *gpuList = (list_t *)file->device;

    if(gpu_get_reference(file) != NULL) {
        printf("RRR GPU\n");
        while(1);
    }

    virtgpu_node_reference_t *reference = kmalloc(sizeof(virtgpu_node_reference_t));
    reference->pid = cpuLocals[current_cpu].currentProcess;
    reference->tid = cpuLocals[current_cpu].currentThread;
    reference->ctxID = 0;
    reference->resourceID = 0;

    list_insert_back(gpuList, reference);
}

static uint32_t resourceAllocator = 15;
static uint32_t contextAllocator = 1;

extern volatile spinlock_t schedulerLock;

static size_t gpu_ioctl(vfs_node_t *file, size_t requestType, void *argp) {
    virtgpu_node_reference_t *reference = gpu_get_reference(file);
    if(!reference) return -1;

    switch(requestType) {
        case VIRTGPU_IOCTL_CREATE_RESOURCE_2D:
            {
                uint32_t currentResource = resourceAllocator++;

                if(!argp) return -1;

                virtgpu_create_resource_2d_t *gpuResource = (virtgpu_create_resource_2d_t *)argp;

                virtio_gpu_resource_create_2d_t *req = (virtio_gpu_resource_create_2d_t *)kmalloc(sizeof(virtio_gpu_resource_create_2d_t));
                req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
                req->resourceID = currentResource;
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
                kfree(res);
                kfree(req);
                return currentResource;
            }

        case VIRTGPU_IOCTL_CREATE_RESOURCE_3D:
            {
                uint32_t currentResource = resourceAllocator++;

                if(!argp) return -1;

                virtgpu_create_resource_3d_t *gpuResource = (virtgpu_create_resource_3d_t *)argp;

                virtio_gpu_resource_create_3d_t *req = (virtio_gpu_resource_create_3d_t *)kmalloc(sizeof(virtio_gpu_resource_create_3d_t));
                req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
                req->resourceID = currentResource;
                req->target = gpuResource->target;
                req->bind = gpuResource->bind;
                req->format = gpuResource->format;
                req->width = gpuResource->width;
                req->height = gpuResource->height;
                req->depth = gpuResource->depth;
                req->arraySize = gpuResource->arraySize;
                req->lastLevel = gpuResource->lastLevel;
                req->nrSamples = gpuResource->nrSamples;
                req->flags = gpuResource->flags;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_resource_create_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }
                kfree(res);
                kfree(req);
                return currentResource;
            }

        case VIRTGPU_IOCTL_GET_DISPLAY_INFO:
            {
                if(!argp) return -1;

                virtio_mode_t *modesArray = (virtio_mode_t *)argp;

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
                if(!argp) return -1;

                virtgpu_attach_backing_t *virtioAttachBacking = (virtgpu_attach_backing_t *)argp;

                struct {
                    virtio_gpu_resource_attach_backing_t req;
                    virtio_gpu_mem_entry_t mem;
                } *req = kmalloc(sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t));
            
                req->req.header.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
                req->req.resourceID = reference->resourceID;
                req->req.nrEntries = 1;
            
                if(!cpuLocals[current_cpu].currentProcess) {
                    req->mem.address = (virtioAttachBacking->address - VIRT_PHYS_BASE);
                } else {
                    req->mem.address = (uint64_t)vmm_get_phys((void *)get_active_process(cpuLocals[current_cpu].currentProcess)->process_pml4, (void *)virtioAttachBacking->address);
                }
                req->mem.length = virtioAttachBacking->length;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                kfree(res);
                kfree(req);

                return 0;
            }

        case VIRTGPU_IOCTL_SET_SCANOUT:
            {
                if(!argp) return -1;

                virtgpu_set_scanout_t *virtioScanout = (virtgpu_set_scanout_t *)argp;

                virtio_gpu_set_scanout_t *req = (virtio_gpu_set_scanout_t *)kmalloc(sizeof(virtio_gpu_set_scanout_t));
                req->header.type = VIRTIO_GPU_CMD_SET_SCANOUT;
                req->rect.x = req->rect.y = 0;
                req->rect.width = virtioScanout->width;
                req->rect.height = virtioScanout->height;
                req->scanoutID = virtioScanout->scanoutID;
                req->resourceID = reference->resourceID;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_set_scanout_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
            
                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                kfree(res);
                kfree(req);

                return 0;
            }

        case VIRTGPU_IOCTL_TRANSFER_AND_FLUSH:
            {
                if(!argp) return -1;

                virtgpu_transfer_and_flush_t *virtioTransferAndFlush = (virtgpu_transfer_and_flush_t *)argp;
                if(!virtioTransferAndFlush->notTransfer)
                {
                    virtio_gpu_transfer_to_host_2d_t *req = (virtio_gpu_transfer_to_host_2d_t *)kmalloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
                    req->header.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
                    req->rect.x = req->rect.y = 0;
                    req->rect.width = virtioTransferAndFlush->width;
                    req->rect.height = virtioTransferAndFlush->height;
                    req->offset = 0;
                    req->resourceID = reference->resourceID;
                    req->header.ctxID = reference->ctxID;

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

                {
                    virtio_gpu_resource_flush_t *req = (virtio_gpu_resource_flush_t *)kmalloc(sizeof(virtio_gpu_resource_flush_t));
                    req->header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
                    req->rect.x = req->rect.y = 0;
                    req->rect.width = virtioTransferAndFlush->width;
                    req->rect.height = virtioTransferAndFlush->height;
                    req->resourceID = reference->resourceID;
                    req->header.ctxID = reference->ctxID;

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
                if(!argp) return -1;

                reference->resourceID = *((uint32_t *)argp);
                return 0;
            }

        case VIRTGPU_IOCTL_SET_CONTEXT_ID:
            {
                if(!argp) return -1;

                reference->ctxID = *((uint32_t *)argp);
                return 0;
            }

        case VIRTGPU_IOCTL_CREATE_GPU_CONTEXT:
            {
                if(!argp) return -1;

                uint32_t currentContext = contextAllocator++;

                virtgpu_create_gpu_context_t *virtioContext = (virtgpu_create_gpu_context_t *)argp;

                virtio_gpu_context_create_t *req = (virtio_gpu_context_create_t *)kmalloc(sizeof(virtio_gpu_context_create_t));
                req->header.type = VIRTIO_GPU_CMD_CTX_CREATE;
                req->header.ctxID = currentContext;
                req->nameLength = virtioContext->nameLength;
                memcpy(req->debugName, virtioContext->name, virtioContext->nameLength > 64 ? 64 : virtioContext->nameLength);

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_context_create_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
            
                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                kfree(res);
                kfree(req);

                return currentContext;
            }

        case VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE:
            {
                if(!argp) return -1;

                virtgpu_3d_command_t *virtioCommand = (virtgpu_3d_command_t *)argp;
                
                reference->commandQueue = krealloc(reference->commandQueue, sizeof(virtio_gpu_submit_3d_t) * ++reference->commandCount);

                reference->commandQueue[reference->commandCount - 1].command = virtioCommand->command;
                reference->commandQueue[reference->commandCount - 1].option = virtioCommand->option;
                reference->commandQueue[reference->commandCount - 1].length = virtioCommand->length;
                reference->commandQueue[reference->commandCount - 1].parameters = (uint32_t *)kmalloc(virtioCommand->length * 4);
                memcpy(reference->commandQueue[reference->commandCount - 1].parameters, virtioCommand->parameters, virtioCommand->length * 4);

                return 0;
            }

        case VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE:
            {
                if(!reference->commandCount) return -1;

                size_t sizeInBytes = 0;
                for(size_t i = 0; i < reference->commandCount; i++) {
                    sizeInBytes += reference->commandQueue[i].length + 1; // Arguments length in uint32_t + one uint32_t for the length and the command id
                }
                sizeInBytes *= 4;

                virtio_gpu_submit_3d_t *req = (virtio_gpu_submit_3d_t *)kmalloc(sizeof(virtio_gpu_submit_3d_t) + sizeInBytes);
                req->header.type = VIRTIO_GPU_CMD_SUBMIT_3D;
                req->header.ctxID = reference->ctxID;
                req->size = sizeInBytes;
                req->header.flags = 1;
                req->header.fenceID = 1;

                size_t offset = 0;
                for(size_t i = 0; i < reference->commandCount; i++) {
                    req->command[offset] = (reference->commandQueue[i].length << 16) | (reference->commandQueue[i].option << 8) | reference->commandQueue[i].command;
                    offset++;
                    memcpy((void *)(((uint64_t)req->command) + offset * 4), reference->commandQueue[i].parameters, reference->commandQueue[i].length * 4);
                    kfree(reference->commandQueue[i].parameters);
                    offset += reference->commandQueue[i].length; // Arguments length in uint32_t + one uint32_t for the length and the command id
                }

                //Free the queue
                reference->commandCount = 0;
                reference->commandQueue = krealloc(reference->commandQueue, reference->commandCount);

                /*memcpy(req->command, virtioSubmit->dwordArray, virtioSubmit->size);

                float floatArray[6] = {
                    1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
                };

                req->command[0] = (1 << 16) | VIRGL_CCMD_CREATE_SUB_CTX;
                req->command[1] = 1;
                req->command[2] = (6 << 16) | VIRGL_CCMD_SET_VIEWPORT_STATE;
                memcpy(&req->command[3], floatArray, 6);
                req->command[9] = (3 << 16) | VIRGL_CCMD_SET_SCISSOR_STATE;
                req->command[10] = 0;
                req->command[11] = (0xffff) | (0xffff << 16);
                req->command[12] = (0x0f0f) | (0xf00f << 16);*/

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_submit_3d_t) + sizeInBytes, (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
                //printf("%x\n", res->type);
            
                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }

                kfree(res);
                kfree(req);

                return 0;
            }

        case VIRTGPU_IOCTL_ATTACH_RESOURCE_3D:
            {
                if(!argp) return -1;

                virtgpu_attach_resource_3d_t *gpuResource = (virtgpu_attach_resource_3d_t *)argp;

                virtio_gpu_ctx_attach_resource_t *req = (virtio_gpu_ctx_attach_resource_t *)kmalloc(sizeof(virtio_gpu_resource_create_2d_t));
                req->header.type = VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
                req->header.ctxID = reference->ctxID;
                req->resourceID = gpuResource->resourceID;

                virtio_gpu_ctrl_hdr_t *res = NULL;
                send_command_response(req, sizeof(virtio_gpu_ctx_attach_resource_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                    kfree(res);
                    kfree(req);
                    return -1;
                }
                
                kfree(res);
                kfree(req);

                return 0;
            }

        case VIRTGPU_IOCTL_TRANSFER_FROM_HOST_3D:
            {
                if(!argp) return -1;

                virtgpu_transfer_3d_t *virtioTransfer = (virtgpu_transfer_3d_t *)argp;
                {
                    virtio_gpu_resource_flush_t *req = (virtio_gpu_resource_flush_t *)kmalloc(sizeof(virtio_gpu_resource_flush_t));
                    req->header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
                    req->rect.x = req->rect.y = 0;
                    req->rect.width = virtioTransfer->width;
                    req->rect.height = virtioTransfer->height;
                    req->resourceID = reference->resourceID;

                    virtio_gpu_ctrl_hdr_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_resource_flush_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

                    if(res->type != VIRTIO_GPU_RESP_OK_NODATA) {
                        kfree(res);
                        kfree(req);
                        return -1;
                    }

                    kfree(res);
                    kfree(req);
                }
                
                {
                    virtio_gpu_transfer_from_host_3d_t *req = (virtio_gpu_transfer_from_host_3d_t *)kmalloc(sizeof(virtio_gpu_transfer_from_host_3d_t));
                    req->header.type = VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D;
                    req->box.x = req->box.y = req->box.z = 0;
                    req->box.width = virtioTransfer->width;
                    req->box.height = virtioTransfer->height;
                    req->box.depth = virtioTransfer->depth;
                    req->layerStride = virtioTransfer->layerStride;
                    req->stride = virtioTransfer->stride;
                    req->level = virtioTransfer->level;
                    req->offset = virtioTransfer->offset;
                    req->resourceID = reference->resourceID;

                    virtio_gpu_ctrl_hdr_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_transfer_from_host_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

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

        case VIRTGPU_IOCTL_TRANSFER_TO_HOST_3D:
            {
                if(!argp) return -1;

                virtgpu_transfer_3d_t *virtioTransfer = (virtgpu_transfer_3d_t *)argp;
                {
                    virtio_gpu_transfer_from_host_3d_t *req = (virtio_gpu_transfer_from_host_3d_t *)kmalloc(sizeof(virtio_gpu_transfer_from_host_3d_t));
                    req->header.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D;
                    req->box.x = req->box.y = req->box.z = 0;
                    req->box.width = virtioTransfer->width;
                    req->box.height = virtioTransfer->height;
                    req->box.depth = virtioTransfer->depth;
                    req->layerStride = virtioTransfer->layerStride;
                    req->stride = virtioTransfer->stride;
                    req->level = virtioTransfer->level;
                    req->offset = virtioTransfer->offset;
                    req->resourceID = reference->resourceID;

                    virtio_gpu_ctrl_hdr_t *res = NULL;
                    send_command_response(req, sizeof(virtio_gpu_transfer_from_host_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));

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

        default:
            return -1;
    }
}

static int _init() {
    printf("Probing virtio-gpu\n");

    printf("\n");

    virtioDevice = pci_get_device_by_vendor(0x1AF4, 0x1050);
    if(virtioDevice) {
        printf("Yeah\n");

        spinlock_lock(&ioEvent);

        uint16_t config4_16 = pci_read_word(virtioDevice, 0x4);
        config4_16 |= 4;
        pci_write_word(virtioDevice, 0x4, config4_16);

        uint32_t config4 = pci_read_dword(virtioDevice, 0x4);
        uint8_t config34 = pci_read_byte(virtioDevice, 0x34);

        uint32_t offset = 0;
        uint32_t offset2 = 0;
        uint8_t barIndex = 0;
        uint8_t barIndex2 = 0;
        __attribute__((unused)) uint32_t notifyMultiplier = 0;

        if((config4 >> 16) & (1 << 4)) {
            uint8_t cap_off = config34;

            while(cap_off) {
                uint8_t cap_id = pci_read_byte(virtioDevice, cap_off);
                uint8_t cap_next = pci_read_byte(virtioDevice, cap_off + 1);

                //printf("cap_id: %x\n", cap_id);

                switch(cap_id) {
                    case 0x09: {
                        uint8_t type = pci_read_byte(virtioDevice, cap_off + 3);
                        //printf("Type: %x\n", type);
                        if(type == 0x1) {
                            offset = pci_read_dword(virtioDevice, cap_off + 0x8);
                            barIndex = pci_read_byte(virtioDevice, cap_off + 4);
                            printf("BarIndex: %x\n", barIndex);
                        } else if(type == 0x2) {
                            barIndex2 = pci_read_byte(virtioDevice, cap_off + 4);
                            offset2 = pci_read_dword(virtioDevice, cap_off + 0x8);
                            notifyMultiplier = pci_read_dword(virtioDevice, cap_off + 0x10);
                            printf("BarIndex2: %x\n", barIndex2);
                        } else if(type == 0x3) {
                            uint8_t isrBarIndex = pci_read_byte(virtioDevice, cap_off + 4);
                            uint32_t isrOffset = pci_read_dword(virtioDevice, cap_off + 0x8);
                            pci_bar_t *isrBar = (pci_bar_t *)kmalloc(sizeof(pci_bar_t));
                            pci_read_bar(virtioDevice, isrBarIndex, isrBar);
                            isrStatus = (uint32_t *)(isrBar->base + isrOffset + VIRT_PHYS_BASE);
                            kfree(isrBar);
                        } else if(type == 0x4) {
                            uint8_t cfgBarIndex = pci_read_byte(virtioDevice, cap_off + 4);
                            uint32_t cfgOffset = pci_read_dword(virtioDevice, cap_off + 0x8);
                            pci_bar_t *cfgBar = (pci_bar_t *)kmalloc(sizeof(pci_bar_t));
                            pci_read_bar(virtioDevice, cfgBarIndex, cfgBar);
                            gpuConfig = (virtio_gpu_config_t *)(cfgBar->base + cfgOffset + VIRT_PHYS_BASE);
                        }
                    } break;
                }
                cap_off = cap_next;
            }
        }

        pci_bar_t *virtioBar = (pci_bar_t *)kmalloc(sizeof(pci_bar_t));
        pci_read_bar(virtioDevice, barIndex, virtioBar);

        pci_bar_t *virtioBarNotify = (pci_bar_t *)kmalloc(sizeof(pci_bar_t));
        pci_read_bar(virtioDevice, barIndex2, virtioBarNotify);
        
        uint64_t base = virtioBar->base + VIRT_PHYS_BASE;

        virtioRegs = kmalloc(sizeof(virtio_registers_t));
        virtioRegs->deviceFeaturesSel = (uint32_t *)(base + offset + 0x00);
        virtioRegs->deviceFeatures = (uint32_t *)(base + offset + 0x04);
        virtioRegs->guestFeaturesSel = (uint32_t *)(base + offset + 0x08);
        virtioRegs->guestFeatures = (uint32_t *)(base + offset + 0x0C);
        virtioRegs->msixConfig = (uint16_t *)(base + offset + 0x10);
        virtioRegs->numQueues = (uint16_t *)(base + offset + 0x12);
        virtioRegs->deviceStatus = (uint8_t *)(base + offset + 0x14);
        virtioRegs->configGeneration = (uint8_t *)(base + offset + 0x15);
        
        virtioRegs->queueSelect = (uint16_t *)(base + offset + 0x16);
        virtioRegs->queueSize = (uint16_t *)(base + offset + 0x18);
        virtioRegs->queueMsixVector = (uint16_t *)(base + offset + 0x1A);
        virtioRegs->queueEnable = (uint16_t *)(base + offset + 0x1C);
        virtioRegs->queueNotify = (uint16_t *)(base + offset + 0x1E);
        virtioRegs->queueDesc = (uint64_t *)(base + offset + 0x20);
        virtioRegs->queueGuest = (uint64_t *)(base + offset + 0x28);
        virtioRegs->queueDevice = (uint64_t *)(base + offset + 0x30);
        /*virtioRegs->queueAddress = (uint32_t *)(base + offset + 0x08);
        virtioRegs->deviceStatus = (uint8_t *)(base + offset + 0x12);
        virtioRegs->isrStatus = (uint8_t *)(base + offset + 0x13);*/

        *virtioRegs->deviceStatus = 0;
        while(*virtioRegs->deviceStatus != 0);

        *virtioRegs->deviceStatus |= VIRTIO_DEVICE_ACKNOWLEDGED;
        *virtioRegs->deviceStatus = VIRTIO_DEVICE_ACKNOWLEDGED | VIRTIO_DRIVER_LOADED;

        *virtioRegs->deviceFeaturesSel = 0x01;
        uint32_t deviceFeartures2 = *virtioRegs->deviceFeatures;
        *virtioRegs->deviceFeaturesSel = 0x00;
        uint32_t deviceFeartures = *virtioRegs->deviceFeatures;
        printf("Virtio: base: %lx, offset: %lx, mmio: %x, prefetchable: %x, size: %lx\n", virtioBarNotify->base, offset2, virtioBarNotify->is_mmio, virtioBarNotify->is_prefetchable, virtioBarNotify->size);
        printf("Virtio: devFeatures: 0x%lx\n", ((uint64_t)deviceFeartures2 << 32) | deviceFeartures);

        *((volatile uint32_t *)virtioRegs->guestFeaturesSel) = 0x00;
        *((volatile uint32_t *)virtioRegs->guestFeatures) = 0x1;
        *((volatile uint32_t *)virtioRegs->guestFeaturesSel) = 0x01;
        *((volatile uint32_t *)virtioRegs->guestFeatures) = 0x1;

        *virtioRegs->deviceStatus = VIRTIO_DEVICE_ACKNOWLEDGED | VIRTIO_DRIVER_LOADED | VIRTIO_FEATURES_OK;
        
        vqs = (queue_virtqueue_t **)kcalloc(2, sizeof(queue_virtqueue_t *));
        
        init_queue(0);
        init_queue(1);

        connectDeviceToPin(virtioDevice->gsi, irq_handler, virtioDevice->gsiFlags);

        *virtioRegs->queueSelect = 0x00;
        notifyQueue0 = virtioBarNotify->base + VIRT_PHYS_BASE + offset2 + *virtioRegs->queueNotify * notifyMultiplier;
        *virtioRegs->queueSelect = 0x01;
        notifyQueue1 = virtioBarNotify->base + VIRT_PHYS_BASE + offset2 + *virtioRegs->queueNotify * notifyMultiplier;

        *virtioRegs->deviceStatus = VIRTIO_DEVICE_ACKNOWLEDGED | VIRTIO_DRIVER_LOADED | VIRTIO_DRIVER_READY;

        virtio_mode_t currentMode = {0};
        uint8_t currentModeNumber = 0;

        //Get display info
        {
            virtio_gpu_ctrl_hdr_t *header = (virtio_gpu_ctrl_hdr_t *)kmalloc(sizeof(virtio_gpu_ctrl_hdr_t));
            header->type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;

            virtio_display_info_t *info = NULL;
            send_command_response(header, sizeof(virtio_gpu_ctrl_hdr_t), (void **)&info, sizeof(virtio_display_info_t));

            //kfree(header);

            for(size_t i = 0; i < 16; i++) {
                if(info->modes[i].enabled) {
                    printf("%u: x %u, y %u, w %u, h %u, flags 0x%x\n", i, info->modes[i].rect.x, info->modes[i].rect.y, info->modes[i].rect.width, info->modes[i].rect.height, info->modes[i].flags);
                    currentMode = info->modes[i];
                    currentModeNumber = i;
                }
            }
            //kfree(info);
        }

        (void)currentMode;
        (void)currentModeNumber;

        /*{
            virtio_gpu_context_create_t *req = (virtio_gpu_context_create_t *)kmalloc(sizeof(virtio_gpu_context_create_t));
            req->header.type = VIRTIO_GPU_CMD_CTX_CREATE;
            req->header.ctxID = 1;
            req->header.flags = 1;
            req->header.fenceID = 10;
            req->nameLength = 4;
            req->debugName[0] = 'H';
            req->debugName[1] = 'o';
            req->debugName[2] = 'l';
            req->debugName[3] = 'a';

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        {
            uint32_t bytes = 4 * ((1 + 1) + (1 + 3) + (1 + 6));

            virtio_gpu_submit_3d_t *req = (virtio_gpu_submit_3d_t *)kmalloc(sizeof(virtio_gpu_submit_3d_t) + bytes);
            req->header.type = VIRTIO_GPU_CMD_SUBMIT_3D;
            req->header.ctxID = 1;

            float floatArray[6] = {
                1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
            };

            req->size = 4 * ((1 + 1) + (1 + 3) + (1 + 6));
            req->command[0] = (1 << 16) | VIRGL_CCMD_CREATE_SUB_CTX;
            req->command[1] = 1;
            req->command[2] = (6 << 16) | VIRGL_CCMD_SET_VIEWPORT_STATE;
            memcpy(&req->command[3], floatArray, 6);
            req->command[9] = (3 << 16) | VIRGL_CCMD_SET_SCISSOR_STATE;
            req->command[10] = 0;
            req->command[11] = (0xffff) | (0xffff << 16);
            req->command[12] = (0x0f0f) | (0xf00f << 16);

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_3d_t) + bytes, (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        {
            virtio_gpu_resource_create_3d_t *req = (virtio_gpu_resource_create_3d_t *)kmalloc(sizeof(virtio_gpu_resource_create_3d_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
            req->header.ctxID = 1;
            req->resourceID = 2;
            req->target = 3;
            req->format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
            req->bind = PIPE_BIND_SAMPLER_VIEW;
            req->width = 50;
            req->height = 1;
            req->depth = 1;
            req->arraySize = 1;
            req->lastLevel = 0;
            req->nrSamples = 0;
            

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }*/

        /*{
            virtio_gpu_resource_create_3d_t *req = (virtio_gpu_resource_create_3d_t *)kmalloc(sizeof(virtio_gpu_resource_create_3d_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
            req->resourceID = 2;
            req->target = 3;
            req->format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
            req->bind = PIPE_BIND_SAMPLER_VIEW;
            req->width = 50;
            req->height = 1;
            req->depth = 1;
            req->arraySize = 1;
            req->lastLevel = 0;
            req->nrSamples = 0;
            

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_3d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }*/

        /*{ // Cursor
            virtio_gpu_resource_create_2d_t *req = (virtio_gpu_resource_create_2d_t *)kmalloc(sizeof(virtio_gpu_resource_create_2d_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
            req->header.flags = 1;
            req->header.fenceID = 10;
            req->resourceID = 2;
            req->format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
            req->width = 64;
            req->height = 64;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_create_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        void *cframebuffer = NULL;
        {
            struct {
                virtio_gpu_resource_attach_backing_t req;
                virtio_gpu_mem_entry_t mem;
            } *req = kmalloc(sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t));
        
            req->req.header.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
            req->req.header.flags = 1;
            req->req.header.fenceID = 10;
            req->req.resourceID = 2;
            req->req.nrEntries = 1;

            size_t pageslength = 64 * 64 * 4;
            cframebuffer = (void *)kmalloc(pageslength);
        
            req->mem.address = ((uint64_t)cframebuffer - VIRT_PHYS_BASE);
            req->mem.length = pageslength;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_attach_backing_t) + sizeof(virtio_gpu_mem_entry_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        for(size_t i = 0; i < 64; i++) {
            for(size_t j = 0; j < 64; j++) {
                uint32_t *pixel = (uint32_t*)((uint64_t)cframebuffer + (j + i * 64) * 4);
                *pixel = 0xFFFF0000;
            }
        }

        {
            virtio_gpu_transfer_to_host_2d_t *req = (virtio_gpu_transfer_to_host_2d_t *)kmalloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
            req->header.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
            req->header.flags = 1;
            req->header.fenceID = 10;
            req->rect.x = req->rect.y = 0;
            req->rect.width = 64;
            req->rect.height = 64;
            req->offset = 0;
            req->resourceID = 2;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_transfer_to_host_2d_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        {
            virtio_gpu_update_cursor_t *req = (virtio_gpu_update_cursor_t *)kmalloc(sizeof(virtio_gpu_update_cursor_t));
            req->header.type = VIRTIO_GPU_CMD_UPDATE_CURSOR;
            req->position.scanoutId = currentModeNumber;
            req->position.x = 50;
            req->position.y = 50;
            req->hotX = 0;
            req->hotY = 0;
            req->resourceID = 2;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response_cursor(req, sizeof(virtio_gpu_update_cursor_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }

        {
            virtio_gpu_resource_flush_t *req = (virtio_gpu_resource_flush_t *)kmalloc(sizeof(virtio_gpu_resource_flush_t));
            req->header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
            req->rect.x = req->rect.y = 0;
            req->rect.width = currentMode.rect.width;
            req->rect.height = currentMode.rect.height;
            req->resourceID = 1;

            virtio_gpu_ctrl_hdr_t *res = NULL;
            send_command_response(req, sizeof(virtio_gpu_resource_flush_t), (void **)&res, sizeof(virtio_gpu_ctrl_hdr_t));
        
            printf("Type: %x\n", res->type);
            kfree(res);
        }*/

        devfs_node_t *gpu = (devfs_node_t *)kcalloc(sizeof(devfs_node_t *), 1);
        strcpy(gpu->name, "VirtIOGPU");
        gpu->functions.write = gpu_write;
        gpu->functions.read = gpu_read;
        gpu->functions.ioctl = gpu_ioctl;
        gpu->functions.open = gpu_open;
        gpu->flags |= FS_FILE;

        gpu->device = list_create();

        devfs_mount("gpu", gpu);

        return 0;
    }

    return 1;
}

static int _exit() {
    printf("Goodbye :D\n");
    return 0;
}

MODULE_DEF(virtiogpu, _init, _exit);