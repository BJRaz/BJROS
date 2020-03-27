// Brians own kernel main ... 
#include <stdio.h>

int print(const char*);
int println(const char*);

int kmain(void* multiboot_structure, void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);
	char buffer[20];

	_itoa(result, buffer);
	
	char* text = "Welcome to Brians kernel...\nSystem halted\n";
	int len = print(text);
	println(buffer);
	for(int i=0;i<200;i++){
		_itoa(i, buffer);
		println(buffer);
	}
	return len;	
} 

int print(const char* text) {
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

int println(const char* text) {
	int i =	print(text);
	_putchar('\n');
	return i+1;	
}
