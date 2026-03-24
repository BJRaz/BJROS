/* process.c - process management
 * BJROS microkernel
 *
 * Process stacks are carved from a fixed region starting at PROC_STACK_BASE.
 * Each process gets PROCESS_STACK_SIZE bytes. No dynamic allocation needed.
 */
#include <process.h>
#include <standard/stdint.h>
#include <standard/stddef.h>
#include <standard/string.h>

#define PROC_STACK_BASE  0x400000      /* fixed region for process stacks */

static struct process proc_table[MAX_PROCESSES];
static uint32_t next_pid = 0;
static struct process *current_proc = NULL;

void process_init(void)
{
	int i;
	for (i = 0; i < MAX_PROCESSES; i++) {
		proc_table[i].pid = 0;
		proc_table[i].esp = 0;
		proc_table[i].state = PROC_READY;
		proc_table[i].stack_top = 0;
		proc_table[i].next = NULL;
	}
	next_pid = 0;
	current_proc = NULL;
}

struct process *process_create(process_entry_t entry)
{
	if (next_pid >= MAX_PROCESSES)
		return NULL;

	struct process *p = &proc_table[next_pid];
	p->pid = next_pid;

	/* Stack grows downward: stack_top is the highest address in the region */
	p->stack_top = PROC_STACK_BASE + (next_pid + 1) * PROCESS_STACK_SIZE;

	/* Set up initial stack frame so context switch can pop into the entry.
	 * Layout (top to bottom):
	 *   EFLAGS (0x202 = IF set)
	 *   CS     (0x08)
	 *   EIP    (entry function)
	 *   EAX, ECX, EDX, EBX, ESP_dummy, EBP, ESI, EDI  (pushal frame)
	 */
	uint32_t *sp = (uint32_t *)p->stack_top;

	/* IRET frame */
	*(--sp) = 0x00000202;           /* EFLAGS: IF=1 */
	*(--sp) = 0x08;                 /* CS: kernel code segment */
	*(--sp) = (uint32_t)entry;      /* EIP: process entry point */

	/* pushal frame (8 registers): EDI ESI EBP ESP EBX EDX ECX EAX */
	*(--sp) = 0;    /* EDI */
	*(--sp) = 0;    /* ESI */
	*(--sp) = 0;    /* EBP */
	*(--sp) = 0;    /* ESP (ignored by popal) */
	*(--sp) = 0;    /* EBX */
	*(--sp) = 0;    /* EDX */
	*(--sp) = 0;    /* ECX */
	*(--sp) = 0;    /* EAX */

	p->esp = (uint32_t)sp;
	p->state = PROC_READY;

	next_pid++;
	return p;
}

struct process *process_current(void)
{
	return current_proc;
}
