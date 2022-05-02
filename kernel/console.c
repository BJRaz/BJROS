#include <console.h>

/*
 *	TODO: check for buffer overflow
 * */
void prompt(void (*readbuf)(char*)) {
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

char _getchar(void) {
	while(kbdchar==0); 						// TODO: busy wait - refactor! 	
	char result = kbdchar;
	kbdchar = 0;
	return result;
}


