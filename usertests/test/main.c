int printf(const char* format, ...);

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "../../modules/virtiogpu/virtiogpu.h"
#include <stddef.h>
#include "gl.h"

uint8_t fontBitmap[];

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

//TODO: ERRNO

void debug_print(char *str) {
    asm volatile("syscall" : : "D"(str), "a"(0x0) : "rcx", "r11", "rdx");
}

size_t fopen(char *filePath, size_t flags) {
    size_t ret;

    asm volatile("syscall" : "=a"(ret) : "D"(filePath), "S"(flags), "a"(0x1) : "rcx", "r11", "rdx");

    return ret;
}

size_t fread(size_t fd, void *buffer, size_t count) {
    size_t ret;

    asm volatile("syscall" : "=a"(ret) : "D"(fd), "S"(buffer), "d"(count), "a"(0x2) : "rcx", "r11");

    return ret;
}

size_t ioctl(size_t fd, size_t request, void *argp) {
    size_t ret;

    asm volatile("syscall" : "=a"(ret) : "D"(fd), "S"(request), "d"(argp), "a"(0x5) : "rcx", "r11");

    return ret;
}

void *currentAddr = NULL;

void *sbrk(size_t count) {
    if((int64_t)count <= 0) {
        return currentAddr;
    }

    void *ret;

    size_t pages = ROUND_UP(count, 0x1000) / 0x1000;

    asm volatile("syscall" : "=a"(ret) : "D"(NULL), "S"(pages), "a"(0x6) : "rcx", "r11", "rdx");

    currentAddr = ret;

    return ret;
}

size_t fork() {
    size_t ret;

    asm volatile("syscall" : "=a"(ret) : "a"(0x7) : "rcx", "r11", "rdx");

    return ret;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

size_t strlen(const char *str) {
    size_t len;

    for (len = 0; str[len]; len++);

    return len;
}

char *strcpy(char *dest, const char *src) {
    size_t i;

    for (i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *rdest = dest;

    while (*dest) dest++;
    while ((*dest++ = *src++));
    return rdest;
}

typedef char ALIGN[16];

union header {
	struct {
		size_t size;
		unsigned is_free;
		union header *next;
	} s;
	ALIGN stub;
};
typedef union header header_t;

header_t *head, *tail;

header_t *get_free_block(size_t size)
{
	header_t *curr = head;
	while(curr) {
		if (curr->s.is_free && curr->s.size >= size)
			return curr;
		curr = curr->s.next;
	}
	return NULL;
}

void *malloc(size_t size)
{
	size_t total_size;
	void *block;
	header_t *header;
	if (!size)
		return NULL;
	header = get_free_block(size);
	if (header) {
		header->s.is_free = 0;
		return (void*)(header + 1);
	}
	total_size = sizeof(header_t) + size;
	block = sbrk(total_size);
	if (block == (void*) -1) {
		return NULL;
	}
	header = block;
	header->s.size = size;
	header->s.is_free = 0;
	header->s.next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->s.next = header;
	tail = header;
	return (void*)(header + 1);
}

void free(void *block)
{
	header_t *header, *tmp;
	void *programbreak;

	if (!block)
		return;
	header = (header_t*)block - 1;

	programbreak = sbrk(0);
	if ((char*)block + header->s.size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->s.next == tail) {
					tmp->s.next = NULL;
					tail = tmp;
				}
				tmp = tmp->s.next;
			}
		}
		//sbrk(0 - sizeof(header_t) - header->s.size);
		return;
	}
	header->s.is_free = 1;
}

void *calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	/* check mul overflow */
	if (nsize != size / num)
		return NULL;
	block = malloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}

void *realloc(void *block, size_t size)
{
	header_t *header;
	void *ret;
	if (!block || !size)
		return malloc(size);
	header = (header_t*)block - 1;
	if (header->s.size >= size)
		return block;
	ret = malloc(size);
	if (ret) {
		
		memcpy(ret, block, header->s.size);
		free(block);
	}
	return ret;
}

void *memcpy(void *d, const void *s, size_t n) {
    uint8_t *p = d;
    uint8_t *p2 = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = p2[i];
    }

    return d;
}

char *strdup(const char *src) {
    int len = strlen(src) + 1;
    char * dst = (char *)malloc(len);
    memcpy((void *)dst, (void *)src, len);
    return dst;
}

static void render_char(uint32_t *framebuffer, uint32_t pitch, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    uint32_t *line = framebuffer + y * 16 * pitch + x * 8;
    
    for(size_t i = 0; i < 16; i++) {
        uint32_t *dest = line;
        uint8_t dc = (c >= 32 && c <= 127) ? c : 127;
        uint8_t fontbits = fontBitmap[(dc - 32) * 16 + i];
        for(size_t j = 0; j < 8; j++) {
            int bit = (1 << ((8 - 1) - j));
            *dest++ = (fontbits & bit) ? fg : bg;
        }
        line += pitch;
    }

    asm volatile ("" : : : "memory");
}

void render_string(uint32_t *framebuffer, uint32_t x, uint32_t y, const char *c, uint32_t fg, uint32_t bg) {
    size_t count = strlen(c);
    size_t currX = x;
    for(size_t i = 0; i < count; i++) {
        if(c[i] == '\n') {
            y++;
            x = 0;
            currX = 0;
            continue;
        }
        if(x + currX >= (1024 / 8)) {
            y++;
            x = 0;
            currX = 0;
        }
        render_char(framebuffer, 1024, currX, y, c[i], fg, bg);
        currX++;
    }
}

int strncmp(const char *s1, const char *s2, int c) {
    for (size_t i = 0; i < c; i++) {
        char c1 = s1[i], c2 = s2[i];
        if (c1 != c2)
            return c1 - c2;
        if (!c1)
            return 0;
    }

    return 0;
}

static volatile uint32_t currOpenglResource = 1;

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
static void draw_vbo(size_t gpuNode, uint32_t start, uint32_t count, uint32_t mode) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_DRAW_VBO;
    command.option = 0;
    command.length = 12;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 12);
    command.parameters[0] = start; // Start
    command.parameters[1] = count; // Count
    command.parameters[2] = mode; // Mode PIPE_PRIM_TRIANGLES
    command.parameters[3] = 0; // Indexed
    command.parameters[4] = 0; // Instace Count
    command.parameters[5] = 0; // Index Bias
    command.parameters[6] = 0; // Start Instance
    command.parameters[7] = 0; // Primitive Restart
    command.parameters[8] = 0; // Restart Index
    command.parameters[9] = 0; // Min Index
    command.parameters[10] = 0; // Max Index
    command.parameters[11] = 0; // Cso

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
}

