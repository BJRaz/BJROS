/* stdio.c - VGA text-mode I/O
 * bjros tiny libc
 *
 * _putchar  - write one character to the VGA screen (handles \n, backspace, scroll)
 * _scrollup - scroll the display up one line
 * _clear    - clear the screen
 * _getchar  - blocking read of the next keyboard character
 */
#include <stdio.h>

#define VGA_BASE   0xB8000
#define VGA_W      80
#define VGA_H      25
#define VGA_ATTR   0x07   /* light grey on black */

/* Cursor position - exposed as globals so kernel can read vx/vy if needed */
unsigned int vx = 0;
unsigned int vy = 0;

/* kbdchar is a byte in multiboot.asm section .data, set by the keyboard ISR */
extern volatile char kbdchar;

/* ---- port I/O helpers ---- */
static inline void outb(unsigned short port, unsigned char val)
{
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* serial out to COM1 (0x3f8) - used to echo kernel output to QEMU serial */
static inline void serial_putc(unsigned char c)
{
	/* simple, unbuffered write to COM1 */
	outb(0x3F8, c);
}

/* Move the hardware text cursor to (x, y) via CRT controller */
static void update_cursor(unsigned int x, unsigned int y)
{
	unsigned short pos = (unsigned short)(y * VGA_W + x);
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void _scrollup(void)
{
	unsigned short* vga = (unsigned short*)VGA_BASE;
	int i;
	for (i = 0; i < (VGA_H - 1) * VGA_W; i++)
		vga[i] = vga[i + VGA_W];
	for (i = (VGA_H - 1) * VGA_W; i < VGA_H * VGA_W; i++)
		vga[i] = (unsigned short)(VGA_ATTR << 8) | ' ';
}

void _clear(void)
{
	unsigned short* vga = (unsigned short*)VGA_BASE;
	int i;
	for (i = 0; i < VGA_H * VGA_W; i++)
		vga[i] = (unsigned short)(VGA_ATTR << 8) | ' ';
	vx = vy = 0;
	update_cursor(0, 0);
}

void _putchar(const char c)
{
	unsigned short* vga = (unsigned short*)VGA_BASE;

	if (c == '\n') {
		vx = 0;
		if (vy < VGA_H - 1)
			vy++;
		else
			_scrollup();
		update_cursor(vx, vy);
		return;
	}

	if (c == '\b') {
		if (vx > 0) {
			vx--;
			vga[vy * VGA_W + vx] = (unsigned short)(VGA_ATTR << 8) | ' ';
			update_cursor(vx, vy);
		}
		return;
	}

	vga[vy * VGA_W + vx] = (unsigned short)(VGA_ATTR << 8) | (unsigned char)c;
	vx++;
	if (vx >= VGA_W) {
		vx = 0;
		if (vy < VGA_H - 1)
			vy++;
		else
			_scrollup();
	}
	update_cursor(vx, vy);
	/* echo to serial for qemu -serial */
	serial_putc((unsigned char)c);
}

char _getchar(void)
{
	char c;
	while (kbdchar == 0)
		__asm__ volatile("hlt");
	c = kbdchar;
	kbdchar = 0;
	return c;
}
