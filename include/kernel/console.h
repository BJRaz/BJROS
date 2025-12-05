#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel.h>

extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
extern uint32_t vx, vy;				// 

void prompt(void (*)(const char*));

#endif
