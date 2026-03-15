/* string.c - string and memory functions
 * bjros tiny libc
 */
#include <string.h>

void* _memset(void* buf, uint8_t c, int n)
{
	unsigned char* p = (unsigned char*)buf;
	while (n--)
		*p++ = c;
	return buf;
}

void* _memcpy(void* dst, const void* src, int n)
{
	unsigned char* d = (unsigned char*)dst;
	const unsigned char* s = (const unsigned char*)src;
	while (n--)
		*d++ = *s++;
	return dst;
}

int _strlen(const char* s)
{
	int n = 0;
	while (*s++)
		n++;
	return n;
}

int _strcmp(const char* a, const char* b)
{
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return (unsigned char)*a - (unsigned char)*b;
}
