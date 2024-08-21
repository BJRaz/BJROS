AS=nasm
ASFLAGS=-felf32
CC=gcc
CFLAGS=-nostdinc -Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -I../../include/libc

OBJS=string.so string.o strlen.o utoa.o utox.o itoa.o atou.o atoi.o 

VPATH=stdio

all: $(OBJS) 
	$(LD) -m elf_i386 -i $(OBJS) -o libc.o

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@
%.so: %.c
	$(CC) $(CFLAGS) $< -o $@  
clean:
	rm *.o *.so
	
