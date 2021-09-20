// Brians own kernel main ... 
//
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <multiboot.h>
#define i_pushall 	__asm__("pushal");
#define i_popall	__asm__("popal");
#define	i_return 	__asm__("leave;iret");
#define i_cli		__asm__("cli");
#define i_sti		__asm__("sti");
#define halt		__asm__("hlt");

#define IDT_SEGMENT	0x8
#define IDT_FILL	0x0
#define IDT_FLAGS	0x8e			// TODO: check this - aka 10001110b

#define PIC1_DATA	0x21
#define PIC1_CMD	0x20
#define	PIC2_DATA	0xa1
#define PIC2_CMD	0xa0

#define PIC_EOI		0x20			// End Of Interrupt value sent to PIC
#define KBD_ARRAY_SIZE	128

#define PS2_DATA	0x60			// i8042 ps/2 controller data port (r/w)
#define PS2_CMD		0x64			// i8042 ps/2 status register (read), command register (write)

#define ISR_FUNC	__attribute__((__section__(".isr")))
#define PACKED		__attribute__ ((__packed__)) 

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
	extern char kbdchar;
	extern char kbdarray[KBD_ARRAY_SIZE];
	extern char kbdarray_upper[KBD_ARRAY_SIZE];	
	
	extern uint8_t inb(uint8_t reg);		// get byte from memory register reg
	extern void outb(uint8_t reg, uint8_t byte);	// write byte to memory register reg
	extern void interrupt();
	
	void prompt(void (*)(char*));
	void callback(char*);
	void test();
#ifdef __cplusplus
}
#endif

struct interrupt_gate_descriptor *idt_array;
struct multiboot_info* mb_info;
void* mv;

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

void ps2_controller_send_command(uint8_t command)
{
	while(inb(PS2_CMD) & 0x2);	// 
	outb(PS2_CMD, command); 
}

void ps2_controller_write_data(uint8_t data) 
{
	while(inb(PS2_CMD) & 0x2);	//	 
	outb(PS2_DATA, data); 
}

uint8_t ps2_controller_read_data()
{
	while(!(inb(PS2_CMD) & 0x1));	//
	return inb(PS2_DATA);
}

void setup_ps2()
{
	uint8_t ps2_response = 0;

	// step 1: Initialize all USB devices, and disable USB legacy support
	//
	// step 2: check for existence of i8042 controller
	// Note: should be done via ACPI "IA PC Boot architecture" flags 
	// offset 109 in FADT (	Fixed ACPI description table )
	// At the moment, if its not present the system may fail

	// STEP 3-4: disable devices and read result
	// send RESET to kbd device
	ps2_controller_write_data(0xFF);
	kprintf("Data written\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 present status: 0x%x\n", ps2_response);

	// send RESET to mouse device
	ps2_controller_send_command(0xD4);
	ps2_controller_write_data(0xFF);
	kprintf("Data written\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 present status: 0x%x\n", ps2_response);

	// disable first PS2 port (kbd)
	ps2_controller_send_command(0xAD);
	kprintf("Command sendt\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 present status: 0x%x\n", ps2_response);

	// disable second PS2 port (mouse)
	ps2_controller_send_command(0xA7);
	kprintf("Command sendt\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 present status: 0x%x\n", ps2_response);
	
	// STEP 5: Get and set configuration byte 
	ps2_controller_send_command(0x20);
	kprintf("Command sendt\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 present status: 0x%x\n", ps2_response);
	
	kprintf("Ps2 configuration byte: 0x%x\n", ps2_response);
	ps2_response &= 0b1011100;		// clear bits 0,1 and 6

	kprintf("Ps2 changed configuration byte: 0x%x\n", ps2_response);
	ps2_controller_send_command(0x60);
	ps2_controller_write_data(ps2_response);
	
	// STEP 6: Do self test - should result in 0x55 
	ps2_controller_send_command(0xAA);
	kprintf("Command sendt (self test)\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 self test status: 0x%x\n", ps2_response);

	// STEP 7: (optional)
	// STEP 8: first and second port status test 
	ps2_controller_send_command(0xAB);
	kprintf("Command sendt (1 port status test)\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 first port test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0xA9);
	kprintf("Command sendt (2 port status test)\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 second port test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");

	// STEP 9: enable ports 1 and 2
	ps2_controller_send_command(0xAE);
	kprintf("Command sendt (1 port enable)\n");
	//ps2_response = ps2_controller_read_data();
	//kprintf("Ps2 first port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0xA8);
	kprintf("Command sendt (2 port enable)\n");
	//ps2_response = ps2_controller_read_data();
	//kprintf("Ps2 second port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0x20);
	kprintf("Command sendt\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 configuration byte: 0x%x\n", ps2_response);
	ps2_response |= 0b0100011;		// enable bits 0,1 and 6

	ps2_controller_send_command(0x60);
	ps2_controller_write_data(ps2_response);

	//ps2_response = ps2_controller_read_data();
	kprintf("Ps2 changed configuration byte: 0x%x\n", ps2_response);
	
	// STEP 10: reset devices
	
	ps2_controller_send_command(0xD1);
	ps2_controller_write_data(0xFF);	// KDB RESET AND START SELF TEST -  responds: AA = OK, FE = Resend, FC/FD = Self test failed 

	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 keyboard reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xaa) ? "Self test OK" : "NOT OK");

	ps2_controller_send_command(0xD4);	// Tell controller to access the mouse
	ps2_controller_write_data(0xFF);	// MOUSE RESET AND START SELF TEST

	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 mouse reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xfa) ? "OK" : "NOT OK");

	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 mouse reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xaa) ? "Self test OK" : "NOT OK");
	
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 mouse reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0x0) ? "OK" : "NOT OK");
/*	
	ps2_controller_send_command(0xD4);	// get mouse info
	ps2_controller_write_data(0xE9);
	ps2_response = ps2_controller_read_data();
	
	kprintf("PS2 mouse info: 0x%x\n", ps2_response);
*/
	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(0xF3);	// set sample rate command
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 command status: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");

	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(100);		// set sample rate to 100
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 command status: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");


	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(0xF4);	// enable data reporting - mouse wont work until this is set
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 command status: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");
/*
 * This should disable keyboard, and return the keyboards IDENTIFICATION byte(s)
 * does not work as of sept. 20 2021.
 * TODO: make it work 
 *
 *
	ps2_controller_send_command(0xF5);
	ps2_response = ps2_controller_read_data();
	if(ps2_response == 0xFA) 
	{
		ps2_controller_send_command(0xF2);	// identify command
		if((ps2_response = ps2_controller_read_data()) == 0xFA) {
			ps2_response = ps2_controller_read_data();
			kprintf("IDENT: 0x%x\n", ps2_response);
		} else
			kprintf("Returned: 0x%x\n", ps2_response);

	} else
			kprintf("Returned: 0x%x\n", ps2_response);
*/
	return;	
		

}


