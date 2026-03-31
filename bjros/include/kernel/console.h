#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel.h>
#include <ringbuf.h>

#define BUFFERLEN	255			// input buffer length

// *** characters
#define BSP		0x8			// backspace 
#define LF		0xa			// linefeed



extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
extern uint32_t vx, vy;				// 

/* Global keyboard input ring buffer — written by keyboard ISR */
struct ringbuf kbd_input_rb;

void prompt(void (*)(const char*));

/* Console process entry point for scheduler */
void console_main(void);

/* Set the command callback for the console process */
void console_set_callback(void (*cb)(const char*));

void test();
void showidtinfo(const struct interrupt_gate_descriptor* idt_array);
void showmbinfo(void);
int sysinfo(void);
void test_malloc(void);
void help(void);
void recursive(int i);
 
#endif
