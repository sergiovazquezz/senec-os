.intel_syntax noprefix

.macro EXCEPTION_STUB vector, has_error_code
.align 16
.global vector_\vector\()_handler
vector_\vector\()_handler:
    .if !\has_error_code
    push 0             /* dummy error code */
    .endif
    push \vector
    jmp interrupt_stub
.endm

interrupt_stub:
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    // RSP now points to the base of cpu_status_t
    mov rdi, rsp
    cld
    call interrupt_dispatch

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    // Discard vector_number + error_code
    add rsp, 16

    iretq


EXCEPTION_STUB 0, 0  // #DE - Divide Error
EXCEPTION_STUB 1, 0  // #DB - Debug
EXCEPTION_STUB 2, 0  // NMI - Non-Maskable Interrupt
EXCEPTION_STUB 3, 0  // #BP - Breakpoint
EXCEPTION_STUB 4, 0  // #OF - Overflow
EXCEPTION_STUB 5, 0  // #BR - BOUND Range Exceeded
EXCEPTION_STUB 6, 0  // #UD - Invalid Opcode
EXCEPTION_STUB 7, 0  // #NM - Device Not Available
EXCEPTION_STUB 8, 1  // #DF - Double Fault
EXCEPTION_STUB 9, 0  // Coprocessor Segment Overrun
EXCEPTION_STUB 10, 1 // #TS - Invalid TSS
EXCEPTION_STUB 11, 1 // #NP - Segment Not Present
EXCEPTION_STUB 12, 1 // #SS - Stack-Segment Fault
EXCEPTION_STUB 13, 1 // #GP - General Protection Fault
EXCEPTION_STUB 14, 1 // #PF - Page Fault
EXCEPTION_STUB 15, 0 // Reserved
EXCEPTION_STUB 16, 0 // #MF - x87 Floating-Point Exception
EXCEPTION_STUB 17, 1 // #AC - Alignment Check
EXCEPTION_STUB 18, 0 // #MC - Machine Check
EXCEPTION_STUB 19, 0 // #XM - SIMD Floating-Point Exception
EXCEPTION_STUB 20, 0 // #VE - Virtualization Exception
EXCEPTION_STUB 21, 1 // #CP - Control Protection Exception
EXCEPTION_STUB 22, 0 // Reserved
EXCEPTION_STUB 23, 0 // Reserved
EXCEPTION_STUB 24, 0 // Reserved
EXCEPTION_STUB 25, 0 // Reserved
EXCEPTION_STUB 26, 0 // Reserved
EXCEPTION_STUB 27, 0 // Reserved
EXCEPTION_STUB 28, 0 // Reserved
EXCEPTION_STUB 29, 0 // Reserved
EXCEPTION_STUB 30, 0 // Reserved
EXCEPTION_STUB 31, 0 // Reserved


.section .rodata
.global interrupt_handlers
interrupt_handlers:
    .quad vector_0_handler
    .quad vector_1_handler
    .quad vector_2_handler
    .quad vector_3_handler
    .quad vector_4_handler
    .quad vector_5_handler
    .quad vector_6_handler
    .quad vector_7_handler
    .quad vector_8_handler
    .quad vector_9_handler
    .quad vector_10_handler
    .quad vector_11_handler
    .quad vector_12_handler
    .quad vector_13_handler
    .quad vector_14_handler
    .quad vector_15_handler
    .quad vector_16_handler
    .quad vector_17_handler
    .quad vector_18_handler
    .quad vector_19_handler
    .quad vector_20_handler
    .quad vector_21_handler
    .quad vector_22_handler
    .quad vector_23_handler
    .quad vector_24_handler
    .quad vector_25_handler
    .quad vector_26_handler
    .quad vector_27_handler
    .quad vector_28_handler
    .quad vector_29_handler
    .quad vector_30_handler
    .quad vector_31_handler
