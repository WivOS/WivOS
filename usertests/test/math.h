#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include "gl.h"

typedef struct vec3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} __attribute__((packed)) vec3_t;

typedef struct vec4 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
} __attribute__((packed)) vec4_t;

typedef struct mat4 { // To pass to the uniforms we need the matrix transposed, so we define it transposed
    GLfloat m00, m10, m20, m30; // m00 m01 m02 m03
    GLfloat m01, m11, m21, m31; // m10 m11 m12 m13
    GLfloat m02, m12, m22, m32; // m20 m21 m22 m23
    GLfloat m03, m13, m23, m33; // m30 m31 m32 m33
} __attribute__((packed)) mat4_t;

typedef struct mat3 { // To pass to the uniforms we need the matrix transposed, so we define it transposed
    GLfloat m00, m10, m20; // m00 m01 m02
    GLfloat m01, m11, m21; // m10 m11 m12
    GLfloat m02, m12, m22; // m20 m21 m22
} __attribute__((packed)) mat3_t;

void math_init_mat4(mat4_t *matrix4out);

void math_apply_multiplication_mat4(mat4_t *matrix4out, mat4_t *matrix4in);

void math_apply_rotation_mat4(mat4_t *matrix4, GLfloat angle, vec3_t axis);