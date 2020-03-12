#include "stdio.hpp"

void indexmng()
{
	if (VideoMemoryIndex >= 80)
	{
		VideoMemoryIndex = 0;
		NewLineIndex++;
	}
}

void puts(char* str)
{
	for (int i = 0; str[i] != '\0'; i++){
		VideoMemory[80 * NewLineIndex + VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | str[i];
		VideoMemoryIndex++;
		indexmng();
	}
}

void puti(int num)
{
	char* str;
	itoa(num, str);

	for (int i = 0; str[i] != '\0'; i++){
		VideoMemory[80 * NewLineIndex + VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | str[i];
		VideoMemoryIndex++;
		indexmng();
	}
}

void putc(int c)
{
	VideoMemory[80 * NewLineIndex + VideoMemoryIndex] = (VideoMemory[VideoMemoryIndex] & 0xff00) | c;
	VideoMemoryIndex++;
	indexmng();
}

void putx(int c)
{
	char* f = "00";
	char* hex = "0123456789ABCDEF";
	f[0] = hex[(c >> 4) & 0xF];
	f[1] = hex[c & 0xF];
	puts(f);
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

			if (format[i+1] == 'x'){
				putx(va_arg(v, int));
			}
			i+=2;
		}

		if (format[i] == '\n')
		{
			NewLineIndex++;
			VideoMemoryIndex = 0;
			i++;
		}
		if (format[i] == '\b')
		{
			VideoMemoryIndex--;
			i++;
		}
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