struct vertex {
   float position[4];
   float color[4];
};

static struct vertex vertices[12 * 3] =
{
   {
      { -1.0f, -1.0f, -1.0f, 1.0f },
      { 0.583f,  0.771f,  0.014f, 1.0f }
   },
   {
      { -1.0f, -1.0f, 1.0f, 1.0f },
      { 0.609f,  0.115f,  0.436f, 1.0f }
   },
   {
      { -1.0f, 1.0f, 1.0f, 1.0f },
      { 0.327f,  0.483f,  0.844f, 1.0f }
   },
   {
      { 1.0f, 1.0f, -1.0f, 1.0f },
      { 0.822f,  0.569f,  0.201f, 1.0f }
   },
   {
      { -1.0f, -1.0f, -1.0f, 1.0f },
      { 0.435f,  0.602f,  0.223f, 1.0f }
   },
   {
      { -1.0f, 1.0f, -1.0f, 1.0f },
      { 0.310f,  0.747f,  0.185f, 1.0f }
   },

   {
      { 1.0f, -1.0f, 1.0f, 1.0f },
      { 0.597f,  0.770f,  0.761f, 1.0f }
   },
   {
      { -1.0f, -1.0f, -1.0f, 1.0f },
      { 0.559f,  0.436f,  0.730f, 1.0f }
   },
   {
      { 1.0f, -1.0f, -1.0f, 1.0f },
      { 0.359f,  0.583f,  0.152f, 1.0f }
   },

   {
      { 1.0f, 1.0f, -1.0f, 1.0f },
      { 0.483f,  0.596f,  0.789f, 1.0f }
   },
   {
      { 1.0f, -1.0f, -1.0f, 1.0f },
      { 0.559f,  0.861f,  0.639f, 1.0f }
   },
   {
      { -1.0f, -1.0f, -1.0f, 1.0f },
      { 0.195f,  0.548f,  0.859f, 1.0f }
   },

   {
      {-1.0f,-1.0f,-1.0f,  1.0f},
      {0.014f,  0.184f,  0.576f, 1.0f}
   },

  {{ -1.0f, 1.0f, 1.0f, 1.0f,},
  {0.771f,  0.328f,  0.970f, 1.0f}},

  {{ -1.0f, 1.0f,-1.0f, 1.0f,},
  {0.406f,  0.615f,  0.116f, 1.0f}},

  {{ 1.0f,-1.0f, 1.0f,  1.0f,},
  {0.676f,  0.977f,  0.133f, 1.0f}},

  {{ -1.0f,-1.0f, 1.0f, 1.0f,},
  {0.971f,  0.572f,  0.833f, 1.0f}},

  {{ -1.0f,-1.0f,-1.0f, 1.0f,},
  {0.140f,  0.616f,  0.489f, 1.0f}},

  {{ -1.0f, 1.0f, 1.0f, 1.0f,},
  {0.997f,  0.513f,  0.064f, 1.0f}},

  {{ -1.0f,-1.0f, 1.0f, 1.0f,},
  {0.945f,  0.719f,  0.592f, 1.0f}},

  {{ 1.0f,-1.0f, 1.0f,  1.0f,},
  {0.543f,  0.021f,  0.978f, 1.0f}},

  {{ 1.0f, 1.0f, 1.0f,  1.0f,},
  {0.279f,  0.317f,  0.505f, 1.0f}},

  {{ 1.0f,-1.0f,-1.0f,  1.0f,},
  {0.167f,  0.620f,  0.077f, 1.0f}},

  {{ 1.0f, 1.0f,-1.0f,  1.0f,},
  {0.347f,  0.857f,  0.137f, 1.0f}},

  {{ 1.0f,-1.0f,-1.0f,  1.0f,},
  {0.055f,  0.953f,  0.042f, 1.0f}},

  {{ 1.0f, 1.0f, 1.0f,  1.0f,},
  {0.714f,  0.505f,  0.345f, 1.0f}},

  {{ 1.0f,-1.0f, 1.0f,  1.0f,},
  {0.783f,  0.290f,  0.734f, 1.0f}},

  {{ 1.0f, 1.0f, 1.0f,  1.0f,},
  {0.722f,  0.645f,  0.174f, 1.0f}},

  {{ 1.0f, 1.0f,-1.0f,  1.0f,},
  {0.302f,  0.455f,  0.848f, 1.0f}},

  {{ -1.0f, 1.0f,-1.0f, 1.0f,},
  {0.225f,  0.587f,  0.040f, 1.0f}},

  {{ 1.0f, 1.0f, 1.0f,  1.0f,},
  {0.517f,  0.713f,  0.338f, 1.0f}},

  {{ -1.0f, 1.0f,-1.0f, 1.0f,},
  {0.053f,  0.959f,  0.120f, 1.0f}},

  {{ -1.0f, 1.0f, 1.0f, 1.0f,},
  {0.393f,  0.621f,  0.362f, 1.0f}},

  {{ 1.0f, 1.0f, 1.0f,  1.0f,},
  {0.673f,  0.211f,  0.457f, 1.0f}},

  {{ -1.0f, 1.0f, 1.0f, 1.0f,},
  {0.820f,  0.883f,  0.371f, 1.0f}},

  {{ 1.0f,-1.0f, 1.0f,   1.0f},
  {0.982f,  0.099f,  0.879f, 1.0f}},
};

#define FORCE_EVAL(x) do {                        \
	if (sizeof(x) == sizeof(float)) {         \
		volatile float __x;               \
		__x = (x);                        \
	} else if (sizeof(x) == sizeof(double)) { \
		volatile double __x;              \
		__x = (x);                        \
	} else {                                  \
		volatile long double __x;         \
		__x = (x);                        \
	}                                         \
} while(0)

float floorf(float x)
{
	union {float f; uint32_t i;} u = {x};
	int e = (int)(u.i >> 23 & 0xff) - 0x7f;
	uint32_t m;

	if (e >= 23)
		return x;
	if (e >= 0) {
		m = 0x007fffff >> e;
		if ((u.i & m) == 0)
			return x;
		FORCE_EVAL(x + 0x1p120f);
		if (u.i >> 31)
			u.i += m;
		u.i &= ~m;
	} else {
		FORCE_EVAL(x + 0x1p120f);
		if (u.i >> 31 == 0)
			u.i = 0;
		else if (u.i << 1)
			u.f = -1.0;
	}
	return u.f;
}