// test ISR
// interrupt 2dH
void ISR_FUNC isr(uint32_t arg)
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
	i_cli;
	uint32_t cs = arg;
	uint32_t eip = *(&arg-1);
	uint32_t eflags = *(&arg+1);
	kprintf("Arg: CS, EIP and EFLAGS: 0x%x, 0x%x, 0x%x\n", cs, eip, eflags);
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	outb(PIC2_CMD, PIC_EOI);				// send EOI to PIC 1
	i_sti;
	i_return;
}


// called when division by zero occurs
// type: exception, fault - thus stored eip 
// is pointing to faulting instruction
void ISR_FUNC isr_div_by_zero()
{
	//i_pushall;
	kprintf("DIV BY ZERO Exception - system halted...");
	halt;
	//i_popall;
	i_return;
}

// mouse interrupt
void ISR_FUNC isr_mouse()
{
	i_cli;
	kprintf("Reads data\n");
	uint8_t response = ps2_controller_read_data(); 
	kprintf("mouse... 0x%x\n", response);
	outb(PIC1_CMD, PIC_EOI);
	outb(PIC2_CMD, PIC_EOI);
	i_sti;
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

void setup_interrupts() 
{
	// IDT stuff
		
	idt_array = (struct interrupt_gate_descriptor*) &idt;
	set_isr_entry(idt_array, (uint32_t)&isr_div_by_zero);
	
	idt_array += 32;	
	
	set_isr_entry(idt_array, (uint32_t)&isr_timer); 		// slot 32 (0) - system timer
	set_isr_entry(idt_array + 1, (uint32_t)&isr_keyboard);		// slot 33 (1) - keyboard PS/2
	//set_isr_entry(idt_array + 8, (uint32_t)&timer);		// RTC (8) Real time clock
	set_isr_entry(idt_array + 12, (uint32_t)&isr_mouse);		// slot (12) - mouse PS/2
	set_isr_entry(idt_array + 13, (uint32_t)&isr);		// slot (12) - mouse PS/2
	
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
int sysinfo() 
{

	char* tal = "842";
	int len = 0;
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	kprintf("****** BJROS ******\n");
	char* text = "Welcome to BJROS ...\n";
	len = kprint(text);
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
	
	//kprintf("ISR address: 0x%x\n", &isr);
	kprintf("ISR address div by zero: 0x%x\n", &isr_div_by_zero);


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
	for(int i=0;i<32;i++){
		kprintf("Number: %d\n", i);
	}

	int calculation = 10 / 0;
	kprintf("Calc: %d\n", calculation);
}

void callback(char* buf) 
{
	if(_strcmp("int", buf) == 0)
		interrupt();
	if(_strcmp("test", buf) == 0)
		test();
	if(_strcmp("sysinfo", buf) == 0)
		sysinfo();
	if(_strcmp("clear", buf) == 0)
		_clear();
	else
		kprintf("Buffer: %s\n", buf);
}

void prompt(void (*readbuf)(char*)) {
	char buf[255];
	while(1) {
		_memset(buf, 0, 255);
		kprintf("> ");
		uint8_t idx = 0;
		char c = 0;
		do
		{ 
			if(idx == 255)
				break;
			c = _getchar();
			if(c != 0) {
				switch(c) {
					case 0x08:	//backspace char
						_putchar(c);	
						break;
					case 0x0a:	// linefeed
						_putchar(c);
						break;
					default:
						_putchar(c);
						buf[idx++] = c;
				}
			} 
		} while(c != '\n'); 
		(*readbuf)(buf);		// call the callback function
	}
}

char _getchar(void) {
	while(kbdchar==0); 			// TODO: busy wait - refactor! 	
	char result = kbdchar;
	kbdchar = 0;
	return result;
}

#ifdef __cplusplus
extern "C" {
#endif
	int kmain(struct multiboot_info* multiboot_structure, void* magicvalue) {
		mb_info = multiboot_structure;
		mv = magicvalue;
		
		prompt(callback);
		
		return 0;	
	}
#ifdef __cplusplus
}
#endif


