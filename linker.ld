/* custom linker script til ld.
 *	BJR 2020
 *
 * */

SECTIONS
{
	. = 0x100000;		/* start of extended memory */	
	.text : { *(.text) }	/* kernel code lives here */
	. = 0x110000; 	
	.isr : { *(.isr) }	/* interrupt service routines memory space */
	.data : { *(.data) }
	.bss : { *(.bss) }
	.rodata : { *(.rodata) }
	. = 0x200000;  
	.gdt : { *(.gdt) }
	. = 0X202000;
	.idt : { *(.idt) }
}
