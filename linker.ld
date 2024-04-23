OUTPUT_FORMAT(binary)

SECTIONS
{
  /* Start offset */
  . = 0xC0000500;

	.text : 
	{
		*(.text)
	}
 
	/* Read-only data. */
	.rodata BLOCK(0x1000) : ALIGN(0x1000)
	{
		*(.rodata)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(0x1000) : ALIGN(0x1000)
	{
		*(.data)
	}
 
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(0x1000) : ALIGN(0x1000)
	{
		*(COMMON)
		*(.bss)
	  endkernel = .;
	}
}
