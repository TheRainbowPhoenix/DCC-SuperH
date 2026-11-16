OUTPUT_ARCH(sh)
ENTRY(_start)
SECTIONS
{
    . = IMAGE_BASE;
    __image_start = .;
    .text : { KEEP(*(.text.start)) *(.text*) }
    .rodata ALIGN(4) : { *(.rodata*) }
    .data ALIGN(4) : { *(.data*) }
    .bss ALIGN(4) (NOLOAD) : { *(.bss*) *(COMMON) }
    __image_end = .;
    .stack ALIGN(16) (NOLOAD) : { . += 65536; }
    __stack_top = .;
    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
