#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
extern uint32_t vx, vy;				// 

void prompt(void (*)(char*));

#endif
