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
#define halt		__asm__("hlt");

#define IDT_SEGMENT	0x8
#define IDT_FILL	0x0
#define IDT_FLAGS	0x8e	// TODO: check this - aka 10001110b

#define PIC1_DATA	0x21
#define PIC1_CMD	0x20
#define	PIC2_DATA	0xa1
#define PIC2_CMD	0xa0

#define PIC_EOI		0x20
#define KBD_ARRAY_SIZE	128

#define PS2_DATA	0x60		// i8042 ps/2 controller data port (r/w)
#define PS2_CMD		0x64		//  i8042 ps/2 status register (read), command register (write)

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
	extern uint32_t gdtr;
	extern uint32_t idt;
	extern uint32_t custom;		// references a ISR implemented in nasm/multiboot.asm
	extern uint32_t keyboard;	// references a ISR implemented - 
	extern uint32_t timer;		// references a ISR
	extern char kbdchar;
	extern char kbdarray[KBD_ARRAY_SIZE];
	extern char kbdarray_upper[KBD_ARRAY_SIZE];	
	
	extern uint8_t inb(uint8_t reg);		// get byte from memory register reg
	extern void outb(uint8_t reg, uint8_t byte);	// write byte to memory register reg
	
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

// called when division by zero occurs
// type: exception, fault - thus stored eip 
// is pointing to faulting instruction
void ISR_FUNC isr_div_by_zero()
{
	//i_pushall;
	kprintf("DIV BY ZERO Exception - system halted...");
	__asm__("hlt");
	//i_popall;
	i_return;
}

// test ISR
// interrupt 20H
void ISR_FUNC isr()
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
	outb(PIC1_CMD, PIC_EOI);				// send EOI to PIC 1
	//__asm__("hlt");
	i_return;
}

// mouse interrupt
// type: exception, fault - thus stored eip 
// is pointing to faulting instruction
void ISR_FUNC isr_mouse()
{
	uint8_t response = inb(PS2_DATA);
	kprintf("mouse... 0x%x\n", response);
	outb(PIC1_CMD, PIC_EOI);
	outb(PIC2_CMD, PIC_EOI);
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
	kprintf("IDT current entry address: 0x%x\n", &idt_array->offset_lo);
	kprintf("IDT offset_lo: 0x%x\n", idt_array->offset_lo);
	kprintf("IDT segment: 0x%x\n", idt_array->segment_selector);
	kprintf("IDT fill: 0x%x\n", idt_array->fill);
	kprintf("IDT flags: 0x%x\n", idt_array->flags);
	kprintf("IDT offset_hi: 0x%x\n", idt_array->offset_hi);
}

void ps2_controller_send_command(uint8_t command)
{
	while(inb(PS2_CMD) & 0x2);	// 
	outb(PS2_CMD, command); 
}

void ps2_controller_write_data(uint8_t data) 
{
	while(inb(PS2_CMD) & 0x2);		//	 
	outb(PS2_DATA, data); 
}

uint8_t ps2_controller_read_data()
{
	while(!inb(PS2_CMD) & 0x1);	//
	return inb(PS2_DATA);
}

void setupps2()
{
	uint8_t ps2_response = NULL;

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
	ps2_response &= 0b1011100;		// clear bits 0,2 and 6

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
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 first port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0xA8);
	kprintf("Command sendt (2 port enable)\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 second port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0x20);
	kprintf("Command sendt\n");
	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 configuration byte: 0x%x\n", ps2_response);
	ps2_response |= 0b0100011;		// enable bits 0,2 and 6

	ps2_controller_send_command(0x60);
	ps2_controller_write_data(ps2_response);

//	ps2_response = ps2_controller_read_data();
//	kprintf("Ps2 changed configuration byte: 0x%x\n", ps2_response);
	
	// STEP 10: reset devices
	
	ps2_controller_send_command(0xD1);
	ps2_controller_write_data(0xFF);

	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 first port reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xfa) ? "OK" : "NOT OK");
	
	/*ps2_controller_send_command(0xD4);
	ps2_controller_write_data(0xFF);

	ps2_response = ps2_controller_read_data();
	kprintf("Ps2 second port reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xfa) ? "OK" : "NOT OK");
	*/

	return;	
		

	// step 1: Initialize all USB devices, and disable USB legacy support
	//
	// step 2: check for existence of i8042 controller
	// Note: should be done via ACPI "IA PC Boot architecture" flags 
	// offset 109 in FADT (	Fixed ACPI description table )
	// At the moment, if its not present the system may fail
	// step 3: disable devices
/*	while((inb(PS2_CMD) & 0x2) != 0);	// check input buffer full ?
	outb(PS2_CMD, 0xad);			// write cmd AD: disable PS2 first port (keyboard)
	kprintf("Ps2 first port disabled\n");
	while((inb(PS2_CMD) & 0x2) != 0);	// check input buffer full ?
	outb(PS2_CMD, 0xa7);			// write cmd A7: disable PS2 second port (mouse etc.)
	kprintf("Ps2 second port disabled\n");
	// step 4: flush output buffer
	ps2_response = inb(PS2_DATA);		// just read the data and ingore it...
	// step 5: set the controller configuration byte
	while((inb(PS2_CMD) & 0x2) != 0);	// check input buffer full ?
	outb(PS2_CMD, 0x20);			// write cmd 20: get configuration byte
	while((inb(PS2_CMD) & 0x1) == 0x0);	//
	ps2_response = inb(PS2_DATA);		// read from controller data port
	kprintf("Ps2 configuration byte: 0x%x\n", ps2_response);
	ps2_response &= 0b1011100;		// clear bits 0,2 and 6

	kprintf("Ps2 changed configuration byte: 0x%x\n", ps2_response);
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0x60);			// write cmd 60: set configuration byte
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_DATA, ps2_response);		// write configuration byte
	
	kprintf("Step 6\n");
	return;
	// step 6: perform controller self-test (look code above)
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xaa);			// write cmd AA: test PS2 controller
						// TODO: do some timeout checks
	ps2_response = inb(PS2_DATA);	// read from controller data port
	kprintf("Ps2 test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x55) ? "OK" : "NOT OK");
	// step 7: determine that there are 2 channels (optional)
	// step 8: perform interface tests
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xab);			// write cmd AB: test PS2 first port (keyboard)
	ps2_response = inb(PS2_DATA);		// read from controller data port
	kprintf("Ps2 first port test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xa9);			// write cmd A9: test PS2 second port (mouse etc.)
	ps2_response = inb(PS2_DATA);		// read from controller data port
	kprintf("Ps2 second port test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	// step 9: enable devices
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xae);			// write cmd AE: enable PS2 first port (keyboard)
	ps2_response = inb(PS2_DATA);		// read from controller data port
	kprintf("Ps2 first port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xa8);			// write cmd A8: enable PS2 second port (mouse etc.)
	ps2_response = inb(PS2_DATA);		// read from controller data port
	kprintf("Ps2 second port enable status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0x20);			// write cmd 20: get configuration byte
	ps2_response = inb(PS2_DATA);		// read from controller data port
	ps2_response &= 0b0100011;		// enable bits 0,2 and 6

	kprintf("Ps2 configuration byte: 0x%x\n", ps2_response);
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0x60);			// write back conf.byte
	// step 10: reset devices
	
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_CMD, 0xd1);			// write cmd D2: next byte writes to first ps2 device
	while((inb(PS2_CMD) & 0x2) == 0x1);	// check input buffer full ?
	outb(PS2_DATA, 0xff);			// write cmd ff: reset ps2 device
	while((inb(PS2_CMD) & 0x1) == 0x0);	// check output buffer full ?
	ps2_response = inb(PS2_DATA);		// read from controller data port

	kprintf("Ps2 first port reset status: 0x%x - %s\n", ps2_response, (ps2_response == 0xfa) ? "OK" : "NOT OK");
*/
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



	// debug clutter
/*	kprintf("Kernel test output from kprintf '-190000000': %d\n", -190000000);
	kprintf(" -  string 'Brian': %s\n", "Brian");
	kprintf(" -  string 'Brian' and number '200': %s, %d\n", "Brian", 200);
	_utoa(4000000000, buffer);
	kprintf(" -  '4000000000': %s\n", buffer);

	kprintf("Unsigned converstion atou('4000000000')': %u\n", 4000000000);

	kprintln(buffer);
*/

#ifdef __cplusplus
	Sysinfo s;	// = new Sysinfo();
	kprintf("Sysinfo obj: %d\n", s.getTest());
#endif
	kprintf("PIC1: 0x%x\n", inb(PIC1_DATA));
	kprintf("PIC2: 0x%x\n", inb(PIC2_DATA));

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

	return len;

}

void test() 
{
	for(int i=0;i<32;i++){
		kprintf("Number: %d\n", i);
	}

	int calculation = 10 / 0;
}

void callback(char* buf) 
{
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
		(*readbuf)(buf);	// call the callback function
	}
}

char _getchar(void) {
	while(kbdchar==0); // TODO: busy wait - refactor! 	
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

		// IDT stuff
		idt_array = (struct interrupt_gate_descriptor*) &idt;
		set_isr_entry(idt_array, (uint32_t)&isr_div_by_zero);
		idt_array += 32;	
		set_isr_entry(idt_array + 0, (uint32_t)&timer); 		// slot 32 (0) - system timer
		//idt_array += 1;
		set_isr_entry(idt_array + 1, (uint32_t)&keyboard);		// slot 33 (1) - keyboard PS/2
		//idt_array += 7;
		set_isr_entry(idt_array + 12, (uint32_t)&isr_mouse);		// slot 45 (12) - mouse PS/2
	
		setupps2();
		
		prompt(callback);
	
		return 0;	
	}
#ifdef __cplusplus
}
#endif


