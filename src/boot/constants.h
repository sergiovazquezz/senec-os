#ifndef BOOT_CONSTANTS_H
#define BOOT_CONSTANTS_H

#define KERNEL_VMA 0xFFFFFFFF80000000

#ifdef __ASSEMBLER__
#define V2P(a) ((a) - KERNEL_VMA)
#define P2V(a) ((a) + KERNEL_VMA)
#else

#include <stdint.h>

#define V2P(a)          ((uintptr_t)(a) - KERNEL_VMA)
#define P2V(a)          ((uintptr_t)(a) + KERNEL_VMA)

#define DIRECT_MAP_BASE 0xFFFF800000000000ULL
#define DIRECT_MAP_SIZE 0x0000400000000000ULL // 64 TB

#define V2P_LATE(va)    ((uintptr_t)(va) - DIRECT_MAP_BASE)
#define P2V_LATE(pa)    ((uintptr_t)(pa) + DIRECT_MAP_BASE)

#define KHEAP_BASE      0xFFFFC00000000000ULL
#define KHEAP_SIZE      0x0000000100000000ULL // 4 GB

#define MMIO_BASE       0xFFFFC00100000000ULL
#define MMIO_SIZE       0x0000000100000000ULL // 4 GB

#endif // __ASSEMBLER__
#endif // BOOT_CONSTANTS_H
