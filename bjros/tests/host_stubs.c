/* Stubs for host testing of libc functions */
#include <stdint.h>

volatile char kbdchar = 0;

int _getchar(void) {
    return 0;  // Stub: not available in host tests
}

void _putchar(char c) {
    // Stub
}
