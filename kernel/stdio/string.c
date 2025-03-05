#include <standard/string.h>


void* _memset(void* buffer, unsigned char c, int size) {
	int idx = 0;
	while(idx < size) 
	{
		*(unsigned char*)buffer = c;
		idx++;
		buffer++;
	}
	return buffer -= size;
}


