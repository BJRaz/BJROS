// Brians own kernel main ... 
#include <stdio.h>
#define	i_return __asm__("leave;iret");

extern unsigned int gdt;
extern unsigned int idt_start;

// need the attribute packed, otherwise the alignment of short limit is 4 bytes
// yielding a false value. When packed the alignment is 2 bytes  
struct __attribute__ ((__packed__)) gdtr 
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

void isr()
{
	__asm__("mov $2, %eax");
	__asm__("mov %eax, %gs");	

	i_return;
}

void set_isr_entry(struct interrupt_gate_descriptor *idt_entry, unsigned int isr_address) 
{
	unsigned short lo_bytes = (unsigned short)isr_address;
	unsigned short hi_bytes = (unsigned short)(isr_address >> 16);
	idt_entry->offset_lo = lo_bytes;
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
	struct interrupt_gate_descriptor *idt_array = (struct interrupt_gate_descriptor*) &idt_start+32;
	
	set_isr_entry(idt_array, &isr);
	
	kprintf("IDT address: %d\n", &idt_array->offset_lo);
	kprintf("IDT offset_lo: %d\n", idt_array->offset_lo);
	kprintf("IDT segment: %d\n", idt_array->segment_selector);
	kprintf("IDT fill: %d\n", idt_array->fill);
	kprintf("IDT offset_hi: %d\n", idt_array->offset_hi);

	kprintf("ISR address: %d\n", &isr);


// gdt stuff:	
	struct gdtr *gdtreg = (struct gdtr*) &gdt;
	
	kprintf("GDT address: %d\n", &gdt);

	kprintf("GDTR limit value: %d\n", gdtreg->limit);
			
	kprintf("GDTR baseaddress value: %d\n", gdtreg->baseaddress);
// gdt stuff end
//
	for(int i=0;i<3;i++){
		_itoa(i, buffer);
		kprintf("Number: %s\n", buffer);
	}
	//int calculation = 10 / 0;

	return len;	
} 
