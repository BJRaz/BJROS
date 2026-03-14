#include <stdio.h>
#include <libc/string.h>

int main(void) {
    char buf[32];
    for (int i = 0; i < (int)sizeof(buf); ++i) buf[i] = 0xAA;
    _memset(buf, 0, sizeof(buf));
    for (int i = 0; i < (int)sizeof(buf); ++i) {
        if (buf[i] != 0) {
            printf("FAIL: buf[%d]=%02x\n", i, (unsigned char)buf[i]);
            return 1;
        }
    }
    printf("PASS: _memset zeroed buffer\n");
    /* also test strlen/strcmp basic behavior on empty string */
    if (_strlen("") != 0) { printf("FAIL: strlen\n"); return 1; }
    if (_strcmp("a","a") != 0) { printf("FAIL: strcmp eq\n"); return 1; }
    if (_strcmp("a","b") == 0) { printf("FAIL: strcmp neq\n"); return 1; }
    printf("PASS: string helpers\n");
    return 0;
}
