#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int64_t token;
    int64_t hash;
    char *name;
    int64_t class;
    int64_t type;
    int64_t value;
    int64_t location;
    int64_t vecSize;
    int64_t variableType;
    int64_t BClass;
    int64_t BType;
    int64_t BValue;
} identifier_t;

char *compile_glsl_vertex(const char *string, bool preserveLastUniformData, bool preserveLastSymbolsTable);
char *compile_glsl_fragment(const char *string, bool preserveLastUniformData, bool preserveLastSymbolsTable);

identifier_t *get_copy_of_last_execution_symbols();