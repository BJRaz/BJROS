#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel.h>
#include <ringbuf.h>

extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
extern uint32_t vx, vy;				// 

/* Global keyboard input ring buffer — written by keyboard ISR */
extern struct ringbuf kbd_input_rb;

void prompt(void (*)(const char*));

/* Console process entry point for scheduler */
void console_main(void);

/* Set the command callback for the console process */
void console_set_callback(void (*cb)(const char*));

#endif
