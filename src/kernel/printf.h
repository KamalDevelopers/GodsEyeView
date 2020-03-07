#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>
#include "itoa.h"
#include "string.h"

unsigned short* VideoMemory = (unsigned short*)0xb8000;
int VideoMemoryIndex = 0;

void puts(char* str)
{
	for (int i = 0; str[i] != '\0'; i++){
		VideoMemory[VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | str[i];
		VideoMemoryIndex++;
	}
}

void puti(int num)
{
	char* str;
	itoa(num, str);

	for (int i = 0; str[i] != '\0'; i++){
		VideoMemory[VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | str[i];
		VideoMemoryIndex++;
	}
}

void putc(int c)
{
	VideoMemory[VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | c;
	VideoMemoryIndex++;
}

void vprintf(const char *format, va_list v)
{
	int size = len(format);
	int i = 0;
	char res[100];
	while (i < size)
	{
		if (format[i] == '%')
		{

			if (format[i+1] == 's'){
				puts(va_arg(v, char*));
			}

			if (format[i+1] == 'd'){
				puti(va_arg(v, int));
			}

			if (format[i+1] == 'c'){
				putc(va_arg(v, int));
			}
		}
		i+=2;
	}
}

void printf (const char *format, ...)
{
   va_list arg;
   int done;

   va_start (arg, format);
   vprintf (format, arg);
   va_end (arg);
}

void clear()
{
	for (int i = 0; i < 2200; i++){
		VideoMemory[i] = (VideoMemory[i] & 0xff00) | ' ';
	}
}

#endif