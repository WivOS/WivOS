#pragma once

#include <stdint.h>
#include <stddef.h>

#define LIMINE_COMMON_MAGIC 0xc7b1dd30df4c8b88, 0x0a82e883a194f07b
#define LIMINE_BOOTLOADER_INFO_REQUEST { LIMINE_COMMON_MAGIC, 0xf55038d8e2a1202f, 0x279426fcf5f59740 }
#define LIMINE_STACK_SIZE_REQUEST { LIMINE_COMMON_MAGIC, 0x224ef0460a8e8926, 0xe1cb0fc25f46ea3d }
#define LIMINE_MEMMAP_REQUEST { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 }
#define LIMINE_KERNEL_ADDRESS_REQUEST { LIMINE_COMMON_MAGIC, 0x71ba76863cc55f63, 0xb2644a48c516a487 }
#define LIMINE_RSDP_REQUEST { LIMINE_COMMON_MAGIC, 0xc5e77b6b397e7b43, 0x27637845accdcf3c }


typedef struct limine_response_header {
    uint64_t revision;
} limine_response_header_t;

typedef struct limine_request_header {
    uint64_t id[4];
    uint64_t revision;
    limine_response_header_t *response;
} limine_request_header_t;


typedef struct limine_stack_size_request {
    limine_request_header_t header;
    uint64_t stack_size;
} limine_stack_size_request_t;

typedef struct limine_stack_size_response {
    limine_response_header_t header;
} limine_stack_size_response_t;


#define LIMINE_MEMMAP_USABLE                 0
#define LIMINE_MEMMAP_RESERVED               1
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE       2
#define LIMINE_MEMMAP_ACPI_NVS               3
#define LIMINE_MEMMAP_BAD_MEMORY             4
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 5
#define LIMINE_MEMMAP_KERNEL_AND_MODULES     6
#define LIMINE_MEMMAP_FRAMEBUFFER            7

typedef struct limine_memmap_entry {
    uint64_t base;
    uint64_t length;
    uint64_t type;
} limine_memmap_entry_t;

typedef struct limine_memmap_request {
    limine_request_header_t header;
} limine_memmap_request_t;

typedef struct limine_memmap_response {
    limine_response_header_t header;
    uint64_t entry_count;
    limine_memmap_entry_t **entries;
} limine_memmap_response_t;


typedef struct limine_kernel_address_request {
    limine_request_header_t header;
} limine_kernel_address_request_t;

typedef struct limine_kernel_address_response {
    limine_response_header_t header;
    uint64_t physical_base;
    uint64_t virtual_base;
} limine_kernel_address_response_t;


typedef struct limine_rsdp_request  {
    limine_request_header_t header;
} limine_rsdp_request_t;

typedef struct limine_rsdp_response {
    limine_response_header_t header;
    void *address;
} limine_rsdp_response_t;