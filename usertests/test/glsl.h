#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

char *compile_glsl_vertex(const char *string);
char *compile_glsl_fragment(const char *string);