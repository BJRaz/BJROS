#include <stdio.h>

int main() 
{
	int number = 0;
	char buffer[20];
	_itoa(number, buffer);

	printf("Buffer: %s\n", buffer);
	return 0;
}
