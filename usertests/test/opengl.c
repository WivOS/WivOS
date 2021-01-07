int printf(const char* format, ...);

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../../modules/virtiogpu/virtiogpu.h"
#include <stddef.h>

#include "opengl.h"
#include "glsl.h"

static volatile uint32_t currOpenglResource = 1;

union fi {
   float f;
   int32_t i;
   uint32_t ui;
};

static inline unsigned
fui( float f )
{
   union fi fi;
   fi.f = f;
   return fi.ui;
}

static void set_sub_ctx(size_t gpuNode, uint32_t contextResourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_SUB_CTX;
    command.option = 0;
    command.length = 1;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = contextResourceID; // ctx id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static uint32_t create_opengl_context(size_t gpuNode) {
    virtgpu_3d_command_t command = {0};

    uint32_t contextResourceID = currOpenglResource++;

    command.command = VIRGL_CCMD_CREATE_SUB_CTX;
    command.option = 0;
    command.length = 1;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = contextResourceID; // ctx id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);

    return contextResourceID;
}


//TODO: Check every type
static uint32_t create_opengl_object(size_t gpuNode, uint32_t objectType, uint32_t *parameters, size_t count, uint8_t bindIt) {
    virtgpu_3d_command_t command = {0};

    uint32_t objectResourceID = currOpenglResource++;

    command.command = VIRGL_CCMD_CREATE_OBJECT;
    command.option = objectType;
    command.length = (1 + count);
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * (1 + count));
    command.parameters[0] = objectResourceID; // Surface id
    for(size_t i = 0; i < count; i++) {
        command.parameters[1 + i] = parameters[i];
    }

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    if(!bindIt) {
        free(command.parameters);
        return objectResourceID;
    }

    command.command = VIRGL_CCMD_BIND_OBJECT;
    command.option = objectType;
    command.length = 1;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = objectResourceID; // Ve id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);

    return objectResourceID;
}

static void bind_opengl_object(size_t gpuNode, uint32_t objectType, uint32_t resourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_BIND_OBJECT;
    command.option = objectType;
    command.length = 1;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = resourceID; // Ve id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void set_framebuffer_state(size_t gpuNode, uint32_t depthBuffer, uint32_t *surfaces, size_t count) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_FRAMEBUFFER_STATE;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = count;
    command.parameters[1] = depthBuffer;
    for(size_t i = 0; i < count; i++) {
        command.parameters[2 + i] = surfaces[i];
    }

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void clearResource(size_t gpuNode, float red, float green, float blue, float alpha, double depth, uint32_t stencil, uint32_t clearFlags) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_CLEAR;
    command.option = 0;
    command.length = 8;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 8);
    command.parameters[0] = clearFlags; //(1 << 2);
    *((float *)&command.parameters[1]) = red;
    *((float *)&command.parameters[2]) = green;
    *((float *)&command.parameters[3]) = blue;
    *((float *)&command.parameters[4]) = alpha;
    *((double *)&command.parameters[5]) = depth;
    command.parameters[7] = stencil;

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static uint32_t create_simple_buffer(size_t gpuNode, uint32_t width, uint32_t bind, uint32_t **buffer) {
    virtgpu_create_resource_3d_t createResource3D = {0};
    createResource3D.target = PIPE_BUFFER;
    createResource3D.format = 64;
    createResource3D.width = width;
    createResource3D.height = 1;
    createResource3D.depth = 1;
    createResource3D.arraySize = 1;
    createResource3D.lastLevel = 0;
    createResource3D.nrSamples = 0;
    createResource3D.bind = bind;
    createResource3D.flags = 0;

    uint32_t resource3D = ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_3D, &createResource3D);
    
    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = resource3D;
    ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    //Later we will store this in an array, for now omit it
    uint32_t vboFbSize = createResource3D.width * createResource3D.height;
    uint32_t *vboFb = (uint32_t *)malloc(vboFbSize);
    if(buffer)
        *buffer = vboFb;

    virtgpu_attach_backing_t attachBackingData = {0};
    attachBackingData.address = (uint64_t)vboFb;
    attachBackingData.length = vboFbSize;

    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resource3D);
    ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);

    return resource3D;
}

