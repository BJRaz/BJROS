/* convert.c - numeric conversion functions
 * bjros tiny libc
 */
#include <stdio.h>

/* signed int → decimal string; returns number of chars written */
int _itoa(int n, char* buf)
{
	char tmp[12];
	int i = 0, neg = 0, len;

	if (n < 0) {
		neg = 1;
		n = -n;
	}
	if (n == 0) {
		tmp[i++] = '0';
	} else {
		while (n) {
			tmp[i++] = '0' + (n % 10);
			n /= 10;
		}
	}
	if (neg)
		tmp[i++] = '-';

	len = i;
	int j = 0;
	while (i--)
		buf[j++] = tmp[i];
	buf[j] = '\0';
	return len;
}

/* unsigned int → decimal string */
int _utoa(unsigned int n, char* buf)
{
	char tmp[11];
	int i = 0, len;

	if (n == 0) {
		tmp[i++] = '0';
	} else {
		while (n) {
			tmp[i++] = '0' + (n % 10);
			n /= 10;
		}
	}

	len = i;
	int j = 0;
	while (i--)
		buf[j++] = tmp[i];
	buf[j] = '\0';
	return len;
}

/* unsigned int → hex string (no "0x" prefix) */
int _utox(unsigned int n, char* buf)
{
	char tmp[9];
	int i = 0, len;
	static const char hex[] = "0123456789abcdef";

	if (n == 0) {
		tmp[i++] = '0';
	} else {
		while (n) {
			tmp[i++] = hex[n & 0xf];
			n >>= 4;
		}
	}

	len = i;
	int j = 0;
	while (i--)
		buf[j++] = tmp[i];
	buf[j] = '\0';
	return len;
}

/* decimal string → signed int */
int _atoi(const char* s)
{
	int n = 0, neg = 0;
	if (*s == '-') { neg = 1; s++; }
	while (*s >= '0' && *s <= '9')
		n = n * 10 + (*s++ - '0');
	return neg ? -n : n;
}

/* decimal string → unsigned int */
int _atou(const char* s)
{
	unsigned int n = 0;
	while (*s >= '0' && *s <= '9')
		n = n * 10 + (*s++ - '0');
	return (int)n;
}
