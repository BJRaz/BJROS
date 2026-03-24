/* header file for kernel/stdio 
 * Brian Juul Rasmussen 2025
 * */
#ifndef KERNEL_STDIO_H
#define KERNEL_STDIO_H

	int kprint(const char*);
	int kprintln(const char*);
	int kprintf(const char* format, ...);
	int _atoi(const char*);
	int _atou(const char*);
	int _itoa(int, char*);
	int _utoa(unsigned int, char*);
	int _utox(unsigned int, char*);
	void _putchar(const char);
	char _getchar(void);
	void _scrollup(void);
	void _clear();
	void* _memset(void* buffer, unsigned char c, int size);
	void* _malloc(unsigned int size);
	void  _free(void* ptr);

#endif

