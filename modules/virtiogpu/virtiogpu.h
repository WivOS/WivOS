#pragma once

#include <modules/modules.h>
#include <util/util.h>
#include <cpu/pci.h>

typedef struct {
    uint32_t *deviceFeaturesSel;
    uint32_t *deviceFeatures;
    uint32_t *guestFeaturesSel;
    uint32_t *guestFeatures;
    uint16_t *msixConfig;
    uint16_t *numQueues;
    uint8_t *deviceStatus;
    uint8_t *configGeneration;

    uint16_t *queueSelect;
    uint16_t *queueSize;
    uint16_t *queueMsixVector;
    uint16_t *queueEnable;
    uint16_t *queueNotify;
    uint64_t *queueDesc;
    uint64_t *queueGuest;
    uint64_t *queueDevice;
} virtio_registers_t;

typedef struct {
    uint32_t events_read;
	uint32_t events_clear;
	uint32_t num_scanouts;
	uint32_t reserved;
} virtio_gpu_config_t;

#define VIRTIO_DEVICE_ACKNOWLEDGED 0x1
#define VIRTIO_DRIVER_LOADED 0x2
#define VIRTIO_DRIVER_READY 0x4
#define VIRTIO_FEATURES_OK 0x8
#define VIRTIO_DEVICE_ERROR 0x40
#define VIRTIO_DRIVER_FAILED 0x80

typedef struct {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} queue_descriptor_t;

typedef struct {
    uint16_t flags;
    uint16_t index;
    uint16_t rings[];
} queue_available_t;

typedef struct {
    uint32_t index;
    uint32_t length;
} queue_used_element_t;

typedef struct {
    uint16_t flags;
    uint16_t index;
    queue_used_element_t rings[];
} queue_used_t;

typedef struct {
    queue_descriptor_t *descriptors;
    queue_available_t *available;
    queue_used_t *used;
    uint16_t freeList;
    uint16_t freeCount;
    uint16_t queueSize;
    uint16_t queueMask;
    uint16_t lastUsed;
} queue_virtqueue_t;

#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_DESC_F_INDIRECT 4

#define VIRTQ_USED_F_NO_NOTIFY 1

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t fenceID;
    uint32_t ctxID;
    uint32_t padding;
} virtio_gpu_ctrl_hdr_t;

typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;
} virtio_rect_t;

typedef struct {
	uint32_t x;
	uint32_t y;
    uint32_t z;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} virtio_box_t;

typedef struct {
	virtio_rect_t rect;
    uint32_t enabled;
    uint32_t flags;
} virtio_mode_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_mode_t modes[16];
} virtio_display_info_t;

enum virtio_gpu_ctrl_type { 
        /* 2d commands */ 
        VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100, 
        VIRTIO_GPU_CMD_RESOURCE_CREATE_2D, 
        VIRTIO_GPU_CMD_RESOURCE_UNREF, 
        VIRTIO_GPU_CMD_SET_SCANOUT, 
        VIRTIO_GPU_CMD_RESOURCE_FLUSH, 
        VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D, 
        VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING, 
        VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING, 
        VIRTIO_GPU_CMD_GET_CAPSET_INFO, 
        VIRTIO_GPU_CMD_GET_CAPSET, 
        VIRTIO_GPU_CMD_GET_EDID,
        VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,

        /* 3d commands (OpenGL) */ 
        VIRTIO_GPU_CMD_CTX_CREATE = 0x0200, 
        VIRTIO_GPU_CMD_CTX_DESTROY, 
        VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE, 
        VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE, 
        VIRTIO_GPU_CMD_RESOURCE_CREATE_3D, 
        VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D, 
        VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D, 
        VIRTIO_GPU_CMD_SUBMIT_3D,
 
        /* cursor commands */ 
        VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300, 
        VIRTIO_GPU_CMD_MOVE_CURSOR, 
 
        /* success responses */ 
        VIRTIO_GPU_RESP_OK_NODATA = 0x1100, 
        VIRTIO_GPU_RESP_OK_DISPLAY_INFO, 
        VIRTIO_GPU_RESP_OK_CAPSET_INFO, 
        VIRTIO_GPU_RESP_OK_CAPSET, 
        VIRTIO_GPU_RESP_OK_EDID, 
 
