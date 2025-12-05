#ifndef KERNEL_H
#define KERNEL_H

#include <standard/stdint.h>
#include <standard/stddef.h>
#include <standard/stdio.h>
#include <standard/string.h>

#define i_pushall 	__asm__("pushal");
#define i_popall	__asm__("popal");
#define	i_return 	__asm__("leave;iret");
#define i_cli		__asm__("cli");
#define i_sti		__asm__("sti");
#define halt		__asm__("hlt");

#define IDT_SEGMENT	0x8
#define IDT_FILL	0x0
#define IDT_FLAGS	0x8e			// TODO: check this - aka 10001110b
#define IDT_SIZE	0xFF			// size of interrupt descriptor table (256)

#define PIC1_DATA	0x21
#define PIC1_CMD	0x20
#define	PIC2_DATA	0xa1
#define PIC2_CMD	0xa0

#define PIC_EOI		0x20			// End Of Interrupt value sent to PIC


#define ISR_FUNC	__attribute__((__section__(".isr")))
#define PACKED		__attribute__ ((__packed__)) 
#define ISRSTACK(a)	(struct isrstackframe*)(a-1);
	

#ifdef __cplusplus 

class Sysinfo
{
public:
	Sysinfo() : test(10) { }
	unsigned int getTest() { return test; }
private:
	unsigned int test;
};

extern "C" {
#endif
	extern uint32_t gdtr;			// references the global descriptor table register
	extern uint32_t idt;			// references the interrupt descriptor table
	
	extern uint32_t isr_custom;		// references a ISR implemented in nasm/multiboot.asm
	extern uint32_t isr_keyboard;		// references a ISR implemented - 
	extern uint32_t isr_timer;		// references a ISR
	extern uint32_t isr_division_by_zero;	// division by zero interrupt handler
	extern uint32_t isr_mouse;		// mouse interrupt handler - wraps isr_mouse_handler
	extern uint32_t isr;	
	extern uint8_t inb(uint8_t reg);		// get byte from memory register reg
	extern void outb(uint8_t reg, uint8_t byte);	// write byte to memory register reg
	
#ifdef __cplusplus
}
#endif


struct isrstackframe {
	uint32_t EIP;
	uint32_t CS;
	uint32_t EFLAGS;
};

// need the attribute packed, otherwise the alignment of short limit is 4 bytes
// yielding a false value. When packed the alignment is 2 bytes  
struct PACKED gdtr_register 
{
	uint16_t limit;
	uint32_t baseaddress;
};

struct PACKED interrupt_gate_descriptor
{
	uint16_t offset_lo;
	uint16_t segment_selector;
	uint8_t fill;
	uint8_t flags;
	uint16_t offset_hi; 
};

void* mv;						// pointer to the magicvalue
struct interrupt_gate_descriptor* idt_array;
struct multiboot_info* mb_info;

#endif
