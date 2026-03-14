/* Buffered console implementation
 * drop-new policy for ISR enqueues
 */
#include <console.h>
#include <spinlock.h>
#include <standard/stddef.h>
#include <standard/stdio.h>

/* Sizes */
#define LOG_BUF_SIZE 4096
#define KBD_BUF_SIZE 256

static char log_buf[LOG_BUF_SIZE];
static unsigned int log_head = 0, log_tail = 0;
static spinlock_t log_lock;
static unsigned int log_overflow = 0;

static char kbd_buf[KBD_BUF_SIZE];
static unsigned int kbd_head = 0, kbd_tail = 0;
static spinlock_t kbd_lock;
static unsigned int kbd_overflow = 0;

/* set in header: extern char kbdchar; */

void console_init(void)
{
    spinlock_init(&log_lock);
    spinlock_init(&kbd_lock);
    log_head = log_tail = 0;
    kbd_head = kbd_tail = 0;
    log_overflow = 0;
    kbd_overflow = 0;
}

static int log_is_full(void)
{
    return ((log_head + 1) % LOG_BUF_SIZE) == log_tail;
}

static int log_is_empty(void)
{
    return log_head == log_tail;
}

static void log_enqueue_char(char c)
{
    log_buf[log_head] = c;
    log_head = (log_head + 1) % LOG_BUF_SIZE;
}

static int log_dequeue_char(char *c)
{
    if (log_is_empty()) return 0;
    *c = log_buf[log_tail];
    log_tail = (log_tail + 1) % LOG_BUF_SIZE;
    return 1;
}

static int kbd_is_full(void)
{
    return ((kbd_head + 1) % KBD_BUF_SIZE) == kbd_tail;
}

static int kbd_is_empty(void)
{
    return kbd_head == kbd_tail;
}

static void kbd_enqueue_char(char c)
{
    kbd_buf[kbd_head] = c;
    kbd_head = (kbd_head + 1) % KBD_BUF_SIZE;
}

static int kbd_dequeue_char(char *c)
{
    if (kbd_is_empty()) return 0;
    *c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return 1;
}

int console_putc(int c)
{
    spinlock_lock(&log_lock);
    if (log_is_full()) {
        log_overflow++;
        spinlock_unlock(&log_lock);
        return 0;
    }
    log_enqueue_char((char)c);
    spinlock_unlock(&log_lock);
    return 1;
}

int console_putc_isr(int c)
{
    /* non-blocking enqueue for ISR: trylock, drop-new on contention */
    if (!spinlock_trylock(&log_lock)) {
        log_overflow++;
        return 0;
    }
    if (log_is_full()) {
        log_overflow++;
        spinlock_unlock(&log_lock);
        return 0;
    }
    log_enqueue_char((char)c);
    spinlock_unlock(&log_lock);
    return 1;
}

int console_input_try_enqueue(int ch)
{
    if (!spinlock_trylock(&kbd_lock)) {
        kbd_overflow++;
        return 0;
    }
    if (kbd_is_full()) {
        kbd_overflow++;
        spinlock_unlock(&kbd_lock);
        return 0;
    }
    kbd_enqueue_char((char)ch);
    spinlock_unlock(&kbd_lock);
    return 1;
}

void console_process(void)
{
    char c;
    /* flush log buffer to _putchar (non-ISR context) */
    if (spinlock_trylock(&log_lock)) {
        while (log_dequeue_char(&c)) {
            _putchar(c);
        }
        spinlock_unlock(&log_lock);
    } else {
        /* If contended, skip flushing this cycle */
    }

    /* move one kbd char into global kbdchar if empty */
    if (kbdchar == 0) {
        if (spinlock_trylock(&kbd_lock)) {
            if (kbd_dequeue_char(&c)) {
                kbdchar = c;
            }
            spinlock_unlock(&kbd_lock);
        }
    }
}

unsigned int console_log_overflow_count(void) { return log_overflow; }
unsigned int console_kbd_overflow_count(void) { return kbd_overflow; }
#include <console.h>

/*
 *	TODO: check for buffer overflow
 * */
void prompt(void (*readbuf)(const char*)) {
	char buf[BUFFERLEN];
	char *pmt = "BJROS> ";
	int len = _strlen(pmt);
	while(1) {
		_memset(buf, 0, BUFFERLEN);
		
		kprintf(pmt);
		setcursor(vx, vy);
		uint8_t idx = 0;
		char c = 0;
		do
		{ 
			if(idx == BUFFERLEN)
				break;
			c = _getchar();
			if(c != 0) {
				switch(c) {
					case 0x08:			// backspace char
						if(vx > len)		// TODO: refactor
						{
							_putchar(c);
							buf[--idx] = 0;	// remove char from buffer	
						}
						break;
					case 0x0a:			// linefeed
						_putchar(c);
						break;
					default:			// prints the actual character to screen and puts it in buffer
						_putchar(c);
						buf[idx++] = c;
				}
			}
		       	if(vx >= len)					// TODO: refactor	
				setcursor(vx, vy);
		} while(c != '\n'); 
		(*readbuf)(buf);					// call the callback function
	}
}



