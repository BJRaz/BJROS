//
// implementation of _clean, and _putchar
// Brian Juul Rasmussen jan 2020
// 
// todo:
// error handling
// input-routines 
// cursor-routines
//

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
