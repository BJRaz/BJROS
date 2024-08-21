// string.h - string functions
// BJR 2/1 2021
//
#ifndef STRING_H
#define STRING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int _strcmp(const char* str1, const char* str2);
int _strlen(const char*);
void* _memset(void* buffer, uint8_t c, int size);

#ifdef __cplusplus
}
#endif

#endif
