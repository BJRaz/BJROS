#ifndef CONSOLE_H
#define CONSOLE_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BUFFERLEN	255			// input buffer length

#define KBD_ARRAY_SIZE	128

#ifdef __cplusplus
extern "C" {
#endif
	extern char kbdchar;
	extern char kbdarray[KBD_ARRAY_SIZE];
	extern char kbdarray_upper[KBD_ARRAY_SIZE];	

	extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
	extern uint32_t vx, vy;				// 

	void prompt(void (*)(char*));
#ifdef __cplusplus
}
#endif
#endif
