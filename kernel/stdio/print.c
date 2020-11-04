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
#define VIDEO_Y	25
#define ATTRIBUTE 0x07			// attribute byte 7(blink), 654(backcolor), 3(fg bright bit), 210(forecolor)

unsigned int 	vx = 0;
unsigned int 	vy = 0;
unsigned char* video = (unsigned char*)VIDEO;

extern void _scrollup();

void _clean() 
{
	video = (unsigned char*)VIDEO;
	for(int i=0;i<VIDEO_X * VIDEO_Y * 2;i++)
	{
		video[i] = 0;
	}
}

void _putchar(char c) 
{
	if(c == '\n')
	{
		if(vy < VIDEO_Y)
			video = (unsigned char*)VIDEO + (VIDEO_X * ++vy * 2);	// mult by 2 to account for attrbute + char (2 bytes)
		if(vy >= VIDEO_Y)
		{	
			video = (unsigned char*)0xb8f00;	// (unsigned char*)VIDEO + VIDEO_X * (VIDEO_Y - 1) * 2 ;	
			_scrollup();
			//vy--;
		}
		return;
	}
	vx++;
	
	*video++ = c;		// 0x4b;	char
	*video++ = ATTRIBUTE;	// 0xaf;	attribute
	if(video > 0xb8fa0-2)
	{
		_scrollup();
		video = (unsigned char*)0xb8f00;
	}
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
	void* args = (int*)(&format + 4);	// TODO: in clang +4, in gcc this has to be add with +1

	while(*format != '\0')			// TODO: optimize this 
	{
		
		switch(*format) 
		{
			case '%':
				format++;
				switch(*format)
				{		
				
					// TODO: optimize to include some base-number
					case 'x':	// convert to hexadecimal (unsigned)
					{
						char buf[11];
						_utox(*(unsigned int*)args, buf);
						kprint(buf);	
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 'd':	// convert to decimal (signed)
					{
						char buf[11];
						_itoa(*(int*)args, buf);
						kprint(buf);
						args = 1 + (int*)args;
						format++;
					}
					break;
					case 'u':	// convert to decimal (unsigned)
					{
						char buf[11];
						_utoa(*(unsigned int*)args, buf);
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
