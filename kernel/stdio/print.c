//
// implementation of _clean, and _putchar
// Brian Juul Rasmussen jan 2020
// 
// aug 2020: added kprintf, kprint and kprintln
//
// todo:
// error handling
// input-routines 
// cursor-routines
//

#include <stdio.h>

#define VIDEO	0xB8000			// VGA color text buffer (mode 3)
#define VIDEO_X	80
#define VIDEO_Y	24
#define ATTRIBUTE 0x07			// attribute byte 7(blink), 654(backcolor), 3(fg bright bit), 210(forecolor)

unsigned int 	vx = 0;
unsigned int 	vy = 0;
unsigned char* video = (unsigned char*)VIDEO;

void _clean() 
{
	video = (unsigned char*)VIDEO;
	for(int i=0;i<VIDEO_X * (VIDEO_Y + 1) * 2;i++)
	{
		video[i] = 0;
	}
}

void _putchar(char c) 
{
	if(c == '\n')
	{
		video = (unsigned char*)VIDEO + (VIDEO_X * ++vy * 2);	// mult by 2 to account for attrbute + char (2 bytes)
		return;
	}
	*video++ = c;		// 0x4b;	char
	*video++ = ATTRIBUTE;	// 0xaf;	attribute
} 

int kprint(const char* text) {
	//__asm__(".intel_syntax noprefix");
	// __asm__("movl $0xaf4b2f4f,  0xb8008");

	int i = 0;
	for(;i<_strlen(text);i++)
	{
		char c = text[i];
		_putchar(c);		
	}	
	return i;	
}

int kprintln(const char* text) {
	int i =	kprint(text);
	_putchar('\n');
	return i+1;	
}

int kprintf(const char* format, ...)
{
	int count = 0;
	void* args = &format + 1;

	while(*format != '\0')
	{
		
		switch(*format) 
		{
			case '%':
				format++;
				switch(*format)
				{
					case 'd':	// convert to decimal (signed)
					{
						char buf[20];
						_itoa(*(int*)args, buf);
						kprint(buf);
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 's':	// string
					{
						kprint(*(char**)args);	
						args = 4 + (char*)args;
						format++;
					}
					break;
				}
			break;
			
		}
		
		_putchar(*format);
		format++;
		count++;
	}
	return count;
}
