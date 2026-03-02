; NO LONGER USED. REFER TO "boot.S" INSTEAD

bits 32

KERNEL_VMA          equ 0xFFFFFFFF80000000
PAGE_PRESENT        equ 0x1
PAGE_WRITABLE       equ 0x2
PAGE_FLAGS          equ (PAGE_PRESENT | PAGE_WRITABLE)

MULTIBOOT2_MAGIC    equ 0xE85250D6
MULTIBOOT2_ARCH     equ 0              ; i386 protected mode
MULTIBOOT2_LENGTH   equ (multiboot_header_end - multiboot_header_start)
MULTIBOOT2_CHECKSUM equ -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH + MULTIBOOT2_LENGTH)


section .multiboot
align 8
multiboot_header_start:
    dd MULTIBOOT2_MAGIC
    dd MULTIBOOT2_ARCH
    dd MULTIBOOT2_LENGTH
    dd MULTIBOOT2_CHECKSUM

    ; End tag
    dw 0                                    ; type = 0 (end tag)
    dw 0                                    ; flags
    dd 8                                    ; size
multiboot_header_end:


section .text.boot
global _start
extern kmain
_start:
    cli                                     ; Disable interrupts
    cld                                     ; Clear direction flag (undefined per Multiboot2 spec)

    ; eax = magic, ebx = multiboot info pointer
    mov ebp, eax                            ; Save magic number (eax get overwritten later)

    mov esp, stack_top - KERNEL_VMA         ; Set up temporary stack (physical address)

    call setup_page_tables                  ; Set up page tables

    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, (1 << 5)                        ; Set PAE bit
    mov cr4, eax

    ; Load PML4 address into CR3
    mov eax, pml4 - KERNEL_VMA
    mov cr3, eax

    ; Enable Long Mode and NX support via EFER MSR
    mov ecx, 0xC0000080                     ; EFER MSR
    rdmsr
    or eax, (1 << 8) | (1 << 11)            ; Set LME (Long Mode Enable) and NXE, unsure about (1 << 11)
    wrmsr

    ; Enable paging and protected mode
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)            ; Set PG and PE bits (not sure if PE bits are necessary)
    mov cr0, eax

    lgdt [gdt64_pointer_phys - KERNEL_VMA]

    ; Far jump to 64-bit code
    jmp 0x08:(long_mode_start - KERNEL_VMA)


; Clobbers: eax, ecx, edi
setup_page_tables:
    ; Clear page table memory
    mov edi, pml4 - KERNEL_VMA
    xor eax, eax
    mov ecx, 4096 * 4 / 4             ; 4 tables × 4096 bytes / 4 bytes
    rep stosd

    ; PML4[0] -> PDPT (identity mapping, removed after entering higher-half)
    mov eax, pdpt - KERNEL_VMA
    or eax, PAGE_FLAGS
    mov [pml4 - KERNEL_VMA], eax

    ; PML4[511] -> PDPT (for higher-half mapping)
    mov [pml4 - KERNEL_VMA + 511 * 8], eax

    ; PDPT[0] -> PD (for identity mapping)
    mov eax, pd - KERNEL_VMA
    or eax, PAGE_FLAGS
    mov [pdpt - KERNEL_VMA], eax

    ; PDPT[510] -> PD (higher-half)
    mov [pdpt - KERNEL_VMA + 510 * 8], eax

    ; PD[0] -> PT
    mov eax, pt - KERNEL_VMA
    or eax, PAGE_FLAGS
    mov [pd - KERNEL_VMA], eax

    ; Fill PT with 512 4KB page entries (maps 2MB total)
    mov edi, pt - KERNEL_VMA
    mov eax, PAGE_FLAGS                ; Start at physical address 0
    mov ecx, 512

.fill_pt:
    mov [edi], eax
    add eax, 0x1000                    ; Next 4KB page
    add edi, 8
    loop .fill_pt

    ; Unmap the stack guard page so stack overflow triggers a page fault.
    ; Compute the PT entry offset: (phys_addr >> 12) * 8
    mov eax, stack_guard - KERNEL_VMA  ; Physical address of guard page
    shr eax, 12                        ; Page index
    mov edi, pt - KERNEL_VMA           ; Base of PT
    mov dword [edi + eax * 8], 0       ; Clear lower 32 bits (present=0)
    mov dword [edi + eax * 8 + 4], 0   ; Clear upper 32 bits

    ret


bits 64
default abs
section .text
long_mode_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rax, .higher_half
    jmp rax

.higher_half:
    ; Now executing at higher-half virtual address (through PML4[511])

    ; Set up higher-half stack
    mov rsp, stack_top

    ; Reload GDT with virtual base address
    lgdt [gdt64_pointer_virt]

    ; Remove identity mapping — PML4[0] is no longer needed
    mov rax, pml4
    mov qword [rax], 0

    ; Flush TLB to invalidate stale identity-mapped entries
    mov rax, cr3
    mov cr3, rax

    ; Save grub info before BSS clear
    mov r12d, ebp                           ; Save magic
    mov r13d, ebx                           ; Save multiboot info pointer (physical)

    ; Clear BSS section
    extern __bss_start
    extern __bss_end
    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    shr rcx, 3                              ; Clear 8 bytes at a time
    xor rax, rax
    rep stosq

    mov edi, r12d                           ; Restore multiboot info for kmain

    ; Convert multiboot info pointer from physical to virtual
    mov rsi, r13
    mov rax, KERNEL_VMA
    add rsi, rax

    call kmain

; Halt if kmain returns
.halt:
    cli
    hlt
    jmp .halt


section .rodata
align 16
gdt64:
    ; Null descriptor
    dq 0

    ; Code segment: 64-bit, present, executable, readable
    ; Base=0, Limit=0, L=1 (long mode), D=0, P=1, DPL=0, S=1, Type=0xA
    dq 0x00AF9A000000FFFF

    ; Data segment: 64-bit, present, writable
    ; Base=0, Limit=0, P=1, DPL=0, S=1, Type=0x2
    dq 0x00CF92000000FFFF

gdt64_end:

gdt64_pointer_phys:
    dw gdt64_end - gdt64 - 1          ; Limit
    dq gdt64 - KERNEL_VMA             ; Base (physical address)

gdt64_pointer_virt:
    dw gdt64_end - gdt64 - 1          ; Limit
    dq gdt64                          ; Base (virtual address)


section .page_tables nobits alloc write
align 4096
pml4:
    resb 4096

pdpt:
    resb 4096

pd:
    resb 4096

pt:
    resb 4096


section .stack_guard nobits alloc
align 4096
stack_guard:
    resb 4096


section .stack nobits alloc write
align 16
stack_bottom:
    resb 16384
stack_top:
