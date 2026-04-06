#include <console.h>
#include <multiboot.h>

static void (*cmd_callback)(const char*) = 0;

/* Called from keyboard ISR (assembly) to push a character into the ring buffer */
void kbd_input_ringbuf_put(uint8_t c)
{
	ringbuf_put(&kbd_input_rb, c);
}

void console_set_callback(void (*cb)(const char*))
{
	cmd_callback = cb;
}

/* Read one character from the keyboard ring buffer.
 * Yields CPU while waiting (cooperative with scheduler). */
static char console_getchar(void)
{
	uint8_t c;
	while (ringbuf_get(&kbd_input_rb, &c) != 0)
		_wait();	//halt 
	return (char)c;
}

/*
 *	TODO: check for buffer overflow
 * */
void prompt(void (*readbuf)(const char*)) {
	char buf[BUFFERLEN];
	char *pmt = "BJROS> ";		// TODO: replace with get_sysprompt() or similar
	int len = _strlen(pmt);
	while(1) {
		_memset(buf, 0, BUFFERLEN);
		
		kprintf(pmt);
		setcursor(vx, vy);
		uint8_t idx = 0;
		char c = 0;
		do
		{ 
			if(idx == BUFFERLEN)
				break;
			c = _getchar();
			if(c != 0) {
				switch(c) {
					case BSP:			// backspace char
						if(vx > len)		// TODO: refactor
						{
							_putchar(c);
							buf[--idx] = 0;	// remove char from buffer	
						}
						break;
					case LF:			// linefeed
						_putchar(c);
						break;
					default:			// prints the actual character to screen and puts it in buffer
						_putchar(c);
						buf[idx++] = c;
				}
			}
		       	if(vx >= len)					// TODO: refactor	
				setcursor(vx, vy);
		} while(c != '\n'); 
		(*readbuf)(buf);					// call the callback function
	}
}

/* Console as a schedulable process.
 * Reads from kbd_input_rb ring buffer instead of direct _getchar(). */
void console_main(void)
{
	char buf[BUFFERLEN];
	char *pmt = "BJROS> ";
	int len = _strlen(pmt);

	for (;;) {
		_memset(buf, 0, BUFFERLEN);
		kprintf(pmt);
		setcursor(vx, vy);

		uint8_t idx = 0;
		char c = 0;
		do {
			if (idx == BUFFERLEN)
				break;
			c = console_getchar();
			if (c != 0) {
				switch (c) {
				case 0x08:
					if (vx > len) {
						_putchar(c);
						buf[--idx] = 0;
					}
					break;
				case 0x0a:
					_putchar(c);
					break;
				default:
					_putchar(c);
					buf[idx++] = c;
				}
			}
			if (vx >= len)
				setcursor(vx, vy);
		} while (c != '\n');

		if (cmd_callback)
			cmd_callback(buf);
	}
}

/*
 *	This function runs a loop, and performs a division by zero
 * */
void test() 
{
	int number = 32;
	for(int i=0;i<number;i++){
		kprintf("Number: %d", i);
	}

	int calculation = 10 / 0;
	kprintf("Calc: %d\n", calculation);
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

void test_malloc() 
{
	char* str = _malloc(sizeof(char*));
	kprintf("malloc returned: 0x%x\n", str);
	if (str) {
		str[0] = 'H'; str[1] = 'i'; str[2] = '\0';
		kprintf("String: %s\n", str);
		_free(str);
		kprintf("free OK\n");
	}
} 


void help() 
{
	kprintln("int - calls software interrupt");
	kprintln("multiboot - shows multiboot parameters");
	kprintln("test - test program");
	kprintln("test_malloc - test heap allocator");
	kprintln("sysinfo - show system info");
	kprintln("clear - clears screen");
	kprintln("help - this help..");
}

void recursive(int i) 
{
	kprintf("tal: %d\n", i++);
	recursive(i);
}


