#pragma once

#define test_bit(bitmap, bitpos) ({ \
    int ret; \
    __asm__ __volatile__("bt %2, %1;" : "=@ccc"(ret), "+m"(*(bitmap)) : "r"(bitpos) : "memory"); \
    ret; \
})

#define set_bit(bitmap, bitpos) ({ \
    int ret; \
    __asm__ __volatile__("bts %2, %1;" : "=@ccc"(ret), "+m"(*(bitmap)) : "r"(bitpos) : "memory"); \
    ret; \
})

#define reset_bit(bitmap, bitpos) ({ \
    int ret; \
    __asm__ __volatile__("btr %2, %1;" : "=@ccc"(ret), "+m"(*(bitmap)) : "r"(bitpos) : "memory"); \
    ret; \
})

#define write_cr(reg, val) ({ \
    __asm__ __volatile__("movq %0, %%cr" reg ";" : : "r" (val)); \
})

#define read_cr(reg) ({ \
    size_t cr; \
    __asm__ __volatile__("movq %%cr" reg ", %0;": "=r" (cr)); \
    cr; \
})

#define rdmsr(msr) ({ \
    uint32_t edx, eax; \
    __asm__ __volatile__("rdmsr;" : "=a" (eax), "=d" (edx) : "c" (msr)); \
    ((uint64_t)edx << 32) | eax; \
})

#define wrmsr(msr, value) ({ \
    __asm__ __volatile__("wrmsr;" :: "a" ((uint32_t)value), "d" ((uint32_t)(value >> 32)), "c" (msr)); \
})

#define wrxcr(i, value) ({ \
    asm volatile ("xsetbv" :: "a" ((uint32_t)(value)), "d" ((uint32_t)((value) >> 32)), "c" (i)); \
})

#define cpuid(leaf, subleaf, eax, ebx, ecx, edx) ({ \
    uint32_t cpuid_max; \
    asm volatile ("cpuid" : "=a" (cpuid_max) : "a" ((leaf) & 0x80000000) : "rbx", "rcx", "rdx"); \
    if ((leaf) > cpuid_max) \
        asm volatile ("cpuid" : "=a" (*(eax)), "=b" (*(ebx)), "=c" (*(ecx)), "=d" (*(edx)) : "a" ((leaf)), "c" ((subleaf))); \
    ((leaf) <= cpuid_max); \
})

#define load_fs_base(base) wrmsr(0xc0000100, base)