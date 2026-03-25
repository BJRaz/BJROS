#include <serial.h>

void serial_init()
{
	outb(COM1 + 1, 0x00);		// disable all interrupts
	outb(COM1 + 3, 0x80);		// enable DLAB (set baud rate divisor)
	outb(COM1 + 0, 0x01);		// divisor 1 = 115200 baud
	outb(COM1 + 1, 0x00);		// hi byte
	outb(COM1 + 3, 0x03);		// 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7);		// enable FIFO, clear, 14-byte threshold
	outb(COM1 + 4, 0x0B);		// IRQs enabled, RTS/DSR set
}

void serial_putchar(char c)
{
	while (!(inb(COM1 + 5) & 0x20));	// wait for transmit buffer empty
	outb(COM1, c);
}

void serial_print(const char* str)
{
	while (*str) {
		if (*str == '\n')
			serial_putchar('\r');
		serial_putchar(*str++);
	}
}
