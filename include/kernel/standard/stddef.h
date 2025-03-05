#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

#define BUFFERLEN	255			// input buffer length

#define KBD_ARRAY_SIZE	128

extern char kbdchar;
extern char kbdarray[KBD_ARRAY_SIZE];
extern char kbdarray_upper[KBD_ARRAY_SIZE];	


void* mv;						// pointer to the magicvalue
struct interrupt_gate_descriptor *idt_array;
struct multiboot_info* mb_info;

#endif
