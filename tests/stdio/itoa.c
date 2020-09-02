// Brian Juul Rasmussen 2020
//
#include <stdio.h>
#include <stdint.h>
#include <includes/stdio.h>
#define DEBUG

void printverylong();
int kprintf(const char* format, ...);
void testuinttohex(uint32_t number);

int main(int argc, void** argv) 
{
	char* arg1 = 0;

	if(argc > 1)
		arg1 = (char*)argv[1];
	// test _atoi
	int number = _atoi(arg1);

	// test _atou
	unsigned int unumber = _atou(arg1);

	printf("unumber: %u\n", unumber);

	// test _utoa
	int result = 0;
	char buffer[20];
	_utoa(number, buffer);
	printf("utoa: %s\n", buffer);
	// test _itoa
	result = _itoa(number, buffer);
	printf("itoa: %s\n", buffer);

	// test _utox (uint to hex)
	_utox(number, buffer);
	printf("utox: %s\n", buffer);
	
	// misc tests
	printverylong();
	
	testuinttohex(100);

	//kprintf("Her: %s, %d\n", "Brian", 200);
	return result;
}

void testuinttohex(uint32_t number)
{
	uint8_t rest = number / 16;
	printf("Rest: %d\n", rest);
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
