// Brians own kernel main ... 
#include <stdio.h>

extern int gdt;

struct __attribute__ ((__packed__)) gdtr 
{
	short limit;
	int baseaddress;
};

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
	_utoa(4000000000, buffer);
	kprintf(" -  '4000000000': %s\n", buffer);

	kprintln(buffer);
	
	struct gdtr *gdtreg = (struct gdtr*) &gdt;
	
	kprintf("GDT address: %d\n", &gdt);

	kprintf("GDTR limit value: %d\n", gdtreg->limit);
			
	kprintf("GDTR baseaddress value: %d\n", gdtreg->baseaddress);

	for(int i=0;i<23;i++){
		_itoa(i, buffer);
		kprintf("Number: %s\n", buffer);
	}
	return len;	
} 