const float invtwopi=0.1591549f;
const float twopi=6.283185f;
const float threehalfpi=4.7123889f;
const float pi=3.141593f;
const float halfpi=1.570796f;
const float quarterpi=0.7853982f;

float cos_32s(float x)
{
    const float c1= 0.99940307f;
    const float c2=-0.49558072f;
    const float c3= 0.03679168f;
    float x2;      // The input argument squared
    x2=x * x;
    return (c1 + x2*(c2 + c3 * x2));
}
float cosine(float angle){
    //clamp to the range 0..2pi
    angle=angle-floorf(angle*invtwopi)*twopi;
    angle=angle>0.f?angle:-angle;

    if(angle<halfpi) return cos_32s(angle);
    if(angle<pi) return -cos_32s(pi-angle);
    if(angle<threehalfpi) return -cos_32s(angle-pi);
    return cos_32s(twopi-angle);
}
float sine(float angle){
    return cosine(halfpi-angle);
}

float constants[] = 
{  0.4, 0, 0,  1,
    1,   1, 1,  1,
    2,   2, 2,  2,
    4,   8, 16, 32,

    1,  0, 0, 0,
    0, 1.33333333333333333333f, 0, 0,
    0,  0, -1.0020020020020020020f, -1,
    0,  0, -2.0020020020020020020f, 0,

    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, -3, 1,

    0.5, 0, -0.5, 0,
    0, 1, 0, 0,
    0.5, 0, 0.5, 0,
    0, 0, 0, 1
};

static struct vertex vertices2[8] = {
    {
        {-1.0f, -1.0f, 1.0f, 1.0f},
        {0.609f,  0.115f,  0.436f, 1.0f}
    },
    {
        { 1.0f, -1.0f, 1.0f, 1.0f},
        {0.597f,  0.770f,  0.761f, 1.0f}
    },  // bottom right
    {
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.279f,  0.317f,  0.505f, 1.0f}
    },  // bottom left
    {
        {-1.0f,  1.0f, 1.0f, 1.0f},
        {0.327f,  0.483f,  0.844f, 1.0f}
    },  // top left 

    {
        {-1.0f, -1.0f, -1.0f, 1.0f},
        {0.583f,  0.771f,  0.014f, 1.0f}
    },
    {
        { 1.0f, -1.0f, -1.0f, 1.0f},
        {0.559f,  0.861f,  0.639f, 1.0f}
    },  // bottom right
    {
        { 1.0f,  1.0f, -1.0f, 1.0f},
        {0.347f,  0.857f,  0.137f, 1.0f}
    },  // bottom left
    {
        {-1.0f,  1.0f, -1.0f, 1.0f},
        {0.310f,  0.747f,  0.185f, 1.0f}
    },  // top left 
};
unsigned int indices[] = {  // note that we start from 0!
    0, 2, 1,
    0, 3, 2,

    4, 6, 5,
    4, 7, 6,

    4, 0, 3,
    4, 3, 7,

    1, 6, 5,
    1, 2, 6,

    3, 6, 2,
    3, 7, 6,

    1, 0, 4,
    1, 4, 5
}; 

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

uint8_t tex2d[16][16][4];

#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )

extern void windows_flush();

#include "opengl.h"

//#define DONT_USE_EBO

int main() {
    tex2d[0][0][0] = 0;
    tex2d[0][0][1] = 255;
    tex2d[0][0][2] = 255;
    tex2d[0][0][3] = 0;

    tex2d[0][1][0] = 0;
    tex2d[0][1][1] = 0;
    tex2d[0][1][2] = 255;
    tex2d[0][1][3] = 255;

    tex2d[1][0][0] = 255;
    tex2d[1][0][1] = 255;
    tex2d[1][0][2] = 0;
    tex2d[1][0][3] = 255;

    tex2d[1][1][0] = 255;
    tex2d[1][1][1] = 0;
    tex2d[1][1][2] = 0;
    tex2d[1][1][3] = 255;

    debug_print("Hello world\n");
    debug_print("Hello userland world\n");

    printf("%lx\n", (size_t)&vertices);

    size_t fd = fopen("/hola.txt", 0);
    if(fd == -1) return -1;

    printf("FD: 0x%x\n", fd);

    char testStr2[8];
    size_t ret = fread(fd, testStr2, 7);
    if(ret == -1) return -1;

    testStr2[7] = '\0';
    printf("%s\n", testStr2);

    size_t gpuNode = fopen("/dev/gpu", 0);

    virtio_mode_t *virtioModes = (virtio_mode_t *)sbrk(sizeof(virtio_mode_t) * 16);
    ioctl(gpuNode, VIRTGPU_IOCTL_GET_DISPLAY_INFO, (void *)virtioModes);

    virtgpu_create_resource_2d_t resourceData = {0};
    virtgpu_attach_backing_t attachBackingData = {0};
    virtgpu_set_scanout_t scanoutData = {0};
    virtgpu_transfer_and_flush_t transferAndFlushData = {0};

    resourceData.format = 1;

    for(size_t i = 0; i < 16; i++) {
        if(virtioModes[i].enabled) {
            printf("%u: x %u, y %u, w %u, h %u, flags 0x%x\n", i, virtioModes[i].rect.x, virtioModes[i].rect.y, virtioModes[i].rect.width, virtioModes[i].rect.height, virtioModes[i].flags);
            resourceData.width = scanoutData.width = transferAndFlushData.width = virtioModes[i].rect.width;
            resourceData.height = scanoutData.height = transferAndFlushData.height = virtioModes[i].rect.height;
            scanoutData.scanoutID = i;
        }
    }

    uint32_t resourceID = (uint32_t)ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_2D, (void *)&resourceData);
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);

    uint32_t framebufferSize = resourceData.width * resourceData.height * 4;
    uint32_t *framebuffer = (uint32_t *)sbrk(framebufferSize + 0x1000);

    attachBackingData.address = (uint64_t)framebuffer;
    attachBackingData.length = framebufferSize;

    ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_SCANOUT, (void *)&scanoutData);

    for(size_t i = 0; i < 768; i++) {
        for(size_t j = 0; j < 1024; j++) {
            uint32_t *pixel = &framebuffer[j + i * 1024];
            *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
        }
    }

    render_string(framebuffer, 0, 0, "Hola que tal?\n Yo bien y tu?\n\nPues tambien bien :D", 0xFFFFFFFF, 0x0);

    ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
    
    /*if(fork()) {
        printf("Yeah new\n");
        while(1);
    } else {
        printf("Yeah old\n");
    }*/

    virtgpu_create_gpu_context_t createContext = {0};
    createContext.name = strdup("mainContext");
    createContext.nameLength = strlen("mainContext");

    uint32_t contextID = ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_GPU_CONTEXT, &createContext);
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_CONTEXT_ID, &contextID);

    virtgpu_3d_command_t command = {0};
    char *flag = "tgsi=1";
    uint32_t slen = strlen(flag) + 1;
    uint32_t sslen = (slen + 3) & ~3;
    uint32_t string_length = (uint32_t)MIN2(sslen, slen);

    command.command = VIRGL_CCMD_SET_DEBUG_FLAGS;
    command.option = 0;
    command.length = sslen / 4;
    command.parameters = (uint32_t *)realloc(command.parameters, sslen);
    memcpy(command.parameters, flag, string_length);

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);


    uint32_t *parameters = NULL;

    init_opengl(gpuNode, resourceID, 1024, 768);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const char *glslCode = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "out vec4 vertexColor;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   vertexColor = vec4(0.5, 0.0, 0.0, 1.0);\n"
    "}\n"
    "\n";

    //Time to implement some type of compiler