static uint32_t create_resource3D(size_t gpuNode, uint32_t width, uint32_t height, uint32_t bind, uint32_t target, uint32_t format) {
    virtgpu_create_resource_3d_t createResource3D = {0};
    createResource3D.target = target;
    createResource3D.format = format;
    createResource3D.width = width;
    createResource3D.height = height;
    createResource3D.depth = 1;
    createResource3D.arraySize = 1;
    createResource3D.lastLevel = 0;
    createResource3D.nrSamples = 0;
    createResource3D.bind = bind;
    createResource3D.flags = 0;

    uint32_t resource3D = ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_3D, &createResource3D);
    
    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = resource3D;
    ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    //Later we will store this in an array, for now omit it
    uint32_t vboFbSize = createResource3D.width * createResource3D.height * 4;
    uint32_t *vboFb = (uint32_t *)malloc(vboFbSize);

    virtgpu_attach_backing_t attachBackingData = {0};
    attachBackingData.address = (uint64_t)vboFb;
    attachBackingData.length = vboFbSize;

    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resource3D);
    ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);

    return resource3D;
}

static uint32_t create_texture2D(size_t gpuNode, uint32_t width, uint32_t height, uint32_t format, uint32_t **buffer) {
    virtgpu_create_resource_3d_t createResource3D = {0};
    createResource3D.target = PIPE_TEXTURE_2D;
    createResource3D.format = format;
    createResource3D.width = width;
    createResource3D.height = height;
    createResource3D.depth = 1;
    createResource3D.arraySize = 1;
    createResource3D.lastLevel = 0;
    createResource3D.nrSamples = 0;
    createResource3D.bind = PIPE_BIND_SAMPLER_VIEW;
    createResource3D.flags = 0;

    uint32_t resource3D = ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_3D, &createResource3D);
    
    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = resource3D;
    ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    //Later we will store this in an array, for now omit it
    uint32_t vboFbSize = createResource3D.width * createResource3D.height;
    uint32_t *vboFb = (uint32_t *)malloc(vboFbSize);
    if(buffer)
        *buffer = vboFb;

    virtgpu_attach_backing_t attachBackingData = {0};
    attachBackingData.address = (uint64_t)vboFb;
    attachBackingData.length = vboFbSize;

    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resource3D);
    ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);

    return resource3D;
}

static void send_inline_write(size_t gpuNode, uint32_t resourceID, uint32_t level, uint32_t usage, virtio_box_t box, const void *data, uint32_t stride, uint32_t layerStride, uint32_t format) {
    virtgpu_3d_command_t command = {0};

    uint32_t length;
    uint32_t size;
    uint32_t layerSize;
    uint32_t strideInternal = stride;
    uint32_t layerStrideInternal = layerStride;

    if(!stride) strideInternal = box.width * ((format == 1) ? 4 : 1); // Asume format of 1 byte
    layerSize = box.height * strideInternal;
    if(layerStride && layerStride < layerSize) return;
    if(!layerStride) layerStrideInternal = layerSize;
    size = layerStrideInternal * box.depth;

    //TODO: what happens when not enough space

    command.command = VIRGL_CCMD_RESOURCE_INLINE_WRITE;
    command.option = 0;
    command.length = ((size + 3) / 4) + 11;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * (((size + 3) / 4) + 11));
    command.parameters[0] = resourceID; // res id
    command.parameters[1] = level; // level
    command.parameters[2] = usage; // Usage
    command.parameters[3] = stride; // Stride
    command.parameters[4] = layerStride; // Layer Stride
    command.parameters[5] = box.x; // x
    command.parameters[6] = box.y; // y
    command.parameters[7] = box.z; // z
    command.parameters[8] = box.width; // width
    command.parameters[9] = box.height; // height
    command.parameters[10] = box.depth; // depth
    memcpy((void *)&command.parameters[11], (void *)data, size);
    /*for(size_t i = 0; i < size; i++) {
        command.parameters[11 + i] = ((uint32_t *)(data))[i];
    }*/

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void set_vertex_buffers(size_t gpuNode, uint32_t stride, uint32_t bufferOffset, uint32_t resourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_VERTEX_BUFFERS;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = stride; // Stride
    command.parameters[1] = bufferOffset; // Buffer Offset
    command.parameters[2] = resourceID; // Ve id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void set_index_buffers(size_t gpuNode, uint32_t sizeofIndex, uint32_t offset, uint32_t resourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_INDEX_BUFFER;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = resourceID;
    command.parameters[1] = sizeofIndex;
    command.parameters[2] = offset;

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void set_streamout_targets(size_t gpuNode, uint32_t bitmask, uint32_t streamoutID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_STREAMOUT_TARGETS;
    command.option = 0;
    command.length = 2;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 2);
    command.parameters[0] = bitmask; // Bitmask
    command.parameters[1] = streamoutID; // SO id

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static uint32_t create_shader(size_t gpuNode, const char *tgsiAssembly, uint32_t shaderType, uint32_t bindIt) {
    uint32_t shader_len = strlen(tgsiAssembly) + 1;

    uint32_t *parameters = (uint32_t *)malloc(sizeof(uint32_t) * (4 + ((shader_len + 3) / 4)));
    parameters[0] = shaderType; // Type, 0 -> vertex, 1 -> fragment
    parameters[1] = (((shader_len) & 0x7fffffff) << 0); // First instruction ???
    parameters[2] = 300; // Number of tokens
    parameters[3] = 0; // Stream so count
    memcpy((void *)&parameters[4], tgsiAssembly, shader_len);

    uint32_t shaderID = create_opengl_object(gpuNode, VIRGL_OBJECT_SHADER, parameters, (4 + ((shader_len + 3) / 4)), 0);

    free(parameters);


    if(!bindIt) return shaderID;

    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_BIND_SHADER;
    command.option = 0;
    command.length = 2;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 2);
    command.parameters[0] = shaderID; // Ve id
    command.parameters[1] = shaderType; // Type, 0 -> vertex, 1 -> fragment

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);

    return shaderID;
}

