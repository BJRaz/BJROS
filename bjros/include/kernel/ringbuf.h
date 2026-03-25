/* ringbuf.h - generic circular ring buffer
 * BJROS microkernel
 */
#ifndef RINGBUF_H
#define RINGBUF_H

#include <standard/stdint.h>

#define RINGBUF_SIZE 256

struct ringbuf {
	uint8_t  data[RINGBUF_SIZE];
	volatile uint32_t head;
	volatile uint32_t tail;
};

void ringbuf_init(struct ringbuf *rb);
int  ringbuf_put(struct ringbuf *rb, uint8_t byte);
int  ringbuf_get(struct ringbuf *rb, uint8_t *byte);
int  ringbuf_empty(const struct ringbuf *rb);
int  ringbuf_full(const struct ringbuf *rb);

#endif
