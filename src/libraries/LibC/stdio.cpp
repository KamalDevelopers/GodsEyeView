#include "stdio.hpp"

void init_serial()
{
    outb(COMPORT + 1, 0x00);
    outb(COMPORT + 3, 0x80);
    outb(COMPORT + 0, 0x03);
    outb(COMPORT + 1, 0x00);
    outb(COMPORT + 3, 0x03);
    outb(COMPORT + 2, 0xC7);
    outb(COMPORT + 4, 0x0B);
}

int transmit_empty()
{
    return inb(COMPORT + 5) & 0x20;
}

void log_putc(char c)
{
    while (transmit_empty() == 0)
        ;
    outb(COMPORT, c);
}

void klog(char* str)
{
    for (int i = 0; i < str_len(datacolorblue); i++)
        log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < str_len(datacoloroff); i++)
        log_putc(datacoloroff[i]); //color off
}

void klog(int num)
{
    char str[20];
    itoa(num, str);

    for (int i = 0; i < str_len(datacolorblue); i++)
        log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < str_len(datacoloroff); i++)
        log_putc(datacoloroff[i]); //color off
}

void puts(char* str)
{
    int len = strlen(str);
    asm("int $0x80"
        :
        : "a"(4), "b"(1), "c"(str), "d"(len));
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

            if (format[i + 1] == 'd')
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
    __asm__ volatile("outb %0, %1"
                     :
                     : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0"
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

void PCS_play_sound(uint32_t nFrequence)
{
    uint32_t Div;
    uint8_t tmp;

    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)(Div));
    outb(0x42, (uint8_t)(Div >> 8));

    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void PCS_nosound()
{
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep(int ms_time, int frequency)
{
    PCS_play_sound(frequency);
    usleep(ms_time);
    PCS_nosound();
}
