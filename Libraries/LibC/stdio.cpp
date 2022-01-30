#include "stdio.hpp"
#include "string.hpp"

void puts_hook(void (*t)(char*))
{
    hwrite = t;
}

void flush()
{
    if (hwrite == 0) {
        write(1, write_buffer, write_index);
    } else {
        hwrite(write_buffer);
    }

    memchr(write_buffer, 0, BUFSIZ);
    write_index = 0;
}

void puts(char* str)
{
    if (hwrite != 0) {
        hwrite(str);
        return;
    }

    int len = strlen(str);
    bool do_flush = false;

    if (len > BUFSIZ) {
        write(1, str, len);
        return;
    }

    if ((write_index + len) >= BUFSIZ)
        flush();

    for (int i = 0; i < len; i++) {
        write_buffer[write_index] = str[i];
        write_index++;

        if (str[i] == 10)
            do_flush = true;
    }

    if (do_flush)
        flush();
}

void putc(int c)
{
    char buff[2];
    buff[0] = c;
    buff[1] = 0;
    puts(buff);
}

void puti(int num)
{
    if (num < 0) {
        num = -num;
        puts("-");
    }
    char str[20];
    itoa(num, str);
    puts(str);
}

void putf(float f)
{
    if (f < 0.0f) {
        f = -f;
        puts("-");
    }
    char* str;
    ftoa(f, str, 2);
    puts(str);
}

void putx(int c)
{
    char hex[20];
    char res[20];

    int i = 0;
    while (c != 0) {
        int temp = 0;
        temp = c % 16;
        if (temp < 10) {
            hex[i] = temp + 48;
            i++;
        } else {
            hex[i] = temp + 55;
            i++;
        }
        c = c / 16;
    }

    int iteration = 0;
    for (int j = i - 1; j >= 0; j--) {
        res[iteration] = hex[j];
        iteration += 1;
    }
    res[iteration] = '\0';
    puts(res);
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

    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

void clear()
{
    puts("\33\x1");
}

void beep(uint32_t ms_time, uint32_t frequency)
{
    asm("int $0x80"
        :
        : "a"(400), "b"(ms_time), "c"(frequency));
}
