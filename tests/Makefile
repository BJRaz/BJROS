vpath %.h ../include

CC ?= gcc

LIBC_DIR = ../src/libc

all: itoa_test memset_test

libc: 
	cd $(LIBC_DIR) && $(MAKE)

INCLUDES = -I.. -I../include/kernel -I../include/libc

itoa_test: libc stdio/itoa.c
	$(CC) -m32 $(INCLUDES) -g stdio/itoa.c -o itoa_test $(LIBC_DIR)/libc.a

memset_test: libc tests/libc/string_memset_test.c
	$(CC) -m32 $(INCLUDES) -g tests/libc/string_memset_test.c -o memset_test $(LIBC_DIR)/libc.a

clean:
	-rm -f itoa_test memset_test
