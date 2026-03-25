vpath %.h ../include
CC=gcc
CFLAGS=-nostdinc -Wpadded -std=c99 -c -Wall -I../include/libc
LIBC_OBJS=test_string.o test_stdio.o test_convert.o test_stubs.o

# Compile libc functions for host testing (no 32-bit, no -ffreestanding)
test_%.o: ../src/libc/%.c
	$(CC) $(CFLAGS) $< -o $@

test_stubs.o: test_stubs.c
	$(CC) $(CFLAGS) $< -o $@

tests: $(LIBC_OBJS) stdio/itoa.c
	$(CC) -I.. -g stdio/itoa.c $(LIBC_OBJS) -o test

clean:
	-rm test $(LIBC_OBJS) test_stubs.c
