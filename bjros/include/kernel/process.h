/* process.h - process control block and management
 * BJROS microkernel
 */
#ifndef PROCESS_H
#define PROCESS_H

#include <standard/stdint.h>

#define PROCESS_STACK_SIZE  4096        /* 4 KB per process */
#define MAX_PROCESSES       8

enum process_state {
	PROC_READY,
	PROC_RUNNING,
	PROC_BLOCKED
};

struct process {
	uint32_t            pid;
	uint32_t            esp;        /* saved stack pointer */
	enum process_state  state;
	uint32_t            stack_top;  /* top of allocated stack region */
	struct process     *next;       /* circular linked list */
};

typedef void (*process_entry_t)(void);

/* Create a new process with the given entry function.
 * Stack is allocated from fixed process stack region.
 * Returns pointer to PCB or NULL on failure. */
struct process *process_create(process_entry_t entry);

/* Get the currently running process */
struct process *process_current(void);

/* Initialize the process subsystem */
void process_init(void);

#endif
