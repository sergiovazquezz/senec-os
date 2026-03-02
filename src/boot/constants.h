#define KERNEL_VMA 0xFFFFFFFF80000000

#ifdef __ASSEMBLER__
#define V2P(a) ((a) - KERNEL_VMA)
#define P2V(a) ((a) + KERNEL_VMA)
#else
#define V2P(a) ((uintptr_t)(a) - KERNEL_VMA)
#define P2V(a) ((uintptr_t)(a) + KERNEL_VMA)
#endif
