// Brians own kernel main ... 
#include <stdio.h>
#include <stdint.h>

extern uint32_t gdtr;
extern uint32_t idt;
extern uint32_t custom;		// references a ISR implemented in nasm/multiboot.asm
extern void keyboard;		// references a ISR implemented - 
extern void timer;		// references a ISR

void prompt();

// need the attribute packed, otherwise the alignment of short limit is 4 bytes
// yielding a false value. When packed the alignment is 2 bytes  
struct gdtr_register 
{
	uint16_t limit;
	uint32_t baseaddress;
};

struct interrupt_gate_descriptor
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
	//i_popall;
	return;
}

// test ISR
// interrupt 20H
void isr(uint32_t* arg1, uint32_t *arg2)
{
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	//i_pushall;
	kprintf("ISR args  %x, %x\n", *arg1, *arg2); 
	kprintf("ISR args %d, %d\n", 22, 24);
	kprintf("her: %d, %d, %x\n", 1, 2, 16);
	// remember to send EOI to PIC if used as a hardware-interrupt routine.
	//i_popall ; TODO: somehow this doesn't work when run in emulator (sets ebp = 0x2)
	return;
}

void set_isr_entry(struct interrupt_gate_descriptor *idt_entry, const uint32_t isr_address) 
{
	idt_entry->offset_lo = (uint16_t)isr_address & 0xFFFF;
	idt_entry->segment_selector = 0x8;
	idt_entry->fill = 0;
	idt_entry->flags = 0x8E;	// aka 10001110b
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

int kmain(const void* multiboot_structure, const void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	kprintf("**************\n");
	/*kprintf("multiboot info memlower: 0x%x, memupper: 0x%x\n", 
		multiboot_structure->mem_lower, 
		multiboot_structure->mem_upper);*/
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
	/*idt_array+=8;
	set_isr_entry(idt_array, (uint32_t)&timer);
	idt_array+=1;
	set_isr_entry(idt_array, (uint32_t)&keyboard);
	idt_array+=32;	
	set_isr_entry(idt_array, (uint32_t)&isr);
	*/
	showidtinfo(idt_array);	
	
	kprintf("Interrupt gate descriptor baseaddress: 0x%x\n", &idt);
	
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
	kprintf("> ");
}


