#pragma once

#include "gl.h"

size_t fopen(char *filePath, size_t flags);
size_t fread(size_t fd, void *buffer, size_t count);
size_t ioctl(size_t fd, size_t request, void *argp);
void *malloc(size_t size);
void free(void *block);
void *calloc(size_t num, size_t nsize);
void *realloc(void *block, size_t size);
char *strdup(const char *src);
int memcmp(const void * ptr1, const void * ptr2, size_t num);
int strcmp(const char *dst, char *src);
int printf(const char * format, ...);
int sprintf(char* buffer, const char* format, ...);
int strncmp(const char *s1, const char *s2, int c);

typedef struct {
    uint32_t bufferObject;
    uint32_t bufferID;
    uint32_t bind;
} gl_buffer_object_t;

typedef struct {
    uint32_t format;
    GLint size;
    GLenum type;
    GLuint position;
} gl_attrib_object_t;

typedef struct {
    uint8_t isDirt;
    volatile uint32_t enabledVertexAttribArrays;
    GLuint bufferObject;
    gl_attrib_object_t attribs[32];
} gl_vertex_array_object_t;

typedef struct {
    GLenum type;
    GLchar *string;
    uint32_t shaderID;
} gl_shader_object_t;

typedef struct {
    gl_shader_object_t *vertexShader;
    gl_shader_object_t *fragmentShader;
    gl_shader_object_t *geometryShader;
} gl_program_object_t;

typedef struct { // TODO: When we have more than two opengl contexts we needs this

} gl_context_t;

//TODO: Refactor
#define GLSL_TOKEN_VERSION "#version"
#define GLSL_TOKEN_LAYOUT "layout"
#define GLSL_TOKEN_OUT "out"
#define GLSL_TOKEN_VOID "void"
