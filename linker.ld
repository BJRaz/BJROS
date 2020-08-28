/* custom linker script til ld.
 *	BJR 2020
 *
 * */

SECTIONS
{
	. = 0x100000;
	.text : { *(.text) }
	. = 0x110000; /**/
	.data : { *(.data) }
	.bss : { *(.bss) }
	.rodata : { *(.rodata) }
	. = 0x200000;  
	.gdt : { *(.gdt) }
	. = 0x202000;
	.idt : { *(.idt) }
}
