// Brian Juul Rasmussen 2020
//
#include <stdio.h>
#include <includes/stdio.h>

void printverylong();
int kprintf(const char* format, ...);

int main(int argc, void** argv) 
{

	kprintf("Hest: %d\n", 200);
	printverylong();
	
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

void printverylong() 
{
	unsigned long long int verylong = 5000000000;
	unsigned long long int verylong2 = 1;

	verylong += verylong2;
	
	printf("Very long: %llu\n", verylong);
}

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
}
