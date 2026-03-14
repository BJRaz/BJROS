/* Simple spinlock implementation for i386 */
#include <spinlock.h>

/* Nothing else needed since header contains inline impls. Provide a C symbol for init if needed. */
void spinlock_init_c(spinlock_t *lock)
{
    spinlock_init(lock);
}
