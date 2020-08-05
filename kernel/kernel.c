// Brians own kernel main ... 
#include <stdio.h>

int kprint(const char*);
int kprintln(const char*);
int kprintf(const char* format, ...);

int kmain(void* multiboot_structure, void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	
	char* text = "Welcome to Brians kernel...\nSystem halted\n";
	int len = kprint(text);
	kprintf("Kernel test output from kprintf '-190000000': %d\n", -190000000);
	kprintf(" -  string 'Brian': %s\n", "Brian");
	kprintf(" -  string 'Brian' and number '200': %s, %d\n", "Brian", 200);

	kprintln(buffer);
	for(int i=0;i<23;i++){
		_itoa(i, buffer);
		kprintf("Number: %s\n", buffer);
	}
	return len;	
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
	int *args = (int*)&format + 1;

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
						 _itoa(*args++, buf);
						kprint(buf);
						format++;
					}
					break;
					case 's':	// string
					{
						kprint(*args++);	
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
}
