// Brian Juul Rasmussen 2020
//
#include <stdio.h>
#include <stdint.h>
#include <includes/stdio.h>
#define DEBUG

void printverylong();
int _kprintf(const char* format, ...);
void testuinttohex(uint32_t number);

void* _memset(void* buffer, unsigned char c, int size) {
	int idx = 0;
	void *start = buffer;
	while(idx < size) {
		*(unsigned char*)buffer = c;
		idx++;
		buffer++;
	}
	return buffer -= size;
}


int main(int argc, char** argv) 
{
	char* text1 = "Brian tester: 0x%x";
	_kprintf(text1, 16);
	return 0;
	char* arg1 = 0;

	if(argc > 1)
		arg1 = (char*)argv[1];
	
	char buf[20];

 	void* p = _memset(buf, 0, 20);

	char str = 'M'; //"Brian";
	printf("%d\n", strlen(str));

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
	
	// test _utox (uint to hex)
	char c = 'B';
	_kprintf("c char: %c\n", c);
	
	// misc tests
	printverylong();
	
	testuinttohex(100);

	_kprintf("Her: %d\n", 200);
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

int _kprintf(const char* format, ...)
{
	int count = 0;
	int *args = (int*)(&format + 1);

	while(*format != '\0')
	{
		
		switch(*format) 
		{
			case '%':
				format++;
				switch(*format)
				{
					case 'x':	// convert to hexadecimal (unsigned)
					{
						char buf[11];
						_utox(*(unsigned int*)args, buf);
						printf(buf);	
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 'd':	// convert to decimal (signed)
					{
						char buf[11];
						_itoa(*(int*)args, buf);
						printf(buf);
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 'u':	// convert to decimal (unsigned)
					{
						char buf[11];
						_utoa(*(unsigned int*)args, buf);
						printf(buf);
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 's':	// string
					{
						printf(*(char**)args);	
						args = 4 + (char*)args;
						format++;
					}
					break;
					case 'c':	// string
					{
						printf(*args);	
						args = 4 + (char*)args;
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
	return count;
}
