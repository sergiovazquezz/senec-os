/* 
 * Maps kernel to virtual address 0xFFFFFFFF80000000 (last 2GB)
 * Physical load address at 1MB (0x100000) 
 */

ENTRY(_start)

KERNEL_LMA = 0x100000;
KERNEL_VMA = 0xFFFFFFFF80000000;

SECTIONS
{
    . = KERNEL_LMA;

    .multiboot ALIGN(8) : AT(ADDR(.multiboot))
    {
        *(.multiboot)
    }

    /* 32-bit bootstrap code */
    .text.boot ALIGN(4K) : AT(ADDR(.text.boot))
    {
        *(.text.boot)
    }

    . += KERNEL_VMA;

    /* 64-bit code section */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text .text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata .rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data .data.*)
    }

    .page_tables ALIGN(4K) : AT(ADDR(.page_tables) - KERNEL_VMA)
    {
        *(.page_tables)
    }

    .stack_guard ALIGN(4K) : AT(ADDR(.stack_guard) - KERNEL_VMA)
    {
        __stack_guard = .;
        *(.stack_guard)
    }

    .stack ALIGN(4K) : AT(ADDR(.stack) - KERNEL_VMA)
    {
        *(.stack)
    }

    /* Cleared at boot */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    __kernel_end = __bss_end - KERNEL_VMA;

    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
    }
}
