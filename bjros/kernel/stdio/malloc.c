/* malloc.c - simple first-fit heap allocator
 * BJROS microkernel
 *
 * Heap lives at a fixed address (HEAP_START) defined in the linker script.
 * First-fit free list with block coalescing on free.
 */
#include <standard/stdint.h>
#include <standard/stddef.h>

#define HEAP_START  0x300000
#define HEAP_SIZE   0x100000    /* 1 MB */

struct block_header {
	uint32_t size;              /* usable bytes (excludes header) */
	uint32_t free;
	struct block_header *next;
};

#define HEADER_SIZE  (sizeof(struct block_header))
#define ALIGN4(x)   (((x) + 3) & ~3)

static struct block_header *free_list = NULL;
static int heap_initialized = 0;

static void heap_init(void)
{
	free_list = (struct block_header *)HEAP_START;
	free_list->size = HEAP_SIZE - HEADER_SIZE;
	free_list->free = 1;
	free_list->next = NULL;
	heap_initialized = 1;
}

/* Split a block if it is large enough to hold the requested size
 * plus another header + at least 4 usable bytes. */
static void split_block(struct block_header *blk, uint32_t size)
{
	if (blk->size >= size + HEADER_SIZE + 4) {
		struct block_header *new_blk =
			(struct block_header *)((uint8_t *)blk + HEADER_SIZE + size);
		new_blk->size = blk->size - size - HEADER_SIZE;
		new_blk->free = 1;
		new_blk->next = blk->next;

		blk->size = size;
		blk->next = new_blk;
	}
}

void *_malloc(uint32_t size)
{
	if (!heap_initialized)
		heap_init();

	if (size == 0)
		return NULL;

	size = ALIGN4(size);

	struct block_header *curr = free_list;
	while (curr) {
		if (curr->free && curr->size >= size) {
			split_block(curr, size);
			curr->free = 0;
			return (void *)((uint8_t *)curr + HEADER_SIZE);
		}
		curr = curr->next;
	}

	return NULL;
}

void _free(void *ptr)
{
	if (!ptr)
		return;

	struct block_header *blk =
		(struct block_header *)((uint8_t *)ptr - HEADER_SIZE);
	blk->free = 1;

	/* Coalesce adjacent free blocks */
	struct block_header *curr = free_list;
	while (curr) {
		if (curr->free && curr->next && curr->next->free) {
			curr->size += HEADER_SIZE + curr->next->size;
			curr->next = curr->next->next;
			continue;   /* check again in case of triple-merge */
		}
		curr = curr->next;
	}
}
