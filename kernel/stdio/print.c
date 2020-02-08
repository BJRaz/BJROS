int print() {
	//__asm__(".intel_syntax noprefix");
	__asm__("movl $0x2f4b2f4f,  0xb8008");
	return 64;	
} 
