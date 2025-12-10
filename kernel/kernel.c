// Brians own kernel main ... 
//
#include <kernel.h>
#include <multiboot.h>
#include <console.h>
#include <ps2.h>

extern void interrupt();		// this function calls software interrupt
extern char* kbd_rb;			// keyboard ring buffer 
extern uint8_t kbd_rb_head, kbd_rb_tail;// keyboard ring buffer head, and tail addresses

	
void callback(const char*);
void test();
void help();
void recursive(int i);

// Keyboard test ISR
// 
void ISR_FUNC isr_keyboard_handler(const void *arg)
{
	struct isrstackframe *frame = ISRSTACK(&arg);
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	char command = inb(PS2_CMD);	
	char scancode = inb(PS2_DATA);	
	// ring buffer test code...
	
	if(kbd_rb_head < 10)
		kbd_rb[kbd_rb_head++] = scancode;
	else
		interrupt();
	
	kprintf("0x%x, 0x%x, %x\n", scancode, scancode & 0x000000FF, &kbd_rb);
	kprintf("EIP: 0x%x, CS: 0x%x, FLAGS: 0x%x\n", frame->EIP, frame->CS, frame->EFLAGS);

	if(scancode == 0x2a)
		kprintf("Shift is pressed");
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	outb(PIC2_CMD, PIC_EOI);				// send EOI to PIC 2
	i_return;
}
// test ISR
// interrupt 2dH
// Called from interrupt handler wrapper
void ISR_FUNC isr_handler(const uint32_t arg)
{
//	i_cli;
	struct isrstackframe *frame = ISRSTACK(&arg);
	kprintf("Arg: EIP, CS and EFLAGS: 0x%x, 0x%x, 0x%x, address cs: 0x%x\n", frame->EIP, frame->CS, frame->EFLAGS, &arg);
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	outb(PIC2_CMD, PIC_EOI);				// send EOI to PIC 2
//	i_sti;
	i_return;
}


// *******
// Called from isr_mouse interrupt handler 
// -  mouse interrupt
// *******
void ISR_FUNC isr_mouse_handler()
{
	kprintf("Reads mousedata\n");
	uint8_t response = ps2_controller_read_data(); 
	kprintf("mouse... 0x%x\n", response);
	outb(PIC1_CMD, PIC_EOI);
	outb(PIC2_CMD, PIC_EOI);
	i_return;
}

void ISR_FUNC isr_general_protection_fault(const void *arg) 
{

	struct isrstackframe *frame = (struct isrstackframe*)(&arg-1);
	kprintf("#GP(0) - GENERAL PROTECTION FAULT\n");
	uint32_t cs = (uint32_t)arg;
	uint32_t eip = (uint32_t)*(&arg+1);
	uint32_t eflags = (uint32_t)*(&arg+2);
	kprintf("Arg: CS, EIP and EFLAGS, args address: 0x%x, 0x%x, 0x%x, 0x%x\n", cs, eip, eflags, &arg);
	kprintf("Arg: EIP, CS and EFLAGS: 0x%x, 0x%x, 0x%x, address cs: 0x%x\n", frame->EIP, frame->CS, frame->EFLAGS, &arg);
	outb(PIC1_CMD, PIC_EOI);
	outb(PIC2_CMD, PIC_EOI);
	halt;
	i_return;
}

void set_isr_entry(const int vector, const void* routineaddress) 
{
	if(vector > IDT_SIZE - 1)
		interrupt();	
	if(routineaddress == NULL)
		interrupt();
	struct interrupt_gate_descriptor *idt_entry = &idt_array[vector];
	uint32_t isr_address 		= (uint32_t)routineaddress;
	idt_entry->offset_hi 		= (uint16_t)(isr_address >> 16) & 0xFFFF; 
	idt_entry->offset_lo 		= (uint16_t)isr_address & 0xFFFF;
	idt_entry->segment_selector 	= IDT_SEGMENT;
	idt_entry->fill 		= IDT_FILL;
	idt_entry->flags 		= IDT_FLAGS;
}

void setup_interrupts() 
{
	// Interrupt descriptor table (IDT) setup
	// vectors 0-31: entries are reserved for the processor.
	// NOTE: the idt variable is defined as global in the system
	// and the address of the variable is equal to the start address of
	// the .idt sector defined in the linker.ld script. 
	// The memory space is pre-initialized with 0-values
	
	idt_array = (struct interrupt_gate_descriptor*) &idt;
	set_isr_entry(0, &isr_division_by_zero);		// set division by zero exception (fault) service routine
	set_isr_entry(13, &isr_general_protection_fault);	// set general protection fault service routine
	// indexes 32-255: user defined custom interrupt handlers 	
	set_isr_entry(32, &isr_timer); 				// slot (0) - system timer
	set_isr_entry(33, &isr_keyboard);			// slot (1) - keyboard PS/2
	//set_isr_entry(40, &timer);				// slot (8) - Real time clock
	set_isr_entry(44, &isr_mouse_handler);			// slot (12) - mouse PS/2
	set_isr_entry(45, &isr_handler);			// slot (13) - custom ISR for software INT test
}

