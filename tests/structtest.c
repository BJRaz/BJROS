#include <stdio.h>
#include <string.h>

struct __attribute__((packed)) test
{
	char c;
	unsigned int x;
};

int main() {

	struct test *hest;
       
	memset(hest, 10, sizeof (struct test));	//	= (struct test*)[10];

	hest[0].x = 100;
	hest[1].x = 200;

	printf("Her: %d\n", hest[0].x);
	printf("Addresses: %d, %d\n", &hest[0], &hest[1]);
	return 0;
} 
