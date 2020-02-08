// Brians own kernel main ... 
#include <stdio.h>

int kmain(void* multiboot_structure, void* magicvalue) {
	char* tal = "842";
	
	int result = _atoi(tal);

	return _strlen(tal);	//print();	//(int)magicvalue;
} 