static void bind_shader(size_t gpuNode, uint32_t shaderID, uint32_t shaderType) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_BIND_SHADER;
    command.option = 0;
    command.length = 2;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 2);
    command.parameters[0] = shaderID; // Ve id
    command.parameters[1] = shaderType; // Type, 0 -> vertex, 1 -> fragment

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

static void set_viewport_state(size_t gpuNode, uint32_t offset, float stateA, float stateB, float stateC, float translateA, float translateB, float translateC) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_VIEWPORT_STATE;
    command.option = 0;
    command.length = 7;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 7);
    command.parameters[0] = offset; // Offset
    memcpy((void *)&command.parameters[1], &stateA, sizeof(float));
    memcpy((void *)&command.parameters[2], &stateB, sizeof(float));
    memcpy((void *)&command.parameters[3], &stateC, sizeof(float));
    memcpy((void *)&command.parameters[4], &translateA, sizeof(float));
    memcpy((void *)&command.parameters[5], &translateB, sizeof(float));
    memcpy((void *)&command.parameters[6], &translateC, sizeof(float));

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    free(command.parameters);
}

//TODO: Create the structure
static void draw_vbo(size_t gpuNode, uint32_t start, uint32_t count, uint32_t mode, uint8_t indexed, uint32_t minIndex, uint32_t maxIndex) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_DRAW_VBO;
    command.option = 0;
    command.length = 12;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 12);
    command.parameters[0] = start; // Start
    command.parameters[1] = count; // Count
    command.parameters[2] = mode; // Mode PIPE_PRIM_TRIANGLES
    command.parameters[3] = indexed; // Indexed
    command.parameters[4] = 0; // Instace Count
    command.parameters[5] = 0; // Index Bias
    command.parameters[6] = 0; // Start Instance
    command.parameters[7] = 0; // Primitive Restart
    command.parameters[8] = 0; // Restart Index
    command.parameters[9] = minIndex; // Min Index
    command.parameters[10] = maxIndex; // Max Index
    command.parameters[11] = 0; // Cso

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
}

static void set_uniform_buffer(size_t gpuNode, uint32_t shaderType, uint32_t sizeofConstants, uint32_t resourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_UNIFORM_BUFFER;
    command.option = 0;
    command.length = 5;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 5);
    command.parameters[0] = shaderType;
    command.parameters[1] = 1; // Index
    command.parameters[2] = 0; // Offset
    command.parameters[3] = sizeofConstants; // Length
    command.parameters[4] = resourceID;

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
}

static size_t gpuNode = 0;
static size_t framebufferIdentidier;

static uint32_t contextID = 0;
static uint32_t zSurfaceID = 0;
static uint32_t cSurfaceID = 0;
static uint32_t windowWidth = 0;
static uint32_t windowHeight = 0;

static GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static GLfloat depthClearValue = 1.0f;
static GLint stencilClearValue = 0;


//------------------------
static gl_buffer_object_t *bufferObjects = NULL;
static size_t currentBufferObject = 1;
static gl_vertex_array_object_t *vertexArrays = NULL;
static size_t currentVertexArray = 1;
static gl_shader_object_t *shaderObjects = NULL;
static size_t currentShaderObject = 1;
static gl_program_object_t *programObjects = NULL;
static size_t currentProgramObject = 1;

//TODO create hash map

static GLuint currentVertexArrayObject = -1;