        /* error responses */ 
        VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200, 
        VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY, 
        VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID, 
        VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID, 
        VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID, 
        VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER, 
};

enum virtio_gpu_formats { 
    VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM  = 1, 
    VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM  = 2, 
    VIRTIO_GPU_FORMAT_A8R8G8B8_UNORM  = 3,
    VIRTIO_GPU_FORMAT_X8R8G8B8_UNORM  = 4, 

    VIRTIO_GPU_FORMAT_R8G8B8A8_UNORM  = 67, 
    VIRTIO_GPU_FORMAT_X8B8G8R8_UNORM  = 68, 

    VIRTIO_GPU_FORMAT_A8B8G8R8_UNORM  = 121, 
    VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM  = 134, 
};

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t resourceID;
    uint32_t format;
    uint32_t width;
    uint32_t height;
} __attribute__((packed)) virtio_gpu_resource_create_2d_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_rect_t rect;
    uint32_t scanoutID;
    uint32_t resourceID;
} __attribute__((packed)) virtio_gpu_set_scanout_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t resourceID;
    uint32_t nrEntries;
} __attribute__((packed)) virtio_gpu_resource_attach_backing_t;

typedef struct {
    uint64_t address;
    uint32_t length;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_mem_entry_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_rect_t rect;
    uint64_t offset;
    uint32_t resourceID;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_transfer_to_host_2d_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_box_t box;
    uint64_t offset;
    uint32_t resourceID;
    uint32_t level;
    uint32_t stride;
    uint32_t layerStride;
} __attribute__((packed)) virtio_gpu_transfer_from_host_3d_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_rect_t rect;
    uint32_t resourceID;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_resource_flush_t;


typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t nameLength;
    uint32_t padding;
    char debugName[64];
} __attribute__((packed)) virtio_gpu_context_create_t;

#define PIPE_BIND_DEPTH_STENCIL        (1 << 0) /* create_surface */
#define PIPE_BIND_RENDER_TARGET        (1 << 1) /* create_surface */
#define PIPE_BIND_BLENDABLE            (1 << 2) /* create_surface */
#define PIPE_BIND_SAMPLER_VIEW         (1 << 3) /* create_sampler_view */
#define PIPE_BIND_VERTEX_BUFFER        (1 << 4) /* set_vertex_buffers */
#define PIPE_BIND_INDEX_BUFFER         (1 << 5) /* draw_elements */
#define PIPE_BIND_CONSTANT_BUFFER      (1 << 6) /* set_constant_buffer */
#define PIPE_BIND_DISPLAY_TARGET       (1 << 8) /* flush_front_buffer */
#define PIPE_BIND_TRANSFER_WRITE       (1 << 9) /* transfer_map */
#define PIPE_BIND_TRANSFER_READ        (1 << 10) /* transfer_map */
#define PIPE_BIND_STREAM_OUTPUT        (1 << 11) /* set_stream_output_buffers */
#define PIPE_BIND_CURSOR               (1 << 16) /* mouse cursor */
#define PIPE_BIND_CUSTOM               (1 << 17) /* state-tracker/winsys usages */
#define PIPE_BIND_GLOBAL               (1 << 18) /* set_global_binding */
#define PIPE_BIND_SHADER_RESOURCE      (1 << 19) /* set_shader_resources */
#define PIPE_BIND_COMPUTE_RESOURCE     (1 << 20) /* set_compute_resources */
#define PIPE_BIND_COMMAND_ARGS_BUFFER  (1 << 21) /* pipe_draw_info.indirect */
#define PIPE_BIND_QUERY_BUFFER         (1 << 22) /* get_query_result_resource */

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t resourceID;
    uint32_t target;
    uint32_t format;
    uint32_t bind;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t arraySize;
    uint32_t lastLevel;
    uint32_t nrSamples;
    uint32_t flags;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_resource_create_3d_t;

typedef struct {
    uint32_t scanoutId;
    uint32_t x;
    uint32_t y;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_cursor_position_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    virtio_gpu_cursor_position_t position;
    uint32_t resourceID;
    uint32_t hotX;
    uint32_t hotY;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_update_cursor_t;

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t resourceID;
    uint32_t padding;
} __attribute__((packed)) virtio_gpu_ctx_attach_resource_t;

