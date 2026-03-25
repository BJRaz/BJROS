#ifndef SERIAL_H
#define SERIAL_H

#include <kernel.h>

#define COM1 0x3F8

void serial_init();
void serial_putchar(char c);
void serial_print(const char* str);

#endif
