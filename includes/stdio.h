/* header file for stdio 
 * Brian Juul Rasmussen 2020
 * */
#ifndef STDIO_H
#define STDIO_H

int _atoi(const char*);
int _atou(const char*);
int _itoa(int, char*);
int _utoa(unsigned int, char*);
int _utox(unsigned int, char*);
int _strlen(const char*);
void _putchar(const char);
char _getchar(void);
//void _scrollup(void);
void* _memset(void* buffer, unsigned char c, int size);

int kprint(const char*);
int kprintln(const char*);
int kprintf(const char* format, ...);


#endif

