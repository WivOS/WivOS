#pragma once

#include "gl.h"

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
    uint32_t enabledVertexAttribArrays;
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

#define GLSL_TOKEN_VERSION "#version"
#define GLSL_TOKEN_LAYOUT "layout"
#define GLSL_TOKEN_OUT "out"
#define GLSL_TOKEN_VOID "void"
