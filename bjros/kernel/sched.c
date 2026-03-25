/* sched.c - round-robin scheduler
 * BJROS microkernel
 *
 * Circular linked list of processes. Timer ISR calls schedule()
 * every SCHED_QUANTUM ticks. The idle process runs when no other
 * process is READY.
 */
#include <sched.h>
#include <kernel.h>
#include <standard/stddef.h>

static struct process *run_queue = NULL;
static uint32_t tick_count = 0;

static void idle_main(void)
{
	for (;;)
		halt;
}

void sched_init(void)
{
	process_init();
	tick_count = 0;
	run_queue = NULL;

	/* Create idle process (PID 0) — always in the run queue */
	struct process *idle = process_create(idle_main);
	if (idle) {
		idle->state = PROC_READY;
		idle->next = idle;       /* circular: points to itself */
		run_queue = idle;
	}
}

void sched_add(struct process *p)
{
	if (!p)
		return;

	p->state = PROC_READY;

	if (!run_queue) {
		p->next = p;
		run_queue = p;
		return;
	}

	/* Insert after current head */
	p->next = run_queue->next;
	run_queue->next = p;
}

/* Called from timer ISR with the interrupted process's ESP.
 * Returns ESP of the next process to resume. */
uint32_t schedule(uint32_t old_esp)
{
	tick_count++;

	if (!run_queue)
		return old_esp;

	/* Save current process state */
	run_queue->esp = old_esp;
	if (run_queue->state == PROC_RUNNING)
		run_queue->state = PROC_READY;

	/* Advance to next process — skip BLOCKED */
	struct process *start = run_queue;
	do {
		run_queue = run_queue->next;
	} while (run_queue->state == PROC_BLOCKED && run_queue != start);

	run_queue->state = PROC_RUNNING;
	return run_queue->esp;
}

void sched_yield(void)
{
	/* Trigger a software interrupt to invoke the timer handler,
	 * which calls schedule(). This reuses the existing timer ISR path. */
	__asm__ volatile("int $32");
}
