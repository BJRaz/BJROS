/* sched.h - round-robin scheduler
 * BJROS microkernel
 */
#ifndef SCHED_H
#define SCHED_H

#include <process.h>

/* Time slice in timer ticks before preemption */
#define SCHED_QUANTUM  10

/* Initialize scheduler with idle process */
void sched_init(void);

/* Add a process to the run queue */
void sched_add(struct process *p);

/* Called from timer ISR to perform context switch.
 * old_esp is the saved ESP of the interrupted process.
 * Returns the ESP of the next process to run. */
uint32_t schedule(uint32_t old_esp);

/* Yield the current time slice voluntarily */
void sched_yield(void);

#endif
