/* test af et custom linker script til ld.*/

SECTIONS
{
	. = 0x100000;
	.text : { *(.text) }
	/*. = 0x110000; */
	.data : { *(.data) }
	.bss : { *(.bss) }  
}
