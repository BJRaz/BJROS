// Brians own kernel main ... 
#include <stdio.h>
#define i_pushall 	__asm__("pushal");
#define i_popall	__asm__("popal");
#define	i_return 	__asm__("leave;iret");

extern unsigned int gdtr;
extern unsigned int idt;

// need the attribute packed, otherwise the alignment of short limit is 4 bytes
// yielding a false value. When packed the alignment is 2 bytes  
struct __attribute__ ((__packed__)) gdtr_register 
{
	unsigned short limit;
	unsigned int baseaddress;
};

struct __attribute__ ((__packed__)) interrupt_gate_descriptor
{
	unsigned short offset_lo;
	unsigned short segment_selector;
	unsigned char fill;
	unsigned char flags;
	unsigned short offset_hi; 
};

void isr_div_by_zero()
{
	//i_pushall;
	//kprintf("DIV BY ZERO Exception");
	__asm__("hlt");
	//i_popall;
	i_return;
}
void isr(unsigned int* arg1, unsigned int *arg2)
{
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	i_pushall;
	__asm__("mov $2, %eax");
	__asm__("mov %eax, %gs");	
	kprintf("ISR args  %x, %x\n", *arg1, *arg2);
	i_popall;
	i_return;
}

void set_isr_entry(struct interrupt_gate_descriptor *idt_entry, unsigned int isr_address) 
{
	unsigned short lo_bytes = (unsigned short)isr_address;
	unsigned short hi_bytes = (unsigned short)(isr_address >> 16);
	idt_entry->offset_lo = lo_bytes;
	idt_entry->segment_selector = 0x8;
	idt_entry->flags = 0x8E;	// aka 10001110b
	idt_entry->offset_hi = hi_bytes; 
}

int kmain(void* multiboot_structure, void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	
	char* text = "Welcome to Brians kernel...\nSystem halted\n";
	int len = kprint(text);
	kprintf("Kernel test output from kprintf '-190000000': %d\n", -190000000);
	kprintf(" -  string 'Brian': %s\n", "Brian");
	kprintf(" -  string 'Brian' and number '200': %s, %d\n", "Brian", 200);
	_utoa(4000000000, buffer);
	kprintf(" -  '4000000000': %s\n", buffer);

	kprintf("Unsigned converstion atou('4000000000')': %u\n", 4000000000);

	kprintln(buffer);
// IDT stuff
	struct interrupt_gate_descriptor *idt_array = (struct interrupt_gate_descriptor*) &idt;
	set_isr_entry(idt_array, &isr_div_by_zero);
	idt_array+=32;	
	set_isr_entry(idt_array, &isr);
	
	kprintf("IDT baseaddress: 0x%x\n", &idt);
	kprintf("IDT entry address: 0x%x\n", &idt_array->offset_lo);
	kprintf("IDT offset_lo: 0x%x\n", idt_array->offset_lo);
	kprintf("IDT segment: 0x%x\n", idt_array->segment_selector);
	kprintf("IDT fill: 0x%x\n", idt_array->fill);
	kprintf("IDT flags: 0x%x\n", idt_array->flags);
	kprintf("IDT offset_hi: 0x%x\n", idt_array->offset_hi);

	kprintf("ISR address: 0x%x\n", &isr);


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
	int calculation = 10 / 0;

	return len;	
} 
