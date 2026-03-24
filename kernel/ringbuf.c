/* ringbuf.c - generic circular ring buffer
 * BJROS microkernel
 *
 * Fixed 256-byte buffer. Power-of-2 size allows bitmask modulo.
 * Safe for single-producer/single-consumer (ISR → process).
 */
#include <ringbuf.h>

#define RINGBUF_MASK (RINGBUF_SIZE - 1)

void ringbuf_init(struct ringbuf *rb)
{
	rb->head = 0;
	rb->tail = 0;
}

/* Returns 0 on success, -1 if buffer full */
int ringbuf_put(struct ringbuf *rb, uint8_t byte)
{
	uint32_t next = (rb->head + 1) & RINGBUF_MASK;
	if (next == rb->tail)
		return -1;
	rb->data[rb->head] = byte;
	rb->head = next;
	return 0;
}

/* Returns 0 on success, -1 if buffer empty */
int ringbuf_get(struct ringbuf *rb, uint8_t *byte)
{
	if (rb->head == rb->tail)
		return -1;
	*byte = rb->data[rb->tail];
	rb->tail = (rb->tail + 1) & RINGBUF_MASK;
	return 0;
}

int ringbuf_empty(const struct ringbuf *rb)
{
	return rb->head == rb->tail;
}

int ringbuf_full(const struct ringbuf *rb)
{
	return ((rb->head + 1) & RINGBUF_MASK) == rb->tail;
}