enum {
   VIRGL_CCMD_NOP = 0,
   VIRGL_CCMD_CREATE_OBJECT = 1,
   VIRGL_CCMD_BIND_OBJECT,
   VIRGL_CCMD_DESTROY_OBJECT,
   VIRGL_CCMD_SET_VIEWPORT_STATE,
   VIRGL_CCMD_SET_FRAMEBUFFER_STATE,
   VIRGL_CCMD_SET_VERTEX_BUFFERS,
   VIRGL_CCMD_CLEAR,
   VIRGL_CCMD_DRAW_VBO,
   VIRGL_CCMD_RESOURCE_INLINE_WRITE,
   VIRGL_CCMD_SET_SAMPLER_VIEWS,
   VIRGL_CCMD_SET_INDEX_BUFFER,
   VIRGL_CCMD_SET_CONSTANT_BUFFER,
   VIRGL_CCMD_SET_STENCIL_REF,
   VIRGL_CCMD_SET_BLEND_COLOR,
   VIRGL_CCMD_SET_SCISSOR_STATE,
   VIRGL_CCMD_BLIT,
   VIRGL_CCMD_RESOURCE_COPY_REGION,
   VIRGL_CCMD_BIND_SAMPLER_STATES,
   VIRGL_CCMD_BEGIN_QUERY,
   VIRGL_CCMD_END_QUERY,
   VIRGL_CCMD_GET_QUERY_RESULT,
   VIRGL_CCMD_SET_POLYGON_STIPPLE,
   VIRGL_CCMD_SET_CLIP_STATE,
   VIRGL_CCMD_SET_SAMPLE_MASK,
   VIRGL_CCMD_SET_STREAMOUT_TARGETS,
   VIRGL_CCMD_SET_RENDER_CONDITION,
   VIRGL_CCMD_SET_UNIFORM_BUFFER,

   VIRGL_CCMD_SET_SUB_CTX,
   VIRGL_CCMD_CREATE_SUB_CTX,
   VIRGL_CCMD_DESTROY_SUB_CTX,
   VIRGL_CCMD_BIND_SHADER,
   VIRGL_CCMD_SET_TESS_STATE,
   VIRGL_CCMD_SET_MIN_SAMPLES,
   VIRGL_CCMD_SET_SHADER_BUFFERS,
   VIRGL_CCMD_SET_SHADER_IMAGES,
   VIRGL_CCMD_MEMORY_BARRIER,
   VIRGL_CCMD_LAUNCH_GRID,
   VIRGL_CCMD_SET_FRAMEBUFFER_STATE_NO_ATTACH,
   VIRGL_CCMD_TEXTURE_BARRIER,
   VIRGL_CCMD_SET_ATOMIC_BUFFERS,
   VIRGL_CCMD_SET_DEBUG_FLAGS,
   VIRGL_CCMD_GET_QUERY_RESULT_QBO,
   VIRGL_CCMD_TRANSFER3D,
   VIRGL_CCMD_END_TRANSFERS,
   VIRGL_CCMD_COPY_TRANSFER3D,
   VIRGL_CCMD_SET_TWEAKS,
   VIRGL_CCMD_CLEAR_TEXTURE,
   VIRGL_CCMD_PIPE_RESOURCE_CREATE,
   VIRGL_MAX_COMMANDS
};

enum virgl_object_type {
    VIRGL_OBJECT_NULL,
    VIRGL_OBJECT_BLEND,
    VIRGL_OBJECT_RASTERIZER,
    VIRGL_OBJECT_DSA,
    VIRGL_OBJECT_SHADER,
    VIRGL_OBJECT_VERTEX_ELEMENTS,
    VIRGL_OBJECT_SAMPLER_VIEW,
    VIRGL_OBJECT_SAMPLER_STATE,
    VIRGL_OBJECT_SURFACE,
    VIRGL_OBJECT_QUERY,
    VIRGL_OBJECT_STREAMOUT_TARGET,
    VIRGL_MAX_OBJECTS,
};