void showidtinfo(const struct interrupt_gate_descriptor* idt_array) 
{ 
	kprintf("IDT current entry address: 0x%x\n", &idt_array->offset_lo);
	kprintf("IDT offset_lo: 0x%x\n", idt_array->offset_lo);
	kprintf("IDT segment: 0x%x\n", idt_array->segment_selector);
	kprintf("IDT fill: 0x%x\n", idt_array->fill);
	kprintf("IDT flags: 0x%x\n", idt_array->flags);
	kprintf("IDT offset_hi: 0x%x\n", idt_array->offset_hi);
}

// Shows the multiboot info
void showmbinfo() 
{
	kprintf("MEMORY\n");
	kprintf("multiboot info address: 0x%x\n", mb_info); 
	kprintf("multiboot info cmdline: %s\n", 
		mb_info->cmdline);
	kprintf("multiboot info memlower (decimal): %dkb, memupper: %dkb\n", 
		mb_info->mem_lower, 
		mb_info->mem_upper);
	kprintf("multiboot magic header %x\n", mv);
}

int sysinfo() 
{
	int len = 0;
	char* text = "****** BJROS v0.2 ******\nWelcome to BJROS ...\n";
	len = kprint(text);
	kprintf("HER: %d\n", len);
	#ifdef __cplusplus
		Sysinfo s;	// = new Sysinfo();
		kprintf("Sysinfo obj: %d\n", s.getTest());
	#endif
	// IDT stuff
	kprintf("PIC1: 0x%x\n", inb(PIC1_DATA));
	kprintf("PIC2: 0x%x\n", inb(PIC2_DATA));

	showidtinfo(idt_array);	
	kprintf("Interrupt gate descriptor baseaddress: 0x%x, %d\n", &idt, &idt);
	kprintf("ISR test (INT 45) address: 0x%x\n", &isr);
	kprintf("ISR address div by zero: 0x%x\n", &isr_division_by_zero);
	// GDT stuff:	
	struct gdtr_register *gdtreg = (struct gdtr_register*) &gdtr;
	kprintf("GDTR address: 0x%x\n", &gdtr);
	kprintf("GDTR limit value: 0x%x\n", gdtreg->limit);
	kprintf("GDTR baseaddress value: 0x%x\n", gdtreg->baseaddress);
	// Memory stuff
	showmbinfo();
	return len;

}

void test2() 
{
	//char* str = malloc(sizeof(char*));
	//printf("Her: %s\n", str);
} 

void test() 
{
	int number = 32;
	for(int i=0;i<number;i++){
		kprintf("Number: %d", i);
	}

	int calculation = 10 / 0;
	kprintf("Calc: %d\n", calculation);
}

void callback(const char* buf) 
{
	if(_strcmp("multiboot", buf) == 0)
	{
		showmbinfo();return;
	}
	if(_strcmp("recursive", buf) == 0)
	{
		recursive(1);return;
	}
	if(_strcmp("int", buf) == 0)
	{
		interrupt();
		return;
	}
	if(_strcmp("test", buf) == 0)
	{	
		test();return;
	}
	if(_strcmp("sysinfo", buf) == 0)
	{	
		sysinfo();return;
	}
	if(_strcmp("clear", buf) == 0)
	{
		_clear();return;
	}
	if(_strcmp("help", buf) == 0)
	{	
		help();return;
	}
	if(_strlen(buf) > 0)
		kprintf("Command not found: %s\n", buf);
}

void help() 
{
	kprintln("int - calls software interrupt");
	kprintln("multiboot - shows multiboot parameters");
	kprintln("test - test program");
	kprintln("sysinfo - show system info");
	kprintln("clear - clears screen");
	kprintln("help - this help..");
}

void recursive(int i) 
{
	kprintf("tal: %d\n", i++);
	recursive(i);
}

#ifdef __cplusplus
extern "C" {
#endif
	int kmain(struct multiboot_info* multiboot_structure, void* magicvalue) 
	{
		mb_info = multiboot_structure;
		mv = magicvalue;
		_clear();		
		prompt(callback);
		
		return 0;	
	}
#ifdef __cplusplus
}
#endif


