// Brian Juul Rasmussen 2020
//
#include <stdio.h>
#include <includes/stdio.h>
#define DEBUG

void printverylong();
int kprintf(const char* format, ...);

int main(int argc, void** argv) 
{
	char* arg1 = 0;

	if(argc > 1)
		arg1 = (char*)argv[1];

	int number = _atoi(arg1);

	int result = 0;
	char buffer[20];
	_utoa(number, buffer);
	printf("utoa: %s\n", buffer);

	result = _itoa(number, buffer);
	printf("itoa: %s\n", buffer);


	printverylong();
	
		kprintf("Her: %s, %d\n", "Brian", 200);
	return result;
}

void printverylong() 
{
	unsigned long long int verylong = 5000000000;
	unsigned long long int verylong2 = 1;

	verylong += verylong2;
	
	printf("Very long: %llu\n", verylong);
}
/*
int kprintf(const char* format, ...)
{
	int count = 0;
	int *args = (int*)&format + 1;

	while(*format != '\0')
	{
		
		switch(*format) 
		{
			case '%':
				format++;
				switch(*format)
				{
					case 'd':
					{
						char buf[20];
						 _itoa(*args, buf);
						printf("%s", buf);
						format++;
					}
					break;
				}
			break;
		}
		
		printf("%c", *format);
		format++;
		count++;
	}
}*/