enum pipe_texture_target {
    PIPE_BUFFER = 0,
    PIPE_TEXTURE_1D = 1,
    PIPE_TEXTURE_2D,
    PIPE_TEXTURE_3D,
    PIPE_TEXTURE_CUBE,
    PIPE_TEXTURE_RECT,
    PIPE_TEXTURE_1D_ARRAY,
    PIPE_TEXTURE_2D_ARRAY,
    PIPE_TEXTURE_CUBE_ARRAY,
};

#define VREND_RES_BIND_DEPTH_STENCIL (1 << 0)
#define VREND_RES_BIND_RENDER_TARGET (1 << 1)
#define VREND_RES_BIND_SAMPLER_VIEW  (1 << 3)
#define VREND_RES_BIND_CUSTOM (1 << 17)

#define PIPE_CLEAR_DEPTH        (1 << 0)
#define PIPE_CLEAR_STENCIL      (1 << 1)
#define PIPE_CLEAR_COLOR0       (1 << 2)
#define PIPE_CLEAR_COLOR1       (1 << 3)
#define PIPE_CLEAR_COLOR2       (1 << 4)
#define PIPE_CLEAR_COLOR3       (1 << 5)
#define PIPE_CLEAR_COLOR4       (1 << 6)
#define PIPE_CLEAR_COLOR5       (1 << 7)
#define PIPE_CLEAR_COLOR6       (1 << 8)
#define PIPE_CLEAR_COLOR7       (1 << 9)
#define PIPE_CLEAR_COLOR        (PIPE_CLEAR_COLOR0 | PIPE_CLEAR_COLOR1 | \
                                 PIPE_CLEAR_COLOR2 | PIPE_CLEAR_COLOR3 | \
                                 PIPE_CLEAR_COLOR4 | PIPE_CLEAR_COLOR5 | \
                                 PIPE_CLEAR_COLOR6 | PIPE_CLEAR_COLOR7)

typedef struct {
    virtio_gpu_ctrl_hdr_t header;
    uint32_t size;
    uint32_t padding;
    uint32_t command[];
} __attribute__((packed)) virtio_gpu_submit_3d_t;

#define VIRTGPU_IOCTL_CREATE_RESOURCE_2D 0x1
#define VIRTGPU_IOCTL_GET_DISPLAY_INFO 0x2
#define VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING 0x3
#define VIRTGPU_IOCTL_SET_SCANOUT 0x4
#define VIRTGPU_IOCTL_TRANSFER_AND_FLUSH 0x5
#define VIRTGPU_IOCTL_SET_RESOURCE_ID 0x6
#define VIRTGPU_IOCTL_SET_CONTEXT_ID 0x7
#define VIRTGPU_IOCTL_CREATE_GPU_CONTEXT 0x8
#define VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE 0x9
#define VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE 0xA
#define VIRTGPU_IOCTL_CREATE_RESOURCE_3D 0xB
#define VIRTGPU_IOCTL_ATTACH_RESOURCE_3D 0xC
#define VIRTGPU_IOCTL_TRANSFER_FROM_HOST_3D 0xD

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t format;
} virtgpu_create_resource_2d_t;

typedef struct {
    uint32_t target;
    uint32_t bind;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t depth;
    uint32_t arraySize;
    uint32_t lastLevel;
    uint32_t nrSamples;
    uint32_t flags;
    uint32_t padding;
} virtgpu_create_resource_3d_t;

typedef struct {
    uint64_t address;
    uint32_t length;
} virtgpu_attach_backing_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t scanoutID;
} virtgpu_set_scanout_t;

typedef struct {
    uint32_t width;
    uint32_t height;
} virtgpu_transfer_and_flush_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t stride;
    uint32_t level;
    uint32_t layerStride;
    uint32_t offset;
} virtgpu_transfer_3d_t;

typedef struct {
    uint8_t nameLength;
    char *name;
} virtgpu_create_gpu_context_t;

typedef struct {
    uint8_t command;
    uint8_t option;
    uint16_t length;
    uint32_t *parameters;
} virtgpu_3d_command_t;

typedef struct {
    uint32_t resourceID;
} virtgpu_attach_resource_3d_t;
