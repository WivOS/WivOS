#include "math.h"

float cosine(float angle);
float sine(float angle);

void math_init_mat4(mat4_t *matrix4out) {
    matrix4out->m00 = 1; matrix4out->m01 = 0; matrix4out->m02 = 0; matrix4out->m03 = 0;
    matrix4out->m10 = 0; matrix4out->m11 = 1; matrix4out->m12 = 0; matrix4out->m13 = 0;
    matrix4out->m20 = 0; matrix4out->m21 = 0; matrix4out->m22 = 1; matrix4out->m23 = 0;
    matrix4out->m30 = 0; matrix4out->m31 = 0; matrix4out->m32 = 0; matrix4out->m33 = 1;
}

void math_apply_multiplication_mat4(mat4_t *matrix4out, mat4_t *matrix4in) {
    mat4_t tempMat = *matrix4out; // maybe we need to optimize this

    matrix4out->m00 = tempMat.m00 * matrix4in->m00 + tempMat.m01 * matrix4in->m10 + tempMat.m02 * matrix4in->m20 + tempMat.m03 * matrix4in->m30;
    matrix4out->m01 = tempMat.m00 * matrix4in->m01 + tempMat.m01 * matrix4in->m11 + tempMat.m02 * matrix4in->m21 + tempMat.m03 * matrix4in->m31;
    matrix4out->m02 = tempMat.m00 * matrix4in->m02 + tempMat.m01 * matrix4in->m12 + tempMat.m02 * matrix4in->m22 + tempMat.m03 * matrix4in->m32;
    matrix4out->m03 = tempMat.m00 * matrix4in->m03 + tempMat.m01 * matrix4in->m13 + tempMat.m02 * matrix4in->m23 + tempMat.m03 * matrix4in->m33;

    matrix4out->m10 = tempMat.m10 * matrix4in->m00 + tempMat.m11 * matrix4in->m10 + tempMat.m12 * matrix4in->m20 + tempMat.m13 * matrix4in->m30;
    matrix4out->m11 = tempMat.m10 * matrix4in->m01 + tempMat.m11 * matrix4in->m11 + tempMat.m12 * matrix4in->m21 + tempMat.m13 * matrix4in->m31;
    matrix4out->m12 = tempMat.m10 * matrix4in->m02 + tempMat.m11 * matrix4in->m12 + tempMat.m12 * matrix4in->m22 + tempMat.m13 * matrix4in->m32;
    matrix4out->m13 = tempMat.m10 * matrix4in->m03 + tempMat.m11 * matrix4in->m13 + tempMat.m12 * matrix4in->m23 + tempMat.m13 * matrix4in->m33;

    matrix4out->m20 = tempMat.m20 * matrix4in->m00 + tempMat.m21 * matrix4in->m10 + tempMat.m22 * matrix4in->m20 + tempMat.m23 * matrix4in->m30;
    matrix4out->m21 = tempMat.m20 * matrix4in->m01 + tempMat.m21 * matrix4in->m11 + tempMat.m22 * matrix4in->m21 + tempMat.m23 * matrix4in->m31;
    matrix4out->m22 = tempMat.m20 * matrix4in->m02 + tempMat.m21 * matrix4in->m12 + tempMat.m22 * matrix4in->m22 + tempMat.m23 * matrix4in->m32;
    matrix4out->m23 = tempMat.m20 * matrix4in->m03 + tempMat.m21 * matrix4in->m13 + tempMat.m22 * matrix4in->m23 + tempMat.m23 * matrix4in->m33;

    matrix4out->m30 = tempMat.m30 * matrix4in->m00 + tempMat.m31 * matrix4in->m10 + tempMat.m32 * matrix4in->m20 + tempMat.m33 * matrix4in->m30;
    matrix4out->m31 = tempMat.m30 * matrix4in->m01 + tempMat.m31 * matrix4in->m11 + tempMat.m32 * matrix4in->m21 + tempMat.m33 * matrix4in->m31;
    matrix4out->m32 = tempMat.m30 * matrix4in->m02 + tempMat.m31 * matrix4in->m12 + tempMat.m32 * matrix4in->m22 + tempMat.m33 * matrix4in->m32;
    matrix4out->m33 = tempMat.m30 * matrix4in->m03 + tempMat.m31 * matrix4in->m13 + tempMat.m32 * matrix4in->m23 + tempMat.m33 * matrix4in->m33;
}

void math_apply_rotation_mat4(mat4_t *matrix4, GLfloat angle, vec3_t axis) {
    mat4_t rotMat = {0};
    rotMat.m33 = 1;
    if(axis.x) { //well, we need to make this the correct way, for example we can have an unitary vector like this (0, 1, 1)
        rotMat.m00 = 1;
        rotMat.m11 = rotMat.m22 = cosine(angle);
        rotMat.m21 = sine(angle);
        rotMat.m12 = -rotMat.m21;
    } else if(axis.y) {
        rotMat.m00 = rotMat.m22 = cosine(angle);
        rotMat.m11 = 1;
        rotMat.m02 = sine(angle);
        rotMat.m20 = -rotMat.m02;
    } else if(axis.z) {
        rotMat.m00 = rotMat.m11 = cosine(angle);
        rotMat.m22 = 1;
        rotMat.m01 = rotMat.m10 = sine(angle);
    }

    math_apply_multiplication_mat4(matrix4, &rotMat);
}