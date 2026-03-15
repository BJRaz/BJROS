#include <standard/stdio.h>
#include <standard/string.h>
#include <console.h>

/* Prototype for _memcpy (not declared in kernel standard headers) */
void* _memcpy(void* dst, const void* src, int n);

/* Simple kernel test runner that exercises libc functions and prints results */
static int assert_str_equal(const char* a, const char* b, const char* testname) {
    if (_strcmp(a,b) == 0) {
        kprintf("PASS: %s -> %s\n", testname, a);
        return 1;
    } else {
        kprintf("FAIL: %s expected '%s' got '%s'\n", testname, b, a);
        return 0;
    }
}

void run_kernel_tests(void) {
    char buf[32];

    _memset(buf, 0, 32);
    _itoa(-1234, buf);
    assert_str_equal(buf, "-1234", "itoa negative");

    _utoa(65535u, buf);
    assert_str_equal(buf, "65535", "utoa max");

    _utox(0xdeadbeefu, buf);
    /* lowercase hex expected */
    assert_str_equal(buf, "deadbeef", "utox hex");

    int v = _atoi("-42");
    if (v == -42) kprintf("PASS: atoi negative -> %d\n", v); else kprintf("FAIL: atoi negative -> %d\n", v);

    unsigned int uv = _atou("12345");
    if (uv == 12345u) kprintf("PASS: atou -> %u\n", uv); else kprintf("FAIL: atou -> %u\n", uv);

    _memcpy(buf, "hello", 6);
    assert_str_equal(buf, "hello", "memcpy hello");

    int ln = _strlen("abcd");
    if (ln == 4) kprintf("PASS: strlen -> %d\n", ln); else kprintf("FAIL: strlen -> %d\n", ln);

    kprintf("Kernel tests complete.\n");
}
