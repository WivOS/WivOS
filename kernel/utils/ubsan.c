#include "common.h"
#include "ctype.h"
#include <stdarg.h>

#include "spinlock.h"

typedef struct {
    uint64_t addr;
    uint64_t size;
    char name[];
} kernel_symbol_t;

extern char kernel_symbols_start[];
extern char kernel_symbols_end[];

const char *sym_resolve(size_t addr, size_t *rela) {
    kernel_symbol_t *symbol = (kernel_symbol_t *)&kernel_symbols_start;

    while((uint64_t)symbol < (uint64_t)&kernel_symbols_end) {
        uintptr_t start = symbol->addr;
        uintptr_t end = start + symbol->size;
        if((addr < start) || (addr >= end)) {
            uintptr_t new_address = (uintptr_t)symbol + sizeof(kernel_symbol_t) + strlen(symbol->name) + 1;
            symbol = (kernel_symbol_t *)ROUND_UP(new_address, sizeof(uintptr_t));

            continue;
        }

        if(rela) *rela = addr - start;
        return symbol->name;
    }

    if(rela) *rela = addr;
    return "(null)";
}

// high |     arg 8     |
//      |     arg 7     |
//      |  return addr  | <- Just after jump RSP
//      |    old RBP    | <- RBP
//      |  local var x  |
//  low |  local var y  | <- RSP
int temp_unwind_from(uintptr_t *addrs, int max, uint64_t rbp) {
    int i = 0;

    rbp = ROUND_DOWN(rbp, sizeof(uintptr_t));
    for(; (i < max) && (rbp != 0); i++) {
        uint64_t *frames = (uint64_t *)rbp;
        addrs[i] = frames[1];
        rbp = frames[0];
        if(0 == addrs[i]) break;
    }

    return i;
}

int temp_unwind(uintptr_t *addrs, int max) {
    uint64_t rbp;
    asm volatile("movq %%rbp, %0;" : "=r"(rbp) :: "memory", "cc");
    return temp_unwind_from(addrs, max, rbp);
}

typedef struct source_location {
    const char *file;
    uint32_t line;
    uint32_t column;
} source_location_t;

typedef struct type_descriptor {
    uint16_t kind;
    uint16_t info;
    char name[];
} type_descriptor_t;

typedef struct type_mismatch_data {
    source_location_t location;
    type_descriptor_t *type;
    uintptr_t alignment;
    uint8_t type_check_kind;
} type_mismatch_data_t;

typedef struct type_mismatch_data_v1 {
    source_location_t location;
    type_descriptor_t *type;
    uint8_t log_alignment;
    uint8_t type_check_kind;
} type_mismatch_data_v1_t;

typedef struct out_of_bounds_data {
    source_location_t location;
    type_descriptor_t *arr_type;
    type_descriptor_t *idx_type;
} out_of_bounds_data_t;

typedef struct shift_data {
    source_location_t location;
    type_descriptor_t *lhs_type;
    type_descriptor_t *rhs_type;
} shift_data_t;

typedef struct invalid_value_data {
    source_location_t location;
    type_descriptor_t *type;
} invalid_value_data_t;

static void log_location(source_location_t *loc) {
    printf("location: %s:%d,%d\n", loc->file, loc->line, loc->column);
}

static void print_frames(const uintptr_t *frames, int num) {
    for(int i = 0; i < num; i++) {
        size_t rela;
        const char *name = sym_resolve(frames[i], &rela);
        printf(" -> frame %2d: %s + 0x%llx\n", i, name, rela);
    }
}

static void log_stacktrace() {
    uintptr_t frames[32];
    int depth = temp_unwind(frames, 32);
    print_frames(&frames[1], depth - 1);
    while(1);
}

const char *TYPE_CHECK_KINFS[] = {
    "load of",
    "store to",
    "reference binding to",
    "member access within",
    "member call on",
    "constructor call on",
    "downcast of",
    "downcast of",
    "upcast of",
    "cast to virtual base of",
};

static void type_mismatch_common(source_location_t *loc, type_descriptor_t *type, uintptr_t alignment, uint8_t kind, uintptr_t ptr) {
    if(ptr == 0) {
        printf("\nubsan: null pointer access\n");
    } else if(alignment && (ptr & (alignment - 1))) {
        printf("\nubsan: unaligned memory access, alignment=%lx, ptr=%lx\n", alignment, ptr);
    } else {
        printf("\nubsan: %s addr %lx with insufficient space for object of type %s\n", TYPE_CHECK_KINFS[kind], ptr, type->name);
    }
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_type_mismatch(type_mismatch_data_t *data, uintptr_t ptr) {
    type_mismatch_common(&data->location, data->type, data->alignment, data->type_check_kind, ptr);
}

void __ubsan_handle_type_mismatch_v1(type_mismatch_data_v1_t *data, uintptr_t ptr) {
    type_mismatch_common(&data->location, data->type, 1UL << data->log_alignment, data->type_check_kind, ptr);
}

void __ubsan_handle_add_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: addition overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_sub_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: subtraction overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_mul_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: multiplication overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_negate_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: negation overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_divrem_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: division remainder overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_pointer_overflow(source_location_t *loc, uintptr_t a, uintptr_t b) {
    (void)a; (void)b;
    printf("\nubsan: pointer overflow\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_out_of_bounds(out_of_bounds_data_t *data, uintptr_t idx) {
    printf("\nubsan: index %ld out of bound\n", idx);
    log_location(&data->location);
    log_stacktrace();
}

void __ubsan_handle_shift_out_of_bounds(shift_data_t *data, uintptr_t lhs, uintptr_t rhs) {
    (void)lhs; (void)rhs;
    printf("\nubsan: shift out of bound\n");
    log_location(&data->location);
    log_stacktrace();
}

void __ubsan_handle_function_type_mismatch(source_location_t *loc) {
    printf("\nubsan: function type mismatch\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_invalid_builtin(source_location_t *loc) {
    printf("\nubsan: invalid builtin\n");
    log_location(loc);
    log_stacktrace();
}

void __ubsan_handle_load_invalid_value(invalid_value_data_t *data, uintptr_t val) {
    (void)val;
    printf("\nubsan: load invalid value\n");
    log_location(&data->location);
    log_stacktrace();
}