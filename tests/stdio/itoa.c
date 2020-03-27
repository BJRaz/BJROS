#include <stdio.h>
#include <includes/stdio.h>

int main(int argc, void** argv) 
{
	char* arg1 = 0;

	if(argc > 1)
		arg1 = (char*)argv[1];

	int number = _atoi(arg1);
	printf("Number: %d\n", number);
	
	char buffer[20];
	int result = _itoa(number, buffer);

	printf("Buffer: %s\n", buffer);
	return result;
}