static uint32_t alloc_opengl_buffer() {
    bufferObjects = realloc(bufferObjects, sizeof(gl_buffer_object_t) * (currentBufferObject + 1));
    currentBufferObject++;

    return (currentBufferObject - 1);
}

static gl_buffer_object_t *get_buffer_from_index(uint32_t index) {
    if(index >= currentBufferObject) return NULL;

    return &bufferObjects[index];
}

static uint32_t alloc_opengl_vertex_array() { // TODO, realloc current variables
    vertexArrays = realloc(vertexArrays, sizeof(gl_vertex_array_object_t) * (currentVertexArray + 1));
    currentVertexArray++;

    return (currentVertexArray - 1);
}

static gl_vertex_array_object_t *get_vertex_array_from_index(uint32_t index) {
    if(index >= currentVertexArray) return NULL;

    return &vertexArrays[index];
}

static uint32_t alloc_opengl_shader() {
    shaderObjects = realloc(shaderObjects, sizeof(gl_vertex_array_object_t) * (currentShaderObject + 1));
    currentShaderObject++;

    return (currentShaderObject - 1);
}

static gl_shader_object_t *get_shader_from_index(uint32_t index) {
    if(index >= currentShaderObject) return NULL;

    return &shaderObjects[index];
}

static uint32_t alloc_opengl_program() {
    programObjects = realloc(programObjects, sizeof(gl_program_object_t) * (currentProgramObject + 1));
    currentProgramObject++;

    return (currentProgramObject - 1);
}

static gl_program_object_t *get_program_from_index(uint32_t index) {
    if(index >= currentProgramObject) return NULL;

    return &programObjects[index];
}
//------------------------

void init_opengl(size_t node, size_t framebufferID, uint32_t width, uint32_t height) {
    gpuNode = node;
    framebufferIdentidier = framebufferID;
    windowWidth = width;
    windowHeight = height;

    uint32_t *parameters = NULL;
    virtgpu_3d_command_t command = {0};

    contextID = create_opengl_context(gpuNode);
    set_sub_ctx(gpuNode, contextID);
    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = framebufferID;
    ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    uint32_t zsurface3D = create_resource3D(gpuNode, width, height, PIPE_BIND_DEPTH_STENCIL, PIPE_TEXTURE_2D, 17);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 10);
    parameters[0] = 0;
    parameters[1] = 0; // format
    parameters[2] = (0xF << 27);
    parameters[3] = 0;
    parameters[4] = 0;
    parameters[5] = 0;
    parameters[6] = 0;
    parameters[7] = 0;
    parameters[8] = 0;
    parameters[9] = 0;
    uint32_t blendID = create_opengl_object(gpuNode, VIRGL_OBJECT_BLEND, parameters, 10, 1);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    //TODO: do not hack

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 4);
    parameters[0] = 0b00111;
    parameters[1] = 0; // Bitfield2
    parameters[2] = 0; // Bitfield3
    parameters[3] = fui(0.0f); // Alpha state
    uint32_t dsaID = create_opengl_object(gpuNode, VIRGL_OBJECT_DSA, parameters, 4, 1);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 9);
    parameters[0] = (1 << 30) | (1 << 29) | (0 << 1) | (0 << 8); // Bitfield1
    *((float *)&parameters[1]) = 1.0f; // Point size
    parameters[2] = 0; // Sprit coord enabled
    parameters[3] = 0xffff; // Bitfield2
    *((float *)&parameters[4]) = 0.0f; // Line width
    *((float *)&parameters[5]) = 0.0f; // Offset units
    *((float *)&parameters[6]) = 0.0f; // Offset scale
    *((float *)&parameters[7]) = 0.0f; // Offset clamp
    *((float *)&parameters[8]) = 0.0f; // Alpha state
    uint32_t rasterizerID = create_opengl_object(gpuNode, VIRGL_OBJECT_RASTERIZER, parameters, 8, 1);

    float znear = -1.0f, zfar = 1.0f;
    float half_w = 1024.0f / 2.0f;
	float half_h = 768.0f / 2.0f;
	float half_d = (zfar - znear) / 2.0f;

    set_viewport_state(gpuNode, 0, half_w, half_h, half_d, half_w, half_h, half_d + znear);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 4);
    parameters[0] = zsurface3D; // Resource ID
    parameters[1] = 17; // Format
    parameters[2] = 0; // Format specific 1
    parameters[3] = 0; // Format specific 2
    zSurfaceID = create_opengl_object(gpuNode, VIRGL_OBJECT_SURFACE, parameters, 4, 0);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 4);
    parameters[0] = framebufferID; // Resource ID
    parameters[1] = 1; // Format
    parameters[2] = 0; // Format specific 1
    parameters[3] = 0; // Format specific 2
    cSurfaceID = create_opengl_object(gpuNode, VIRGL_OBJECT_SURFACE, parameters, 4, 0);

    set_framebuffer_state(gpuNode, zSurfaceID, &cSurfaceID, 1);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);
}