#if 1
    bool running = true;
    char *currString = glslCode;
    char **attributesVariableNames = malloc(16 * sizeof(char *));
    while(running) {
        if(!strncmp(currString, GLSL_TOKEN_VERSION, strlen(GLSL_TOKEN_VERSION))) {
            currString += strlen(GLSL_TOKEN_VERSION);
            while(true) {
                if(currString[0] == '\n') {
                    printf("Found version token\n");
                    currString++;
                    break;
                }
                currString++;
            }
        } else if(!strncmp(currString, GLSL_TOKEN_LAYOUT, strlen(GLSL_TOKEN_LAYOUT))) {
            currString += strlen(GLSL_TOKEN_LAYOUT) + 1;
            if(currString[0] == '(') {
                currString++;
                if(!strncmp(currString, "location", strlen("location"))) {
                    currString += strlen("location") + 3;
                    uint32_t location = currString[0] - '0';
                    while(currString[0] != ')') currString++;
                    currString++;
                    
                    char *tempName = NULL;
                    char *variableName = NULL;
                    while(true) {
                        if(currString[0] != ' ' && currString[0] != '\n') {
                            if(!strncmp(currString, "in", 2)) {
                                currString += 3;
                                continue;
                            } else if(!strncmp(currString, "vec", 3)) {
                                currString += 3 + 1 + 1;
                                continue;
                            } else if(currString[0] == ';') {
                                variableName = malloc(currString - tempName);
                                memcpy(variableName, tempName, currString - tempName);
                                variableName[currString - tempName] = 0;
                            } else { // Variable name
                                if(tempName == NULL)
                                    tempName = currString;
                            }
                        }
                        if(currString[0] == '\n') {
                            currString++;
                            break;
                        }
                        currString++;
                    }
                    attributesVariableNames[location] = variableName;
                    printf("Found layout token, location = %x, name = %s\n", location, variableName);
                }
            } else {
                printf("Error while compiling glsl to tgsi\n");
                break;
            }
        } else if(!strncmp(currString, GLSL_TOKEN_OUT, strlen(GLSL_TOKEN_OUT))) {
            currString += strlen(GLSL_TOKEN_OUT) + 1;
            if(!strncmp(currString, "vec", 3)) {
                currString += 3;
                uint32_t vecSize = currString[0] - '0'; currString += 2;
                char *tempString = currString;
                while(true) {
                    if(currString[0] == '\n') {
                        char *variableName = malloc(currString - tempString);
                        memcpy(variableName, tempString, currString - tempString - 1); variableName[currString - tempString - 1] = 0;
                        printf("New out vec%x variable named as: %s\n", vecSize, variableName);
                        currString++;
                        break;
                    }
                    currString++;
                }
            }
        } else if(!strncmp(currString, GLSL_TOKEN_VOID, strlen(GLSL_TOKEN_VOID))) {
            currString += strlen(GLSL_TOKEN_VOID) + 1;
            if(!strncmp(currString, "main", 3)) {
                while(currString[0] != '{') currString++;
                currString++;

                //Code block here
                bool blockCode = true;
                while(blockCode) {
                    switch(currString[0]) {
                        case '\n':
                            currString++;
                            break;
                        case '}':
                            currString++;
                            blockCode = false;
                            break;
                        default:
                            {
                                if(!strncmp(currString, "gl_Position", 11)) {
                                    currString += 11;
                                } else {
                                    for(uint32_t i = 0; i < 16; i++) { // TODO: Create a function for this
                                        if(attributesVariableNames[i] && !strncmp(currString, attributesVariableNames[i], strlen(attributesVariableNames[i]))) {
                                            //We have a variable that we know
                                            while(*currString != '=') currString++; // Skip until equal
                                            currString++;

                                            while(*currString == ' ') currString++; // Skip until a new character non empty

                                            // Parse code block
                                            bool blockCode2 = true;
                                            while(blockCode) {
                                                switch(currString[0]) {
                                                    case '\n':
                                                        currString++;
                                                        break;
                                                    case '}':
                                                        currString++;
                                                        blockCode = false;
                                                        break;
                                                    default:
                                                        {
                                                            if(!strncmp(currString, "vec", 3)) {
                                                                currString += 3;
                                                                uint32_t vecSize = currString[0] - '0'; currString++;
                                                                
                                                                while(*currString != '(') currString++;
                                                                currString++;

                                                                //Parse the vector Ex: vec3 -> [(variable.xy, number), (variable.x, number, number), ...]
                                                                for(uint32_t j = 0; j < vecSize; j++) {
                                                                    switch(*currString) {
                                                                        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                                                                            //Parse the float
                                                                            break;

                                                                        default:
                                                                            //Variable: save it, check size, and add the size to index j, if greater than or equal to vecSize -> syntax error
                                                                            break;
                                                                    }
                                                                }
                                                            } else {
                                                                printf("%c\n", currString[0]);
                                                                currString++;
                                                            }
                                                        }
                                                        break;
                                                }
                                            }
                                        }
                                    }
                                    printf("%c\n", currString[0]);
                                    currString++;
                                }
                            }
                            break;
                    }
                }
            } else {

            }
        } else if(currString[0] == '\n' || currString[0] == ' ') {
            currString++;
        } else if(currString[0] == 0) {
            running = false;
        } else {
            printf("Error while compiling glsl to tgsi:\n%x\n", currString[0]);
            running = false;
        }
    }

    while(1);
