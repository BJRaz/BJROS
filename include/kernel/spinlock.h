/* simple spinlock for i386 */
#ifndef KERNEL_SPINLOCK_H
#define KERNEL_SPINLOCK_H

typedef volatile unsigned int spinlock_t;

static inline void spinlock_init(spinlock_t *lock)
{
    *lock = 0;
}

static inline void spinlock_lock(spinlock_t *lock)
{
    unsigned int tmp = 1;
    while(1) {
        __asm__ __volatile__(
            "xchgl %0, %1"
            : "=r" (tmp), "+m" (*lock)
            : "0" (tmp)
            : "memory"
        );
        if (tmp == 0) break; /* acquired */
        /* pause hint */
        __asm__ __volatile__("pause");
        tmp = 1;
    }
}

static inline int spinlock_trylock(spinlock_t *lock)
{
    unsigned int tmp = 1;
    __asm__ __volatile__(
        "xchgl %0, %1"
        : "=r" (tmp), "+m" (*lock)
        : "0" (tmp)
        : "memory"
    );
    return (tmp == 0); /* 1 if acquired */
}

static inline void spinlock_unlock(spinlock_t *lock)
{
    __asm__ __volatile__("movl $0, %0" : "+m" (*lock) : : "memory");
}

#endif