GL_APICALL void GL_APIENTRY glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    clearColor[0] = red;
    clearColor[1] = green;
    clearColor[2] = blue;
    clearColor[3] = alpha;
}

GL_APICALL void GL_APIENTRY glClearDepthf (GLfloat d) {
    depthClearValue = d;
}

GL_APICALL void GL_APIENTRY glClearStencil (GLint s) {
    stencilClearValue = s;
}

GL_APICALL void GL_APIENTRY glClear (GLbitfield mask) {
    uint32_t cMask = 0;

    if(mask & GL_COLOR_BUFFER_BIT)
        cMask |= PIPE_CLEAR_COLOR;

    if(mask & GL_DEPTH_BUFFER_BIT)
        cMask |= PIPE_CLEAR_DEPTH;

    if(mask & GL_STENCIL_BUFFER_BIT)
        cMask |= PIPE_CLEAR_STENCIL;

    clearResource(gpuNode, clearColor[0], clearColor[1], clearColor[2], clearColor[3], depthClearValue, stencilClearValue, cMask);
}

GL_APICALL void GL_APIENTRY glGenVertexArrays (GLsizei n, GLuint *arrays) {
    if(!arrays) return;

    for(size_t i = 0; i < n; i++) {
        arrays[i] = alloc_opengl_vertex_array();
        gl_vertex_array_object_t *vao = get_vertex_array_from_index(arrays[i]);
        vao->isDirt = GL_TRUE;
        vao->enabledVertexAttribArrays = 0;
        vao->bufferObject = 0;
        vao->arrayBufferObject = 0;
        vao->elementArrayBufferObject = 0;
        memset(vao->attribs, 0, sizeof(gl_attrib_object_t) * 32);
        vao->attribsDirt = GL_TRUE;
        vao->attribsResource = 0;
    }
}

GL_APICALL void GL_APIENTRY glBindVertexArray (GLuint array) {
    gl_vertex_array_object_t *arrayObj = get_vertex_array_from_index(array);
    if(!arrayObj) return;

    arrayObj->isDirt = GL_TRUE;

    currentVertexArrayObject = array;
}

GL_APICALL void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
    if(currentVertexArrayObject == -1) return;
    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;

    if(index >= 32) return;

    vao->enabledVertexAttribArrays |= (1 << index);

    vao->isDirt = GL_TRUE;
    vao->attribsDirt = GL_TRUE;
}

GL_APICALL void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
    if(currentVertexArrayObject == -1) return;

    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;

    if(index >= 32) return;

    uint32_t format = 31; // Force this for now

    if(size == 4 && type == GL_FLOAT)
        format = 31;
    
    vao->attribs[index].position = (GLuint)((uint64_t)pointer);
    vao->attribs[index].size = size;
    vao->attribs[index].type = type;
    vao->attribs[index].format = format;
    vao->isDirt = GL_TRUE;
    vao->attribsDirt = GL_TRUE;
}

GL_APICALL void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {
    if(currentVertexArrayObject == -1 || currentVertexArrayObject == -1) return;

    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;
    gl_buffer_object_t *vbo = get_buffer_from_index(vao->arrayBufferObject);
    if(!vbo) return;

    if(vao->isDirt == GL_TRUE) {
        uint32_t *parameters = NULL;

        uint32_t enabled_vertex_attribs_count = 0;
        for(size_t i = 0; i < 32; i++) {
            if(vao->enabledVertexAttribArrays & (1 << i)) enabled_vertex_attribs_count++;
        }

        if(!enabled_vertex_attribs_count) return;

        parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * (4 * enabled_vertex_attribs_count));

        size_t currentIndex = 0;
        for(currentIndex = 0; (currentIndex < 32 && enabled_vertex_attribs_count > 0); currentIndex++) {
            if(vao->enabledVertexAttribArrays & (1 << currentIndex)) {
                enabled_vertex_attribs_count--;
            }
            parameters[currentIndex * 4 + 0] = vao->attribs[currentIndex].position; // src offset
            parameters[currentIndex * 4 + 1] = 0; // instance divisor
            parameters[currentIndex * 4 + 2] = 0; // vertex buffer index
            parameters[currentIndex * 4 + 3] = vao->attribs[currentIndex].format; // src format
        }

        uint32_t vertexElementsID = create_opengl_object(gpuNode, VIRGL_OBJECT_VERTEX_ELEMENTS, parameters, (4 * (currentIndex + 1)), 1);

        //TODO: Check when other VAO is binded
        uint32_t sizeVertex = 0;

        for(size_t i = 0; i < 32; i++) {
            if(vao->enabledVertexAttribArrays & (1 << i)) {
                //For now 4 GL_FLOAT
                sizeVertex += 4 * sizeof(float);
            }
        }

        set_vertex_buffers(gpuNode, sizeVertex, 0, vbo->bufferID);

        ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        vao->isDirt = GL_FALSE;
    }
    
    draw_vbo(gpuNode, first, count, mode, 0, 0, 0);
}

