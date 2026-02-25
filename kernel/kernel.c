// Brians own kernel main ... 
//
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <multiboot.h>
#include <console.h>
#include <kernel.h>
#include <ps2.h>

extern void interrupt();		// this function calls software interrupt
extern char* kbd_rb;
extern uint8_t kbd_rb_head, kbd_rb_tail;

	
void callback(char*);
void test();
void help();
void recursive(int i);

// Keyboard test ISR
// 
void ISR_FUNC isr_keyboard_handler(void *arg)
{
	struct isrstackframe *frame = (struct isrstackframe*)&arg-1;
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	char command = inb(PS2_CMD);	
	char scancode = inb(PS2_DATA);	
	// ring buffer test code...
	
	if(kbd_rb_head < 255)
		kbd_rb[kbd_rb_head++] = scancode;
	else
		interrupt();
//	kprintf("0x%x\n", command & 0b00000001);
	kprintf("0x%x, %x\n", scancode & 0x000000FF, &kbd_rb);
	kprintf("EIP: 0x%x, CS: 0x%x, FLAGS: 0x%x\n", frame->EIP, frame->CS, frame->EFLAGS);

	if(scancode == 0x2a)
		kprintf("Shift is pressed");
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	outb(PIC2_CMD, PIC_EOI);				// send EOI to PIC 2
	i_return;
}
// test ISR
// interrupt 2dH
void ISR_FUNC isr_handler(uint32_t arg)
{
	// TODO: store all relevant regs in stack
	// check stack segment etc.
	//i_pushall;
	//__asm__("mov $2, %eax");
	//__asm__("mov %eax, %gs");	
	//__asm__("pushl %eax");
	/*kprintf("ISR args %d, %d\n", 22, 24);
	kprintf("her: %d, %d, %x\n", 1, 2, 16);
	char scancode = inb(0x60);	
	kprintf("scan: %x\n", scancode);

	if(scancode == 0x2a)
		kprintf("Shift is pressed");

	// remember to send EOI to PIC if used as a hardware-interrupt routine.
	//i_popall ; TODO: somehow this doesn't work when run in emulator (sets ebp = 0x2)
	__asm__("popl %eax");
	*/
//	i_cli;
	uint32_t cs = arg;
	uint32_t eip = *(&arg-1);
	uint32_t eflags = *(&arg+1);
	kprintf("Arg: CS, EIP and EFLAGS: 0x%x, 0x%x, 0x%x\n", cs, eip, eflags);
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	outb(PIC2_CMD, PIC_EOI);				// send EOI to PIC 2
//	i_sti;
//	i_return;
}


// called when division by zero occurs
// type: exception, fault - thus stored eip 
// is pointing to faulting instruction
/*void ISR_FUNC isr_div_by_zero()
{
	//i_pushall;
	kprintf("DIV BY ZERO Exception - system halted...");
	halt;
	//i_popall;
	i_return;
}*/

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
}

void ISR_FUNC isr_general_protection_fault(void *arg) 
{
	kprintf("#GP(0) - GENERAL PROTECTION FAULT\n");
	uint32_t cs = (uint32_t)arg;
	uint32_t eip = (uint32_t)*(&arg+1);
	uint32_t eflags = (uint32_t)*(&arg+2);
	kprintf("Arg: CS, EIP and EFLAGS, args address: 0x%x, 0x%x, 0x%x, 0x%x\n", cs, eip, eflags, &arg);
	outb(PIC1_CMD, PIC_EOI);
	outb(PIC2_CMD, PIC_EOI);
	halt;
}

void set_isr_entry(struct interrupt_gate_descriptor *idt_entry, const uint32_t isr_address) 
{
	idt_entry->offset_lo = (uint16_t)isr_address & 0xFFFF;
	idt_entry->segment_selector = IDT_SEGMENT;
	idt_entry->fill = IDT_FILL;
	idt_entry->flags = IDT_FLAGS;
	idt_entry->offset_hi = (uint16_t)(isr_address >> 16) & 0xFFFF; 
}

void setup_interrupts() 
{
	// IDT stuff
	// first 32 entries is reserved for processor.
	idt_array = (struct interrupt_gate_descriptor*) &idt;
	set_isr_entry(idt_array, (uint32_t)&isr_division_by_zero);		// set division by zero interrupt service routine
	idt_array += 13;
	set_isr_entry(idt_array, (uint32_t)&isr_general_protection_fault);	
	//set_isr_entry(idt_array, (uint32_t)&interrupt);	
	idt_array += 19;							// set address past the first 32 entries which is reserved intel/cpu	
	
	// index 32-255 custom interrupt handlers starts here 	
	set_isr_entry(idt_array, (uint32_t)&isr_timer); 			// slot (0) - system timer
	set_isr_entry(idt_array + 1, (uint32_t)&isr_keyboard);		// slot (1) - keyboard PS/2
	//set_isr_entry(idt_array + 8, (uint32_t)&timer);			// slot (8) - Real time clock
	set_isr_entry(idt_array + 12, (uint32_t)&isr_mouse);			// slot (12) - mouse PS/2
	set_isr_entry(idt_array + 13, (uint32_t)&isr);				// slot (13) - custom ISR for software INT test
	
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

void showmbinfo() 
{
	kprintf("MB information flags: 0x%x\n", mb_info);
}

int sysinfo() 
{
	int len = 0;
	kprintf("****** BJROS v0.2 ******\n");
	char* text = "Welcome to BJROS ...\n";
	len = kprint(text);
	kprintf("multiboot info address: 0x%x\n", &mb_info); 
	kprintf("multiboot info cmdline: %s\n", 
		mb_info->cmdline);
	kprintf("multiboot info memlower: 0x%x, memupper: 0x%x\n", 
		mb_info->mem_lower, 
		mb_info->mem_upper);
	kprintf("multiboot magic header %x\n", mv);

#ifdef __cplusplus
	Sysinfo s;	// = new Sysinfo();
	kprintf("Sysinfo obj: %d\n", s.getTest());
#endif
	kprintf("PIC1: 0x%x\n", inb(PIC1_DATA));
	kprintf("PIC2: 0x%x\n", inb(PIC2_DATA));

	showidtinfo(idt_array);	
	
	kprintf("Interrupt gate descriptor baseaddress: 0x%x, %d\n", &idt, &idt);
	
	kprintf("ISR test (INT 45) address: 0x%x\n", &isr);
	kprintf("ISR address div by zero: 0x%x\n", &isr_division_by_zero);


	// gdt stuff:	
	struct gdtr_register *gdtreg = (struct gdtr_register*) &gdtr;
	
	kprintf("GDTR address: 0x%x\n", &gdtr);

	kprintf("GDTR limit value: 0x%x\n", gdtreg->limit);
			
	kprintf("GDTR baseaddress value: 0x%x\n", gdtreg->baseaddress);
	// gdt stuff end

	return len;

}

void test() 
{
	int number = 32;
	for(int i=0;i<number;i++){
		kprintf("Number: %d\n", i);
	}

	int calculation = 10 / 0;
	kprintf("Calc: %d\n", calculation);
}

void callback(char* buf) 
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


