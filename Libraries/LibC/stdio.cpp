#include "LibC/stdio.hpp"

void puts(char* str)
{
    int len = strlen(str);
    write(1, str, len);
}

void putc(int c)
{
    char buff[2];
    buff[0] = c;
    buff[1] = '\0';
    puts(buff);
}

void puti(int num)
{
    char str[20];
    itoa(num, str);
    puts(str);
}

void putf(float f)
{
    char* str;
    ftoa(f, str, 2);
    puts(str);
}

void putx(int c)
{
    char* f = "00";
    char* hex = "0123456789ABCDEF";
    f[0] = hex[(c >> 4) & 0xF];
    f[1] = hex[c & 0xF];
    puts(f);
}

void vprintf(const char* format, va_list v)
{
    int size = len(format);
    int i = 0;
    int flag = 0;

    while (i < size) {
        if (flag > 0)
            flag--;

        if ((flag == 0) && (format[i] != '%') && (format[i] != '\n') && (format[i] != '\b')) {
            putc(format[i]);
            i++;
        }

        if (format[i] == '%') {
            flag = 2;
            if (format[i + 1] == 's')
                puts(va_arg(v, char*));

            if ((format[i + 1] == 'd') || (format[i + 1] == 'i'))
                puti(va_arg(v, int));

            if (format[i + 1] == 'f')
                putf(va_arg(v, double));

            if (format[i + 1] == 'c')
                putc(va_arg(v, int));

            if (format[i + 1] == 'x')
                putx(va_arg(v, int));
            i += 2;
        }

        if (format[i] == '\n') {
            flag = 1;
            newline();
            i++;
        }
        if (format[i] == '\b') {
            flag = 1;
            i++;
        }
    }
}

void printf(const char* format, ...)
{
    va_list arg;
    int done;

    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

void clear()
{
    puts("\33[H\33[2J");
}

void outb(uint16_t port, uint8_t data)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %1, %0"
                 : "=a"(result)
                 : "Nd"(port));
    return result;
}

void sleep(uint32_t timer_count)
{
    while (1) {
        asm volatile("nop");
        timer_count--;
        if (timer_count <= 0)
            break;
    }
}

void usleep(uint32_t ms)
{
    while (1) {
        sleep(40000);
        ms--;
        if (ms <= 0)
            break;
    }
}

void beep(uint32_t ms_time, uint32_t frequency)
{
    asm("int $0x80"
        :
        : "a"(400), "b"(ms_time), "c"(frequency));
}
