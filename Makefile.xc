all:
	vbcci386 kernel/k.c -c99 -elf -Iinclude -Imultiboot -o=k.asm
	as -o k.o k.asm
	vlink -b elf32i386 -Bstatic -w -T linker.ld -o kernel.bin k.o bin/*.o bin/*.so 