#endif

#ifdef DONT_USE_EBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
#else
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), &vertices2, GL_STATIC_DRAW);

//TODO
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
#endif

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)(4 * sizeof(GLfloat)));

/*
    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 8);
    parameters[0 * 4 + 0] = offsetof(struct vertex, position); // src offset
    parameters[0 * 4 + 1] = 0; // instance divisor
    parameters[0 * 4 + 2] = 0; // vertex buffer index
    parameters[0 * 4 + 3] = 31; // src format

    parameters[1 * 4 + 0] = offsetof(struct vertex, color); // src offset
    parameters[1 * 4 + 1] = 0; // instance divisor
    parameters[1 * 4 + 2] = 0; // vertex buffer index
    parameters[1 * 4 + 3] = 31; // src format
    uint32_t vertexElementsID = create_opengl_object(gpuNode, VIRGL_OBJECT_VERTEX_ELEMENTS, parameters, 8, 1);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    uint32_t resource3D = create_simple_buffer(gpuNode, sizeof(vertices), PIPE_BIND_VERTEX_BUFFER, NULL);

    send_inline_write(gpuNode, resource3D, 0, 0, (virtio_box_t){
        0, 0, 0,
        sizeof(vertices), 1, 1
    }, &vertices, sizeof(vertices), 0, 0);

    set_vertex_buffers(gpuNode, sizeof(struct vertex), 0, resource3D);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);*/


    uint32_t xfbID = create_simple_buffer(gpuNode, 12 * 3 * sizeof(struct vertex), PIPE_BIND_STREAM_OUTPUT, NULL);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 3);
    parameters[0] = xfbID; // Resource ID
    parameters[1] = 0; // Buffer offset
    parameters[2] = 12 * 3 * sizeof(struct vertex);
    uint32_t streamoutID = create_opengl_object(gpuNode, VIRGL_OBJECT_STREAMOUT_TARGET, parameters, 3, 0);

    set_streamout_targets(gpuNode, 0, streamoutID);


    const char *geometryShader =
	   "GEOM\n"
	   "PROPERTY GS_INPUT_PRIMITIVE TRIANGLES\n"
	   "PROPERTY GS_OUTPUT_PRIMITIVE TRIANGLE_STRIP\n"
	   "PROPERTY GS_MAX_OUTPUT_VERTICES 3\n"
	   "PROPERTY GS_INVOCATIONS 1\n"
	   "DCL IN[][0], POSITION\n"
	   "DCL IN[][1], GENERIC[20]\n"
	   "DCL OUT[0], POSITION\n"
	   "DCL OUT[1], GENERIC[20]\n"
	   "IMM[0] INT32 {0, 0, 0, 0}\n"
	   "  0:MOV OUT[0], IN[0][0]\n"
	   "  1:MOV OUT[1], IN[0][1]\n"
	   "  2:EMIT IMM[0].xxxx\n"
	   "  3:MOV OUT[0], IN[1][0]\n"
	   "  4:MOV OUT[1], IN[0][1]\n"
	   "  5:EMIT IMM[0].xxxx\n"
	   "  6:MOV OUT[0], IN[2][0]\n"
	   "  7:MOV OUT[1], IN[2][1]\n"
	   "  8:EMIT IMM[0].xxxx\n"
	   "  9:END\n";

    const char *vertexString =
	   "VERT\n"
	   "DCL IN[0]\n"
	   "DCL IN[1]\n"
	   "DCL OUT[0], POSITION\n"
	   "DCL OUT[1], COLOR\n"
       "DCL CONST[1][0..15]\n"
       "DCL TEMP[0..14], LOCAL\n"
       "IMM[0] UINT32 {0, 128, 144, 160}\n"
       "IMM[1] UINT32 {176, 192, 112, 96}\n"
       "IMM[2] UINT32 {80, 64, 16, 48}\n"
       "  0: MUL TEMP[0], CONST[1][12], IN[0].xxxx\n"
       "  1: MAD TEMP[1], CONST[1][13], IN[0].yyyy, TEMP[0]\n"
       "  2: MAD TEMP[2], CONST[1][14], IN[0].zzzz, TEMP[1]\n"
       "  3: MAD TEMP[3], CONST[1][15], IN[0].wwww, TEMP[2]\n"
       "  4: MUL TEMP[4], CONST[1][8], TEMP[3].xxxx\n"
       "  5: MAD TEMP[5], CONST[1][9], TEMP[3].yyyy, TEMP[4]\n"
       "  6: MAD TEMP[6], CONST[1][10], TEMP[3].zzzz, TEMP[5]\n"
       "  7: MAD TEMP[7], CONST[1][11], TEMP[3].wwww, TEMP[6]\n"
       "  8: MUL TEMP[8], CONST[1][4], TEMP[7].xxxx\n"
       "  9: MAD TEMP[9], CONST[1][5], TEMP[7].yyyy, TEMP[8]\n"
       "  10: MAD TEMP[10], CONST[1][6], TEMP[7].zzzz, TEMP[9]\n"
       "  11: MAD TEMP[11], CONST[1][7], TEMP[7].wwww, TEMP[10]\n"
       "  12: ADD TEMP[12], IN[1], CONST[1][1]\n"
       "  13: RCP TEMP[13], CONST[1][3].xxxx\n"
       "  14: MUL TEMP[14], TEMP[12], TEMP[13]\n"
       "  15: MOV OUT[0], TEMP[11]\n"
       "  16: MOV OUT[1], TEMP[14]\n"
       "  17: END\n";

    const char *fragmentString =
	   "FRAG\n"
	    "DCL IN[0], COLOR, LINEAR\n"
	    "DCL OUT[0], COLOR\n"
        "DCL TEMP[0]\n"
        "DCL SAMP[0]\n"
        "DCL SVIEW[0], 2D, FLOAT\n"
        "  0: TXP TEMP[0], IN[0], SAMP[0], 2D\n"
	    "  1: MOV OUT[0], IN[0]\n"
	    "  2: END\n";

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexString, NULL);
    glCompileShader(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentString, NULL);
    glCompileShader(fShader);

    GLuint programID = glCreateProgram();
    glAttachShader(programID, vShader);
    glAttachShader(programID, fShader);
    glLinkProgram(programID);

    glUseProgram(programID);

    //uint32_t vertShaderID = create_shader(gpuNode, vertexString, 0, 1);
    //uint32_t fragShaderID = create_shader(gpuNode, fragmentString, 1, 1);
    //uint32_t geomShaderID = create_shader(gpuNode, geometryShader, 2, 1);

    uint32_t resourceUniform = create_simple_buffer(gpuNode, sizeof(constants), PIPE_BIND_CONSTANT_BUFFER, NULL);

    send_inline_write(gpuNode, resourceUniform, 0, (1 << 1), (virtio_box_t){
        0, 0, 0,
        sizeof(constants), 1, 1
    }, &constants, sizeof(constants), 0, 0);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    command.command = VIRGL_CCMD_SET_UNIFORM_BUFFER;
    command.option = 0;
    command.length = 5;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 5);
    command.parameters[0] = 0;
    command.parameters[1] = 1; // Index
    command.parameters[2] = 0; // Offset
    command.parameters[3] = sizeof(constants); // Length
    command.parameters[4] = resourceUniform;

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    //---------------------

    uint32_t samplerID = create_texture2D(gpuNode, 16, 16, VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM, NULL);

    send_inline_write(gpuNode, samplerID, 0, (1 << 1), (virtio_box_t){
        0, 0, 0,
        16, 16, 1
    }, &tex2d, 16*4, 0, VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 5);
    parameters[0] = samplerID;
    parameters[1] = (PIPE_TEXTURE_2D << 24) | VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;
    uint32_t samplerViewID = create_opengl_object(gpuNode, VIRGL_OBJECT_SAMPLER_VIEW, parameters, 5, 0);

    command.command = VIRGL_CCMD_SET_SAMPLER_VIEWS;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = 1;
    command.parameters[1] = 0; // Index
    command.parameters[2] = samplerViewID; // Offset

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    parameters = (uint32_t *)realloc(parameters, sizeof(uint32_t) * 8);
    parameters[0] = (2 << 11);
    parameters[1] = 0;
    parameters[2] = 0;
    parameters[3] = 0;
    parameters[4] = 0;
    parameters[5] = 0;
    parameters[6] = 0;
    parameters[7] = 0;
    uint32_t samplerStateID = create_opengl_object(gpuNode, VIRGL_OBJECT_SAMPLER_STATE, parameters, 8, 0);

    command.command = VIRGL_CCMD_BIND_SAMPLER_STATES;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)realloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = 1;
    command.parameters[1] = 0; // Index
    command.parameters[2] = samplerStateID; // Offset

    ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

