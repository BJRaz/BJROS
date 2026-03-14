/* Buffered console API for kernel */
#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include <standard/stdint.h>

/* initialize console buffers */
void console_init(void);

/* enqueue a character from non-ISR context (blocks briefly if contended) */
int console_putc(int c);

/* enqueue a character from ISR context (non-blocking). Returns 1 if enqueued, 0 if dropped */
int console_putc_isr(int c);

/* ISR-safe keyboard enqueue (called from asm ISR). Returns 1 on success, 0 if dropped */
int console_input_try_enqueue(int ch);

/* process buffers: flush logs to screen and move keyboard bytes into `kbdchar` */
void console_process(void);

/* overflow counters for diagnostics */
unsigned int console_log_overflow_count(void);
unsigned int console_kbd_overflow_count(void);

#endif
#ifndef CONSOLE_H
#define CONSOLE_H

#include <kernel.h>

extern void setcursor(uint32_t x, uint32_t y);	// sets cursor on screen
extern uint32_t vx, vy;				// 

void prompt(void (*)(const char*));

#endif
