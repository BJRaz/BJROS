// Brians own kernel main ... 
#include <stdio.h>
int print();

int kmain(void* multiboot_structure, void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);
	int len = print();
	return len;	//print();	//(int)magicvalue;
} 

int print() {
	//__asm__(".intel_syntax noprefix");
	// __asm__("movl $0xaf4b2f4f,  0xb8008");

	char* text = "Brian Juul Rasmussen\n\nOluf Bagers Gade 7,3\n1\n2\n";
	int i = 0;
	for(;i<_strlen(text);i++)
	{
		char c = text[i];
		_putchar(c);		
	
	}	

	
	return _strlen(text);	
}