#ifdef DONT_USE_EBO
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
#else
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, (sizeof(indices) / sizeof(uint32_t)) * 3, GL_UNSIGNED_INT, 0);
#endif

    windows_flush();

    //---------------------

    /*draw_vbo(gpuNode, 0, 12 * 3, 4);

    ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    //ksleep(10);

    transferAndFlushData.notTransfer = 1;
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);
    ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);*/

    size_t i = 0;
    float angle = 0;

    while(1) {
        if(i >= 200) continue;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        constants[46] -= 0.05f;

        constants[48] = cosine(angle);
        constants[50] = -sine(angle);
        constants[56] = sine(angle);
        constants[58] = cosine(angle);

        send_inline_write(gpuNode, resourceUniform, 0, (1 << 1), (virtio_box_t){
            0, 0, 0,
            sizeof(constants), 1, 1
        }, &constants, sizeof(constants), 0, 0);

#ifdef DONT_USE_EBO
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
#else
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glDrawElements(GL_TRIANGLES, (sizeof(indices) / sizeof(uint32_t)) * 3, GL_UNSIGNED_INT, 0);
#endif
        windows_flush();

        /*draw_vbo(gpuNode, 0, 12 * 3, 4);

        ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);*/
        i++;
        angle += 0.05f;
    }

    return 0;
}



typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

static inline bool _is_digit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

char* __int_str(uint64_t i, char b[], int base, bool plusSignIfNeeded, bool spaceSignIfNeeded,
                int paddingNo, bool justify, bool zeroPad) {
 
    char digit[32] = {0};
    memset(digit, 0, 32);
    strcpy(digit, "0123456789");
 
    if (base == 16) {
        strcat(digit, "ABCDEF");
    } else if (base == 17) {
        strcat(digit, "abcdef");
        base = 16;
    }
 
    char* p = b;
    if (i < 0) {
        *p++ = '-';
        i *= -1;
    } else if (plusSignIfNeeded) {
        *p++ = '+';
    } else if (!plusSignIfNeeded && spaceSignIfNeeded) {
        *p++ = ' ';
    }
 
    uint64_t shifter = i;
    do {
        ++p;
        shifter = shifter / base;
    } while (shifter);
 
    *p = '\0';
    do {
        *--p = digit[i % base];
        i = i / base;
    } while (i);
 
    int padding = paddingNo - (int)strlen(b);
    if (padding < 0) padding = 0;
 
    if (justify) {
        while (padding--) {
            if (zeroPad) {
                b[strlen(b)] = '0';
            } else {
                b[strlen(b)] = ' ';
            }
        }
 
    } else {
        char a[256] = {0};
        while (padding--) {
            if (zeroPad) {
                a[strlen(a)] = '0';
            } else {
                a[strlen(a)] = ' ';
            }
        }
        strcat(a, b);
        strcpy(b, a);
    }
 
    return b;
}

static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
    if (idx < maxlen) {
        ((char *)buffer)[idx] = character;
    }
}

void displayCharacter(char c, int* a, out_fct_type out, char* buffer, size_t maxlen) {
    out(c, buffer, *a, maxlen);
    *a += 1;
}
 
void displayString(char* c, int* a, out_fct_type out, char* buffer, size_t maxlen) {
    for (int i = 0; c[i]; ++i) {
        displayCharacter(c[i], a, out, buffer, maxlen);
    }
}