GL_APICALL void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {
    if(currentVertexArrayObject == -1) return;

    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;

    gl_buffer_object_t *vbo = get_buffer_from_index(vao->arrayBufferObject);
    gl_buffer_object_t *ebo = get_buffer_from_index(vao->elementArrayBufferObject);
    if(!vao || !ebo) return;

    if(vao->isDirt == GL_TRUE) {
        uint32_t *parameters = NULL;

        uint32_t enabled_vertex_attribs_count = 0;
        for(size_t i = 0; i < 32; i++) {
            if(vao->enabledVertexAttribArrays & (1 << i)) enabled_vertex_attribs_count++;
        }

        if(!enabled_vertex_attribs_count) return;

        parameters = (uint32_t *)calloc(1, sizeof(uint32_t) * (4 * enabled_vertex_attribs_count));

        if(vao->attribsDirt == GL_TRUE || vao->attribsResource == 0) {
            size_t currentIndex = 0;
            for(currentIndex = 0; (currentIndex < 32 && enabled_vertex_attribs_count > 0); currentIndex++) {
                if(vao->enabledVertexAttribArrays & (1 << currentIndex)) {
                    enabled_vertex_attribs_count--;
                }
                parameters[currentIndex * 4 + 0] = vao->attribs[currentIndex].position; // src offset
                parameters[currentIndex * 4 + 1] = 0; // instance divisor
                parameters[currentIndex * 4 + 2] = 0; // vertex buffer index
                parameters[currentIndex * 4 + 3] = vao->attribs[currentIndex].format; // src format
            }

            vao->attribsResource = create_opengl_object(gpuNode, VIRGL_OBJECT_VERTEX_ELEMENTS, parameters, (4 * (currentIndex)), 1);

            vao->attribsDirt = GL_FALSE;
        } else {
            bind_opengl_object(gpuNode, VIRGL_OBJECT_VERTEX_ELEMENTS, vao->attribsResource);
        }

        //TODO: Check when other VAO is binded
        uint32_t sizeVertex = 0;

        for(size_t i = 0; i < 32; i++) {
            if(vao->enabledVertexAttribArrays & (1 << i)) {
                //For now 4 GL_FLOAT
                sizeVertex += 4 * sizeof(float);
            }
        }

        uint32_t sizeOfIndex = 0;
        if(type == GL_UNSIGNED_INT)
            sizeOfIndex = sizeof(uint32_t);

        set_vertex_buffers(gpuNode, sizeVertex, 0, vbo->bufferID);

        set_index_buffers(gpuNode, sizeOfIndex, 0, ebo->bufferID);

        ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        vao->isDirt = GL_FALSE;
    }

    draw_vbo(gpuNode, 0, count, mode, 1, 0, (unsigned)-1);
}

GL_APICALL void GL_APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {
    if(!buffers) return;

    for(size_t i = 0; i < n; i++) {
        uint32_t id = buffers[i] = alloc_opengl_buffer();
        get_buffer_from_index(id)->bufferID = 0;
        get_buffer_from_index(id)->bufferObject = id;
    }
}

GL_APICALL void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
    gl_buffer_object_t *bufferObj = get_buffer_from_index(buffer);
    if(!bufferObj) return;

    bufferObj->bind = target;

    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;

    if(target == GL_ARRAY_BUFFER) {
        vao->arrayBufferObject = buffer;
    } else if(target == GL_ELEMENT_ARRAY_BUFFER) {
        vao->elementArrayBufferObject = buffer;
    }
}

