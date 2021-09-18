// Brian Juul Rasmussen 2020
//
#include <stdio.h>
#include <include/string.h>
#include <string.h>
#include <stdint.h>
#include <include/stdio.h>
#define DEBUG

void printverylong();
int _kprintf(const char* format, ...);
void testuinttohex(uint32_t number);

// TODO: optimize
void* _memset(void* buffer, const unsigned char c, int size) {
	int idx = 0;
	void* start = buffer;
	while(idx++ < size) {
		*(unsigned char*)start++ = c;
	}
	return buffer;
}

void callback(char* buf) {
	_kprintf("Her: %s\n", buf); 
}

void prompt(void (*callback)(char*)) {
	(*callback)("Hest");
}

int main(int argc, char** argv) 
{
	char s[2];
       	s[0] = 'B';
	s[1] = 'r';
	const char* st1 = "Brian";
	const char* st2 = "Brian";
	if(_strcmp(st1, s) != 0) 
		puts("Not equal strings");
	else
		puts("Equal strings");


	prompt(callback);

	char* text1 = "Brian tester: 0x%x, 0x%x hest\n";
	_kprintf(text1, &st1, &st1);
	char* arg1 = 0;

	if(argc > 1)
		arg1 = (char*)argv[1];
	
	char buf[5];

 	void* p = _memset(buf, 0, 5);

	char str = 'M'; //"Brian";
	printf("%d\n", strlen(&str));

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
						puts(buf);	
						args = 4 + (int*)args;
						format++;
					}
					break;
					case 'd':	// convert to decimal (signed)
					{
						char buf[11];
						_itoa(*(int*)args, buf);
						puts(buf);
						args = 4 + (int*)args;
						format++;
					}
					break;
					case 'u':	// convert to decimal (unsigned)
					{
						char buf[11];
						_utoa(*(unsigned int*)args, buf);
						puts(buf);
						args = 4 + (int*)args;
						format++;
					}
					break;
					case 's':	// string
					{
						puts(*(char**)args);	
						args = 4 + (int*)args;
						format++;
					}
					break;
					case 'c':	// string
					{
						puts((char*)args);	
						args = 4 + (int*)args;
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
