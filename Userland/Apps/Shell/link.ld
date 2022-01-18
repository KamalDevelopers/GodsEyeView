ENTRY(_entry)
SECTIONS
{
    . = 11M;
	.text : ALIGN(0x1000)
	{
		*(.text)
	}
	.rodat : ALIGN(0x1000)
	{
		*(.rodata)
	}
	.data : ALIGN(0x1000)
	{
		*(.data)
	}
	.bss : ALIGN(0x1000)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
}