//TODO: Finish this
GL_APICALL void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) { // Unused usage for now
    if(currentVertexArrayObject == -1) return;
    
    gl_buffer_object_t *currObj = NULL;

    gl_vertex_array_object_t *vao = get_vertex_array_from_index(currentVertexArrayObject);
    if(!vao) return;

    if(target == GL_ARRAY_BUFFER)
        currObj = get_buffer_from_index(vao->arrayBufferObject);
    else if(target == GL_ELEMENT_ARRAY_BUFFER)
        currObj = get_buffer_from_index(vao->elementArrayBufferObject);
    
    if(!currObj) return;

    uint32_t bind = 0;
    if(target == GL_ARRAY_BUFFER)
        bind |= PIPE_BIND_VERTEX_BUFFER;
    else if(target == GL_ELEMENT_ARRAY_BUFFER)
        bind |= PIPE_BIND_INDEX_BUFFER;

    if(!currObj->bufferID)
        currObj->bufferID = create_simple_buffer(gpuNode, size, bind, NULL);

    send_inline_write(gpuNode, currObj->bufferID, 0, 0, (virtio_box_t){
        0, 0, 0,
        size, 1, 1
    }, data, size, 0, 0);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);
}

GL_APICALL GLuint GL_APIENTRY glCreateShader (GLenum type) { // TODO: Check types
    GLuint id = alloc_opengl_shader();
    get_shader_from_index(id)->type = type;
    get_shader_from_index(id)->string = NULL;
    get_shader_from_index(id)->shaderID = 0;

    return id;
}

GL_APICALL void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) {
    gl_shader_object_t *shaderObj = get_shader_from_index(shader);
    if(!shaderObj) return;

    if(length == NULL) {

        uint32_t strLength = 0;
        for(size_t i = 0; i < count; i++) {
            strLength += strlen(string[i]);
        }
        shaderObj->string = realloc(shaderObj->string, strLength + 1);
        uint32_t lastOffset = 0;
        for(size_t i = 0; i < count; i++) {
            memcpy((void *)((size_t)shaderObj->string + lastOffset), string[i], strlen(string[i]));
            lastOffset = strlen(string[i]);
        }
        shaderObj->string[strLength] = '\0';
    } else {
        //TODO
    }
}

GL_APICALL void GL_APIENTRY glCompileShader (GLuint shader) {
    gl_shader_object_t *shaderObj = get_shader_from_index(shader);
    if(!shaderObj) return;

    /*uint32_t type = 0;

    if(shaderObj->type == GL_VERTEX_SHADER)
        type = 0;
    else if(shaderObj->type == GL_FRAGMENT_SHADER)
        type = 1;
    else if(shaderObj->type == GL_GEOMETRY_SHADER)
        type = 2;

    shaderObj->shaderID = create_shader(gpuNode, shaderObj->string, type, 0);*/
    //We will compile the shader when we link to merge all the consts
}

GL_APICALL GLuint GL_APIENTRY glCreateProgram (void) {
    GLuint id = alloc_opengl_program();
    get_program_from_index(id)->vertexShader = NULL;
    get_program_from_index(id)->fragmentShader = NULL;
    get_program_from_index(id)->geometryShader = NULL;
    get_program_from_index(id)->uniformSymbols = NULL;
    get_program_from_index(id)->constantBuffer = NULL;
    get_program_from_index(id)->constantBufferID = 0;
    get_program_from_index(id)->constantBufferSize = 0;

    return id;
}

GL_APICALL void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) {
    gl_program_object_t *programObj = get_program_from_index(program);
    if(!programObj) return;

    gl_shader_object_t *shaderObj = get_shader_from_index(shader);
    if(!shaderObj) return;

    switch(shaderObj->type) {
        case GL_VERTEX_SHADER:
            programObj->vertexShader = shaderObj;
            break;
        case GL_FRAGMENT_SHADER:
            programObj->fragmentShader = shaderObj;
            break;
        case GL_GEOMETRY_SHADER:
            programObj->geometryShader = shaderObj;
            break;
        default:
            break;
    }
}

static void compile_shader (gl_shader_object_t *shaderObj, bool preserve) {
    if(!shaderObj) return;

    uint32_t type = 0;

    if(shaderObj->type == GL_VERTEX_SHADER)
        type = 0;
    else if(shaderObj->type == GL_FRAGMENT_SHADER)
        type = 1;
    else if(shaderObj->type == GL_GEOMETRY_SHADER)
        type = 2;

    char *strTmp;

    if(shaderObj->type == GL_VERTEX_SHADER)
        strTmp = compile_glsl_vertex(shaderObj->string, preserve, preserve);
    else if(shaderObj->type == GL_FRAGMENT_SHADER)
        strTmp = compile_glsl_fragment(shaderObj->string, preserve, preserve);

    free(shaderObj->string);
    shaderObj->string = strTmp;

    shaderObj->shaderID = create_shader(gpuNode, shaderObj->string, type, 0);
}

