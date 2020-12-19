ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .got BLOCK(4K) : ALIGN(4K)
    {
        _GLOBAL_OFFSET_TABLE_ = .;
        *(.got)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss)
        *(COMMON)
    }

    .eh_frame BLOCK(4K) : ALIGN(4K)
	{
		*(.eh_frame)
	}
}