int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list list)
{
    int chars        = 0;
    char intStrBuffer[256] = {0};
 
    for (int i = 0; format[i]; ++i) {
 
        char specifier   = '\0';
        char length      = '\0';
 
        int  lengthSpec  = 0; 
        int  precSpec    = 0;
        bool leftJustify = false;
        bool zeroPad     = false;
        bool spaceNoSign = false;
        bool altForm     = false;
        bool plusSign    = false;
        int  expo        = 0;
 
        if (format[i] == '%') {
            ++i;
 
            bool extBreak = false;
            while (1) {
 
                switch (format[i]) {
                    case '-':
                        leftJustify = true;
                        ++i;
                        break;
 
                    case '+':
                        plusSign = true;
                        ++i;
                        break;
 
                    case '#':
                        altForm = true;
                        ++i;
                        break;
 
                    case ' ':
                        spaceNoSign = true;
                        ++i;
                        break;
 
                    case '0':
                        zeroPad = true;
                        ++i;
                        break;
 
                    default:
                        extBreak = true;
                        break;
                }
 
                if (extBreak) break;
            }
 
            while (_is_digit(format[i])) {
                lengthSpec *= 10;
                lengthSpec += format[i] - 48;
                ++i;
            }
 
            if (format[i] == '*') {
                lengthSpec = va_arg(list, int);
                ++i;
            }
 
            if (format[i] == '.') {
                ++i;
                while (_is_digit(format[i])) {
                    precSpec *= 10;
                    precSpec += format[i] - 48;
                    ++i;
                }
 
                if (format[i] == '*') {
                    precSpec = va_arg(list, int);
                    ++i;
                }
            } else {
                precSpec = 6;
            }
 
            if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
                   format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
                length = format[i];
                ++i;
                if (format[i] == 'h') {
                    length = 'H';
                } else if (format[i] == 'l') {
                    length = 'q';
                }
            }
            specifier = format[i];
 
            memset(intStrBuffer, 0, 256);
 
            int base = 10;
            if (specifier == 'o') {
                base = 8;
                specifier = 'u';
                if (altForm) {
                    displayString("0", &chars, out, buffer, maxlen);
                }
            }
            if (specifier == 'p') {
                base = 16;
                length = 'z';
                specifier = 'u';
            }
            switch (specifier) {
                case 'X':
                    base = 16;
                case 'x':
                    base = base == 10 ? 17 : base;
                    if (altForm) {
                        displayString("0x", &chars, out, buffer, maxlen);
                    }
 
                case 'u':
                {
                    switch (length) {
                        case 0:
                        {
                            unsigned int integer = va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'H':
                        {
                            unsigned char integer = (unsigned char) va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'h':
                        {
                            unsigned short int integer = va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'l':
                        {
                            unsigned long integer = va_arg(list, unsigned long);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'q':
                        {
                            unsigned long long integer = va_arg(list, unsigned long long);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'j':
                        {
                            uintmax_t integer = va_arg(list, uintmax_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'z':
                        {
                            size_t integer = va_arg(list, size_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 't':
                        {
                            ptrdiff_t integer = va_arg(list, ptrdiff_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
 
                case 'd':
                case 'i':
                {
                    switch (length) {
                    case 0:
                    {
                        int integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'H':
                    {
                        signed char integer = (signed char) va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'h':
                    {
                        short int integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'l':
                    {
                        long integer = va_arg(list, long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'q':
                    {
                        long long integer = va_arg(list, long long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'j':
                    {
                        intmax_t integer = va_arg(list, intmax_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'z':
                    {
                        size_t integer = va_arg(list, size_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 't':
                    {
                        ptrdiff_t integer = va_arg(list, ptrdiff_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }
 
                case 'c':
                {
                    displayCharacter(va_arg(list, int), &chars, out, buffer, maxlen);
 
                    break;
                }
 
                case 's':
                {
                    displayString(va_arg(list, char*), &chars, out, buffer, maxlen);
                    break;
                }
 
                case 'n':
                {
                    switch (length) {
                        case 'H':
                            *(va_arg(list, signed char*)) = chars;
                            break;
                        case 'h':
                            *(va_arg(list, short int*)) = chars;
                            break;
 
                        case 0: {
                            int* a = va_arg(list, int*);
                            *a = chars;
                            break;
                        }
 
                        case 'l':
                            *(va_arg(list, long*)) = chars;
                            break;
                        case 'q':
                            *(va_arg(list, long long*)) = chars;
                            break;
                        case 'j':
                            *(va_arg(list, intmax_t*)) = chars;
                            break;
                        case 'z':
                            *(va_arg(list, size_t*)) = chars;
                            break;
                        case 't':
                            *(va_arg(list, ptrdiff_t*)) = chars;
                            break;
                        default:
                            break;
                    }
                    break;
                }
 
                case 'a':
                case 'A':
                    //ACK! Hexadecimal floating points...
                    break;
 
                default:
                    break;
            }
 
            if (specifier == 'e') {
                displayString("e+", &chars, out, buffer, maxlen);
            } else if (specifier == 'E') {
                displayString("E+", &chars, out, buffer, maxlen);
            }
 
            if (specifier == 'e' || specifier == 'E') {
                __int_str(expo, intStrBuffer, 10, false, false, 2, false, true);
                displayString(intStrBuffer, &chars, out, buffer, maxlen);
            }
 
        } else {
            displayCharacter(format[i], &chars, out, buffer, maxlen);
        }
    }
 
    return chars;
}

int sprintf(char* buffer, const char* format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int printf(const char* format, ...)
{
    static char buffer[200];
    va_list va;
    va_start(va, format);
    const int ret = _vsnprintf(_out_buffer, buffer, (size_t)-1, format, va);
    buffer[ret] = '\0';
    va_end(va);
    debug_print(buffer);
    return ret;
}

#pragma once

// Same data as the Linux kernel uses, extracted from the PC VGA font.
uint8_t fontBitmap[] = {
	// 32 0x20 ' '
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 33 0x21 '!'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00111100,
	0b00111100,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 34 0x22 '"'
	0b00000000,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00100100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 35 0x23 '#'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01101100,
	0b01101100,
	0b11111110,
	0b01101100,
	0b01101100,
	0b01101100,
	0b11111110,
	0b01101100,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 36 0x24 '$'
	0b00011000,
	0b00011000,
	0b01111100,
	0b11000110,
	0b11000010,
	0b11000000,
	0b01111100,
	0b00000110,
	0b00000110,
	0b10000110,
	0b11000110,
	0b01111100,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,

	// 37 0x25 '%'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000010,
	0b11000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000110,
	0b10000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 38 0x26 '&'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01101100,
	0b01101100,
	0b00111000,
	0b01110110,
	0b11011100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 39 0x27 '''
	0b00000000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b01100000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 40 0x28 '('
	0b00000000,
	0b00000000,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 41 0x29 ')'
	0b00000000,
	0b00000000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 42 0x2a '*'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01100110,
	0b00111100,
	0b11111111,
	0b00111100,
	0b01100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 43 0x2b '+'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b01111110,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 44 0x2c ','
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 45 0x2d '-'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 46 0x2e '.'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 47 0x2f '/'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000000,
	0b10000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 48 0x30 '0'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 49 0x31 '1'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00111000,
	0b01111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 50 0x32 '2'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000000,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 51 0x33 '3'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b00000110,
	0b00000110,
	0b00111100,
	0b00000110,
	0b00000110,
	0b00000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 52 0x34 '4'
	0b00000000,
	0b00000000,
	0b00001100,
	0b00011100,
	0b00111100,
	0b01101100,
	0b11001100,
	0b11111110,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00011110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 53 0x35 '5'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11111100,
	0b00000110,
	0b00000110,
	0b00000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 54 0x36 '6'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01100000,
	0b11000000,
	0b11000000,
	0b11111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 55 0x37 '7'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000110,
	0b00000110,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 56 0x38 '8'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 57 0x39 '9'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00001100,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 58 0x3a ':'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 59 0x3b ';'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 60 0x3c '<'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 61 0x3d '='
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 62 0x3e '>'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01100000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 63 0x3f '?'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b00001100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 64 0x40 '@'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11011110,
	0b11011110,
	0b11011110,
	0b11011100,
	0b11000000,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 65 0x41 'A'
	0b00000000,
	0b00000000,
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11111110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 66 0x42 'B'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 67 0x43 'C'
	0b00000000,
	0b00000000,
	0b00111100,
	0b01100110,
	0b11000010,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000010,
	0b01100110,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 68 0x44 'D'
	0b00000000,
	0b00000000,
	0b11111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01101100,
	0b11111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 69 0x45 'E'
	0b00000000,
	0b00000000,
	0b11111110,
	0b01100110,
	0b01100010,
	0b01101000,
	0b01111000,
	0b01101000,
	0b01100000,
	0b01100010,
	0b01100110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 70 0x46 'F'
	0b00000000,
	0b00000000,
	0b11111110,
	0b01100110,
	0b01100010,
	0b01101000,
	0b01111000,
	0b01101000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 71 0x47 'G'
	0b00000000,
	0b00000000,
	0b00111100,
	0b01100110,
	0b11000010,
	0b11000000,
	0b11000000,
	0b11011110,
	0b11000110,
	0b11000110,
	0b01100110,
	0b00111010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 72 0x48 'H'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11111110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 73 0x49 'I'
	0b00000000,
	0b00000000,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 74 0x4a 'J'
	0b00000000,
	0b00000000,
	0b00011110,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 75 0x4b 'K'
	0b00000000,
	0b00000000,
	0b11100110,
	0b01100110,
	0b01100110,
	0b01101100,
	0b01111000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 76 0x4c 'L'
	0b00000000,
	0b00000000,
	0b11110000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100010,
	0b01100110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 77 0x4d 'M'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11101110,
	0b11111110,
	0b11111110,
	0b11010110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 78 0x4e 'N'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11100110,
	0b11110110,
	0b11111110,
	0b11011110,
	0b11001110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 79 0x4f 'O'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 80 0x50 'P'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 81 0x51 'Q'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11011110,
	0b01111100,
	0b00001100,
	0b00001110,
	0b00000000,
	0b00000000,

	// 82 0x52 'R'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 83 0x53 'S'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b01100000,
	0b00111000,
	0b00001100,
	0b00000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 84 0x54 'T'
	0b00000000,
	0b00000000,
	0b01111110,
	0b01111110,
	0b01011010,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 85 0x55 'U'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 86 0x56 'V'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00010000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 87 0x57 'W'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11111110,
	0b11101110,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 88 0x58 'X'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b01101100,
	0b01111100,
	0b00111000,
	0b00111000,
	0b01111100,
	0b01101100,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 89 0x59 'Y'
	0b00000000,
	0b00000000,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 90 0x5a 'Z'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000110,
	0b10000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000010,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 91 0x5b '['
	0b00000000,
	0b00000000,
	0b00111100,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 92 0x5c '\'
	0b00000000,
	0b00000000,
	0b00000000,
	0b10000000,
	0b11000000,
	0b11100000,
	0b01110000,
	0b00111000,
	0b00011100,
	0b00001110,
	0b00000110,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 93 0x5d ']'
	0b00000000,
	0b00000000,
	0b00111100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 94 0x5e '^'
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 95 0x5f '_'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111111,
	0b00000000,
	0b00000000,

	// 96 0x60 '`'
	0b00000000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 97 0x61 'a'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111000,
	0b00001100,
	0b01111100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 98 0x62 'b'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 99 0x63 'c'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 100 0x64 'd'
	0b00000000,
	0b00000000,
	0b00011100,
	0b00001100,
	0b00001100,
	0b00111100,
	0b01101100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 101 0x65 'e'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11111110,
	0b11000000,
	0b11000000,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 102 0x66 'f'
	0b00000000,
	0b00000000,
	0b00011100,
	0b00110110,
	0b00110010,
	0b00110000,
	0b01111000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 103 0x67 'g'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01110110,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111100,
	0b00001100,
	0b11001100,
	0b01111000,
	0b00000000,

	// 104 0x68 'h'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01101100,
	0b01110110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 105 0x69 'i'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 106 0x6a 'j'
	0b00000000,
	0b00000000,
	0b00000110,
	0b00000110,
	0b00000000,
	0b00001110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b01100110,
	0b01100110,
	0b00111100,
	0b00000000,

	// 107 0x6b 'k'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01100110,
	0b01101100,
	0b01111000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 108 0x6c 'l'
	0b00000000,
	0b00000000,
	0b00111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 109 0x6d 'm'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11101100,
	0b11111110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 110 0x6e 'n'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 111 0x6f 'o'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 112 0x70 'p'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,

	// 113 0x71 'q'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01110110,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111100,
	0b00001100,
	0b00001100,
	0b00011110,
	0b00000000,

	// 114 0x72 'r'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01110110,
	0b01100110,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 115 0x73 's'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b01100000,
	0b00111000,
	0b00001100,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 116 0x74 't'
	0b00000000,
	0b00000000,
	0b00010000,
	0b00110000,
	0b00110000,
	0b11111100,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110110,
	0b00011100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 117 0x75 'u'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 118 0x76 'v'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 119 0x77 'w'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11111110,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 120 0x78 'x'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00111000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 121 0x79 'y'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111110,
	0b00000110,
	0b00001100,
	0b11111000,
	0b00000000,

	// 122 0x7a 'z'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111110,
	0b11001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 123 0x7b '{'
	0b00000000,
	0b00000000,
	0b00001110,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01110000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00001110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 124 0x7c '|'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 125 0x7d '}'
	0b00000000,
	0b00000000,
	0b01110000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00001110,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 126 0x7e '~'
	0b00000000,
	0b01110110,
	0b11011100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 127 0x7f ''
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
};