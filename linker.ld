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

	/* Heap region for _malloc/_free (1 MB) */
	. = 0x300000;
	_heap_start = .;
	. = 0x400000;
	_heap_end = .;

	/* Process stack region (32 KB = 8 processes × 4 KB each) */
	_proc_stack_start = .;
	. = 0x408000;
	_proc_stack_end = .;
}
