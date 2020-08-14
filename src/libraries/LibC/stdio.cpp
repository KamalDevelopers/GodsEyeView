#include "stdio.hpp"

void init_serial() {
   outb(COMPORT + 1, 0x00);
   outb(COMPORT + 3, 0x80);
   outb(COMPORT + 0, 0x03);
   outb(COMPORT + 1, 0x00);
   outb(COMPORT + 3, 0x03);
   outb(COMPORT + 2, 0xC7);
   outb(COMPORT + 4, 0x0B);
}

int transmit_empty() {
   return inb(COMPORT + 5) & 0x20;
}

void log_putc(char c) {
   while (transmit_empty() == 0);
   outb(COMPORT, c);
}

void klog(char* str) {
    for (int i = 0; i < str_len(datacolorblue); i++) log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < str_len(datacoloroff); i++) log_putc(datacoloroff[i]); //color off
}

void klog(int num)
{
    char* str;
    itoa(num, str);

    for (int i = 0; i < str_len(datacolorblue); i++) log_putc(datacolorblue[i]); //color on
    for (int i = 0; str[i] != '\0'; i++) {
        log_putc(str[i]);
    }
    log_putc('\n');
    for (int i = 0; i < str_len(datacoloroff); i++) log_putc(datacoloroff[i]); //color off
}

void indexmng()
{
    if (VideoMemoryIndex >= 80) {
        VideoMemoryIndex = 0;
        NewLineIndex++;
    }
}

void puts(char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != 10)
            putc(str[i]);
        else {
            NewLineIndex++;
            VideoMemoryIndex = 0;
        }
        indexmng();

    }
}

void puti(int num)
{
    char* str;
    itoa(num, str);

    for (int i = 0; str[i] != '\0'; i++) {
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

/* Not real implementation, TODO */
void putf(float f)
{
    int c = f;
    puti(c);
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
    char res[100];
    int flag = 0;
    while (i < size) {
        if (flag > 0) { flag--; }

        if ((flag == 0) && (format[i] != '%') && (format[i] != '\n') && (format[i] != '\b')) {
            putc(format[i]);
            i++;
        }

        if (format[i] == '%') {
            flag = 2;
            if (format[i + 1] == 's') {
                puts(va_arg(v, char*));
            }

            if (format[i + 1] == 'd') {
                puti(va_arg(v, int));
            }

            if (format[i + 1] == 'f') {
                putf(va_arg(v, double));
            }

            if (format[i + 1] == 'c') {
                putc(va_arg(v, int));
            }

            if (format[i + 1] == 'x') {
                putx(va_arg(v, int));
            }
            i += 2;
        }

        if (format[i] == '\n') {
            flag = 1;
            NewLineIndex++;
            VideoMemoryIndex = 0;
            i++;
        }
        if (format[i] == '\b') {
            flag = 1;
            VideoMemoryIndex--;
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
    VideoMemoryIndex = 0;
    NewLineIndex = 0;
    for (int i = 0; i < 2200; i++) {
        VideoMemory[i] = (VideoMemory[i] & 0xff00) | ' ';
    }
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

void PCS_play_sound(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;

    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));

    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void PCS_nosound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void beep(int ms_time, int frequency) {
    PCS_play_sound(frequency);
    usleep(ms_time);
    PCS_nosound();
}
