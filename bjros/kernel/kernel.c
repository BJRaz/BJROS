// Brians own kernel main ... 
//
#include <kernel.h>
#include <multiboot.h>
#include <console.h>
#include <ps2.h>
#include <sched.h>
#include <process.h>
#include <ringbuf.h>


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
	if (ps2_output_buffer_status()) {
		uint8_t response = inb(PS2_DATA);
		kprintf("mouse... 0x%x\n", response);
	}
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
	if(_strcmp("test_malloc", buf) == 0)
	{	
		test_malloc();return;
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

int kmain(struct multiboot_info* multiboot_structure, void* magicvalue) 
{
	mb_info = multiboot_structure;
	mv = magicvalue;
	_clear();
	/* Initialize ring buffers */
	ringbuf_init(&kbd_input_rb);

	/* Initialize scheduler (creates idle process as PID 0) */
	sched_init();

	/* Create console process (PID 1) */
	console_set_callback(callback);
	struct process *console_proc = process_create(console_main);
	sched_add(console_proc);

	kprintln("BJROS microkernel started.");
	kprintln("Console process created (PID 1).");

	/* Scheduler takes over via timer ISR.
	 * Return to mainhalt: in multiboot.asm (hlt + jmp loop). */
	i_sti;
	return 0;	
}

