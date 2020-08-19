/* header file for stdio 
 * Brian Juul Rasmussen 2020
 * */
#ifndef STDIO_H
#define STDIO_H

int _atoi(const char*);
int _itoa(int, char*);
int _strlen(const char*);
void _putchar(const char);

int kprint(const char*);
int kprintln(const char*);
int kprintf(const char* format, ...);


#endif

