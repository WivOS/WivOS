ENTRY(kentry)
SECTIONS
{
    . = 0xffffffff80000000 + 0x01000000;

    .stivale2hdr ALIGN(4K) :
    {
        KEEP(*(.stivale2hdr))
    }

    .text ALIGN(4K) :
    {
        *(.text*)
        *(.symbols*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata*)
    }

    .data ALIGN(4K) :
    {
        *(.data*)
        PROVIDE(kernel_symbols_start = .);
		PROVIDE(kernel_symbols_end = .);
    }

    .bss ALIGN(4K) :
    {
        *(.bss*)
    }
}