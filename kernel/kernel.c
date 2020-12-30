// Brians own kernel main ... 
#include <stdio.h>
#include <stdint.h>
#include <multiboot.h>
#define i_pushall 	__asm__("pushal");
#define i_popall	__asm__("popal");
#define	i_return 	__asm__("leave;iret");
#define halt		__asm__("hlt");

#define IDT_SEGMENT	0x8
#define IDT_FILL	0x0
#define IDT_FLAGS	0x8e	// TODO: check this - aka 10001110b


extern uint32_t gdtr;
extern uint32_t idt;
extern uint32_t custom;		// references a ISR implemented in nasm/multiboot.asm
extern void keyboard;		// references a ISR implemented - 
extern void timer;		// references a ISR
extern char kbdchar;
extern char kbdarray[128];
extern char kbdarray_upper[128];	

extern char inb(char reg);	// I/O in with byte size
extern void outb(char reg, char byte);	// write byte to register

void prompt();

// need the attribute packed, otherwise the alignment of short limit is 4 bytes
// yielding a false value. When packed the alignment is 2 bytes  
struct __attribute__ ((__packed__)) gdtr_register 
{
	uint16_t limit;
	uint32_t baseaddress;
};

struct __attribute__ ((__packed__)) interrupt_gate_descriptor
{
	uint16_t offset_lo;
	uint16_t segment_selector;
	uint8_t fill;
	uint8_t flags;
	uint16_t offset_hi; 
};

// called when division by zero occurs
// type: exception, fault - thus stored eip 
// is pointing to faulting instruction
void isr_div_by_zero()
{
	//i_pushall;
	kprintf("DIV BY ZERO Exception - system halted...");
	__asm__("hlt");
	//i_popall;
	i_return;
}

// test ISR
// interrupt 20H
void isr()
{
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	//i_pushall;
	//__asm__("mov $2, %eax");
	//__asm__("mov %eax, %gs");	
	__asm__("pushl %eax");
	kprintf("ISR args %d, %d\n", 22, 24);
	kprintf("her: %d, %d, %x\n", 1, 2, 16);
	char scancode = inb(0x60);	
	kprintf("scan: %x\n", scancode);

	if(scancode == 0x2a)
		kprintf("Shift is pressed");

	// remember to send EOI to PIC if used as a hardware-interrupt routine.
	//i_popall ; TODO: somehow this doesn't work when run in emulator (sets ebp = 0x2)
	__asm__("popl %eax");
	outb(0x20, 0x20);				// send EOI to PIC
	//__asm__("hlt");
	i_return;
}

void set_isr_entry(struct interrupt_gate_descriptor *idt_entry, const uint32_t isr_address) 
{
	idt_entry->offset_lo = (uint16_t)isr_address & 0xFFFF;
	idt_entry->segment_selector = IDT_SEGMENT;
	idt_entry->fill = IDT_FILL;
	idt_entry->flags = IDT_FLAGS;
	idt_entry->offset_hi = (uint16_t)(isr_address >> 16) & 0xFFFF; 
}

void showidtinfo(const struct interrupt_gate_descriptor* idt_array) 
{ 
	kprintf("IDT entry address: 0x%x\n", &idt_array->offset_lo);
	kprintf("IDT offset_lo: 0x%x\n", idt_array->offset_lo);
	kprintf("IDT segment: 0x%x\n", idt_array->segment_selector);
	kprintf("IDT fill: 0x%x\n", idt_array->fill);
	kprintf("IDT flags: 0x%x\n", idt_array->flags);
	kprintf("IDT offset_hi: 0x%x\n", idt_array->offset_hi);
}

int kmain(const struct multiboot_info* multiboot_structure, void* magicvalue) {
//halt;
	char* tal = "842";
	
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	kprintf("**************\n");
	kprintf("multiboot info memlower: 0x%x, memupper: 0x%x\n", 
		multiboot_structure->mem_lower, 
		multiboot_structure->mem_upper);
	kprintf("multiboot magic header %x\n", magicvalue);

	int len = 0;

	char* text = "Welcome to Brians kernel...\n";
	len = kprint(text);
	// debug clutter
	/*kprintf("Kernel test output from kprintf '-190000000': %d\n", -190000000);
	kprintf(" -  string 'Brian': %s\n", "Brian");
	kprintf(" -  string 'Brian' and number '200': %s, %d\n", "Brian", 200);
	_utoa(4000000000, buffer);
	kprintf(" -  '4000000000': %s\n", buffer);

	kprintf("Unsigned converstion atou('4000000000')': %u\n", 4000000000);

	kprintln(buffer);*/
	
	// IDT stuff
	struct interrupt_gate_descriptor *idt_array = (struct interrupt_gate_descriptor*) &idt;
	set_isr_entry(idt_array, (uint32_t)&isr_div_by_zero);
	idt_array += 32;	
	set_isr_entry(idt_array, (uint32_t)&timer);
	idt_array += 1;
	set_isr_entry(idt_array, (uint32_t)&keyboard);
	idt_array+=32;
	
	showidtinfo(idt_array);	
	
	kprintf("Interrupt gate descriptor baseaddress: 0x%x, %d\n", &idt, &idt);
	
	kprintf("ISR address: 0x%x\n", &isr);
	kprintf("ISR address div by zero: 0x%x\n", &isr_div_by_zero);


	// gdt stuff:	
	struct gdtr_register *gdtreg = (struct gdtr_register*) &gdtr;
	
	kprintf("GDTR address: 0x%x\n", &gdtr);

	kprintf("GDTR limit value: 0x%x\n", gdtreg->limit);
			
	kprintf("GDTR baseaddress value: 0x%x\n", gdtreg->baseaddress);
	// gdt stuff end
	//
	for(int i=0;i<3;i++){
		_itoa(i, buffer);
		kprintf("Number: %s\n", buffer);
	}

	//int calculation = 10 / 0;
	
	prompt();

	return len;	
}


void prompt() {
	char buf[255];
	while(1) {
		_memset(buf, 0, 255);
		kprintf("> ");
		char idx = 0;
		char c = 0;
		do
		{ 
next:
			if(idx == 255)
				break;
			c = _getchar();
			buf[idx++] = c;
			if(c != 0) {
				switch(c) {
					case 0x08:
						_putchar(c);	
						goto next;		
				}
				_putchar(c);
			} 
		} while(c != '\n'); 
		kprintf("Buffer: %s", buf);
	}
}

char _getchar(void) {
	while(kbdchar==0); // TODO: busy wait - refactor! 	
	char result = kbdchar;
	kbdchar = 0;
	return result;
}
