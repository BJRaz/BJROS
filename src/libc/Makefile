AS = nasm
ASFLAGS = -felf32
CC ?= gcc
CFLAGS = -nostdinc -Wpadded -std=c99 -m32 -Wall -ffreestanding -fno-stack-protector -I../../include/libc

OBJS = string.o strlen.o utoa.o utox.o itoa.o atou.o atoi.o

VPATH = ../../kernel/stdio

all: libc.a

libc.a: $(OBJS)
	ar rcs $@ $(OBJS)

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o libc.a
	
