//
// implementation of _clear, and _putchar etc.
// Brian Juul Rasmussen jan 2020
// 
// aug 2020: added kprintf, kprint and kprintln
// dec 2020: added _memset
//
// todo:
// error handling
// input-routines 
// cursor-routines
//

#include <stdio.h>
#include <string.h>

#define VIDEO	0xB8000			// VGA color text buffer (mode 3)
#define VIDEO_X	80
#define VIDEO_Y	25
#define ATTRIBUTE 0x07			// attribute byte 7(blink), 654(backcolor), 3(fg bright bit), 210(forecolor)

unsigned int 	vx = 0;
unsigned int 	vy = 0;
unsigned char* video = (unsigned char*)VIDEO;

void _clear() 
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
newline:
		vx = 0;
		if(vy < VIDEO_Y - 1)
			vy++;
			
		else
			_scrollup();
		return;		
	}
	if(c == 0x08) 
	{	
		// backspace
		c = 0;
		vx--;
		*(video + (vx + vy * VIDEO_X) * 2) = c;
		*(video + (vx + vy * VIDEO_Y) * 2 + 1) = ATTRIBUTE;
		return;
	}

	*(video + (vx + vy * VIDEO_X) * 2) = c;
	*(video + (vx + vy * VIDEO_Y) * 2 + 1) = ATTRIBUTE;
	vx++;
	if(vx >= VIDEO_X)
		goto newline;
}

void _putchar_new(char c) 
{
	if(c == '\n')
	{
		if(vy < VIDEO_Y)
			video = (unsigned char*)VIDEO + (VIDEO_X * ++vy * 2);	// mult by 2 to account for attrbute + char (2 bytes)
		if(vy >= VIDEO_Y)
		{	
			video = (unsigned char*)0xb8f00;	// (unsigned char*)VIDEO + VIDEO_X * (VIDEO_Y - 1) * 2 ;	
			_scrollup();
			//vy++;
		}
		return;
	}
	if(c == 0x08) {	// backspace
		*video-- = 0;
		*video-- = 0;
		return;
	}
	vx++;
	if(vx == VIDEO_X*2)
	{
		vx = 0;
		vy++;
	}		
	*video++ = c;		// 0x4b;	char
	*video++ = ATTRIBUTE;	// 0xaf;	attribute
	if((int)video > 0xb8fa0-2)
	{
		_scrollup();
		video = (unsigned char*)0xb8f00;
	}
} 

void* _memset(void* buffer, unsigned char c, int size) {
	int idx = 0;
	while(idx < size) 
	{
		*(unsigned char*)buffer = c;
		idx++;
		buffer++;
	}
	return buffer -= size;
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
	void* args = (int*)(&format + 1);	// TODO: before: in clang +4, in gcc this has to be added with +1
						// now: is has to be +1 for clang as well (check)

	//while(*format != '\0')			// TODO: optimize this 
	while(1)
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
					case 'c':
					{
						char c = *(char*)args;
						_putchar(c);
						args = 4 + (char*)args;
						format++;
					}
					break;		
				}
			break;
		}
		if(*format != '\0'){		
			_putchar(*format);	
			format++;
			count++;
		} else
			break;
	}
	return count;
}
