#include <console.h>

/* Global keyboard ring buffer — keyboard ISR writes, console reads */
struct ringbuf kbd_input_rb;

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
		halt;
	return (char)c;
}

/*
 *	TODO: check for buffer overflow
 * */
void prompt(void (*readbuf)(const char*)) {
	char buf[BUFFERLEN];
	char *pmt = "BJROS> ";
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
					case 0x08:			// backspace char
						if(vx > len)		// TODO: refactor
						{
							_putchar(c);
							buf[--idx] = 0;	// remove char from buffer	
						}
						break;
					case 0x0a:			// linefeed
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
