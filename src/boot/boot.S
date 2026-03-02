#include "./constants.h"

.intel_syntax noprefix

.set PAGE_PRESENT,          0x1
.set PAGE_WRITABLE,         0x2
.set PAGE_FLAGS,            (PAGE_PRESENT | PAGE_WRITABLE)

.set MULTIBOOT2_MAGIC,      0xE85250D6
.set MULTIBOOT2_ARCH,       0
.set MULTIBOOT2_LENGTH,     (multiboot_header_end - multiboot_header_start)
.set MULTIBOOT2_CHECKSUM,   -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_LENGTH)


.section .multiboot
.align 8
multiboot_header_start:
    .long MULTIBOOT2_MAGIC
    .long MULTIBOOT2_ARCH
    .long MULTIBOOT2_LENGTH
    .long MULTIBOOT2_CHECKSUM

    /* End tag */
    .word 0         /* type = 0 */
    .word 0         /* flags   */
    .long 8         /* size    */
multiboot_header_end:


.code32
.section .text.boot
.global _start
.extern kmain
_start:
    cli                                     // Disable interrupts
    cld                                     // Clear direction flag

    // eax = magic, ebx = multiboot info pointer
    mov ebp, eax                            // Save magic (eax overwritten later)

    mov esp, OFFSET V2P(stack_top)          // Temporary physical stack

    call setup_page_tables

    // Enable PAE
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    // Load PML4 into CR3
    mov eax, OFFSET V2P(pml4)
    mov cr3, eax

    // Enable Long Mode + NXE via EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8) | (1 << 11)
    wrmsr

    // Enable paging + protected mode
    mov eax, cr0
    or  eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    lgdt [V2P(gdt64_pointer_phys)]

    // Far jump to 64-bit code segment
    ljmp 0x08, V2P(long_mode_start)


// Clobbers: eax, ecx, edi
setup_page_tables:
    // Zero all four page tables
    mov edi, OFFSET V2P(pml4)
    xor eax, eax
    mov ecx, 4096
    rep stosd

    // PML4[0] -> PDPT (identity mapping)
    mov eax, OFFSET V2P(pdpt)
    or  eax, PAGE_FLAGS
    mov DWORD PTR [V2P(pml4)], eax

    // PML4[511] -> PDPT (higher-half)
    mov DWORD PTR [V2P(pml4) + 511 * 8], eax

    // PDPT[0] -> PD (identity)
    mov eax, OFFSET V2P(pd)
    or  eax, PAGE_FLAGS
    mov DWORD PTR [V2P(pdpt)], eax

    // PDPT[510] -> PD (higher-half)
    mov DWORD PTR [V2P(pdpt) + 510 * 8], eax

    // PD[0] -> PT
    mov eax, OFFSET V2P(pt)
    or  eax, PAGE_FLAGS
    mov DWORD PTR [V2P(pd)], eax

    // Fill PT: 512 entries × 4 KB = 2 MB identity mapped
    mov edi, OFFSET V2P(pt)
    mov eax, PAGE_FLAGS
    mov ecx, 512

.fill_pt:
    mov DWORD PTR [edi], eax
    add eax, 0x1000
    add edi, 8
    loop .fill_pt

    // Unmap stack guard page → page fault on overflow
    mov eax, OFFSET V2P(stack_guard)
    shr eax, 12                             // Page index
    mov edi, OFFSET V2P(pt)
    mov DWORD PTR [edi + eax * 8],     0
    mov DWORD PTR [edi + eax * 8 + 4], 0

    ret


.code64
.section .text
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rax, OFFSET higher_half
    jmp rax

higher_half:
    mov rsp, OFFSET stack_top

    lgdt [gdt64_pointer_virt]

    // Remove identity mapping
    mov rax, OFFSET pml4
    mov QWORD PTR [rax], 0

    // Flush TLB
    mov rax, cr3
    mov cr3, rax

    // Save multiboot info across BSS clear
    mov r12d, ebp                           // Magic
    mov r13d, ebx                           // Multiboot info pointer (physical)

    // Clear BSS
    .extern __bss_start
    .extern __bss_end
    mov rdi, OFFSET __bss_start
    mov rcx, OFFSET __bss_end
    sub rcx, rdi
    shr rcx, 3                              // 8 bytes at a time
    xor rax, rax
    rep stosq

    mov edi, r12d                           // arg0: magic

    // arg1: multiboot pointer → physical + KERNEL_VMA
    mov rsi, r13
    mov rax, KERNEL_VMA
    add rsi, rax

    call kmain

.halt:
    cli
    hlt
    jmp .halt


.section .rodata
.align 16
gdt64:
    // Null descriptor
    .quad 0
    // Code: G=1, L=1, P=1, DPL=0, S=1, (E=1, DC=0, RW=1, A=0)
    .quad (1 << 55) | (1 << 53) | (1 << 47) | (1 << 44) | (0xA << 40)
    // Data: G=1, L=0, P=1, DPL = 0, S=1, (E=0, DC=0, RW=1, A=0)
    .quad (1 << 55) | (1 << 47) | (1 << 44) | (0x2 << 40)
gdt64_end:

gdt64_pointer_phys:
    .word gdt64_end - gdt64 - 1    // Limit (size - 1)
    .quad V2P(gdt64)               // Base (physical)

gdt64_pointer_virt:
    .word gdt64_end - gdt64 - 1    // Limit (size - 1)
    .quad gdt64                    // Base (virtual)


.section .page_tables, "aw", @nobits
.align 4096
pml4:
    .space 4096
pdpt:
    .space 4096
pd:
    .space 4096
pt:
    .space 4096


.section .stack_guard, "a", @nobits
.align 4096
stack_guard:
    .space 4096


.section .stack, "aw", @nobits
.align 16
stack_bottom: 
    .space 16384
stack_top:
