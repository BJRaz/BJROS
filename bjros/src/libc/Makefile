AS=nasm
ASFLAGS=-felf32
CC=gcc
CFLAGS=-nostdinc -Wpadded -std=c99 -m32 -c -Wall -ffreestanding -fno-stack-protector -I../../include/libc

OBJS=string.o stdio.o convert.o

VPATH=stdio

all: $(OBJS) 
	$(LD) -m elf_i386 -i $(OBJS) -o libc.o

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@
clean:
	rm -f *.o
	