GL_APICALL void GL_APIENTRY glLinkProgram (GLuint program) {
    gl_program_object_t *programObj = get_program_from_index(program);
    if(!programObj) return;

    if(programObj->vertexShader)
        compile_shader(programObj->vertexShader, false);

    if(programObj->fragmentShader)
        compile_shader(programObj->fragmentShader, true);

    if(programObj->geometryShader)
        compile_shader(programObj->geometryShader, true);

    programObj->uniformSymbols = get_copy_of_last_execution_symbols();

    identifier_t *symbolsTable = programObj->uniformSymbols;
    identifier_t *currentId = symbolsTable;
    uint32_t constantBufferSize = 0; // Needs to be multiplied by four, all constants are 4 component vectors
    while(currentId->token) {
        if(currentId->value > constantBufferSize) {
            constantBufferSize = currentId->value;
        }
        currentId = currentId + sizeof(identifier_t);
    }

    constantBufferSize *= sizeof(float) * 4;
    constantBufferSize += sizeof(float) * 4 * 4; //todo, check types

    programObj->constantBuffer = malloc(constantBufferSize);
    memset(programObj->constantBuffer, 0, constantBufferSize);

    programObj->constantBufferID = create_simple_buffer(gpuNode, constantBufferSize, PIPE_BIND_CONSTANT_BUFFER, NULL);

    programObj->constantBufferSize = constantBufferSize;

    return;
}

gl_program_object_t *currentProgram = NULL;

GL_APICALL void GL_APIENTRY glUseProgram (GLuint program) {
    gl_program_object_t *programObj = get_program_from_index(program);
    if(!programObj) return;

    if(programObj->vertexShader && programObj->vertexShader->shaderID)
        bind_shader(gpuNode, programObj->vertexShader->shaderID, 0x0);

    if(programObj->fragmentShader && programObj->fragmentShader->shaderID)
        bind_shader(gpuNode, programObj->fragmentShader->shaderID, 0x1);

    if(programObj->geometryShader && programObj->geometryShader->shaderID)
        bind_shader(gpuNode, programObj->geometryShader->shaderID, 0x2);

    //We can use this to set multiple buffers
    set_uniform_buffer(gpuNode, 0, programObj->constantBufferSize, programObj->constantBufferID);
    set_uniform_buffer(gpuNode, 1, programObj->constantBufferSize, programObj->constantBufferID);
    set_uniform_buffer(gpuNode, 2, programObj->constantBufferSize, programObj->constantBufferID);

    currentProgram = programObj;
}

enum { BOOL, INT, UINT, FLOAT, DOUBLE, LAYOUT, UNIFORM, OUT, IN, VEC, MAT };

GL_APICALL GLint GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar *name) {
    gl_program_object_t *programObj = get_program_from_index(program);
    if(!programObj) return -1;

    if(!programObj->uniformSymbols) return -1;

    identifier_t *symbolsTable = programObj->uniformSymbols;
    identifier_t *currentId = symbolsTable;
    GLint j = 0;
    while(currentId->token) {
        if(!strcmp(currentId->name, (char *)name) && currentId->variableType == UNIFORM /* In case we missed something */) {
            return currentId->value; // Precalculated
        }

        currentId = currentId + sizeof(identifier_t);
    }

    return -1;
}

static void update_constant_buffer(gl_program_object_t *programObj) {
    if(!programObj || programObj->constantBufferID == 0 || programObj->constantBuffer == NULL) return;

    send_inline_write(gpuNode, programObj->constantBufferID, 0, (1 << 1), (virtio_box_t){
        0, 0, 0,
        programObj->constantBufferSize, 1, 1
    }, programObj->constantBuffer, programObj->constantBufferSize, 0, 0);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);
}

GL_APICALL void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    //for now we omit transpose
    float *currentLocation = &currentProgram->constantBuffer[location * 4];

    memcpy(currentLocation, value, sizeof(float) * 4 * 4 * count);

    update_constant_buffer(currentProgram);
}

void windows_flush() {
    virtgpu_transfer_and_flush_t transferAndFlushData = {0};

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    transferAndFlushData.notTransfer = 1;
    transferAndFlushData.width = windowWidth;
    transferAndFlushData.height = windowHeight;
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &framebufferIdentidier);
    ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
}
