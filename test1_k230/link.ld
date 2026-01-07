ENTRY(_start)

SECTIONS
{
    . = 0x80000000; /* RAM base */

    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .bss : { *(.bss*) *(COMMON) }
}

