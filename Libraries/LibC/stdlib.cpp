#include "stdlib.hpp"
#include "unistd.hpp"
#include "utility.hpp"

void exit(int status)
{
    _exit(status);
}

unsigned rand(unsigned int seed, unsigned int max)
{
    seed = seed * 1103515245 + 12345;
    return ((unsigned)(seed / ((max + 1) * 2)) % (max + 1));
}

unsigned int random(unsigned int seed, unsigned int max)
{
    return rand(seed, max);
}

void* memcpy(void* dst, const void* src, unsigned int cnt)
{
    char* psz_dest = (char*)dst;
    const char* psz_source = (const char*)src;
    while (cnt) {
        *(psz_dest++) = *(psz_source++);
        --cnt;
    }
    return dst;
}

void* memcpy32(void* dst, const void* src, size_t cnt)
{
    uint32_t num_dwords = cnt / 4;
    uint32_t num_bytes = cnt % 4;
    uint32_t* dest32 = (uint32_t*)dst;
    uint32_t* src32 = (uint32_t*)src;
    uint8_t* dest8 = ((uint8_t*)dst) + num_dwords * 4;
    uint8_t* src8 = ((uint8_t*)src) + num_dwords * 4;
    uint32_t i;

    for (i = 0; i < num_dwords; i++)
        dest32[i] = src32[i];
    for (i = 0; i < num_bytes; i++)
        dest8[i] = src8[i];
    return dst;
}

void* memset(void* b, char c, int len)
{
    char* b_char = (char*)b;
    while (*b_char && len > 0) {
        *b_char = c;
        b_char++;
        len--;
    }
    return b;
}

void reverse(char* str, int len)
{
    int i = 0, j = len - 1;
    while (i < j) {
        swap(str[i++], str[j--]);
    }
}

int digit_count(int num)
{
    int count = 0;
    if (num == 0)
        return 1;

    while (num > 0) {
        count++;
        num = num / 10;
    }
    return count;
}

void itoa(unsigned int num, char* number)
{
    if (num > 1000000000) {
        *number = '\0';
        return;
    }

    int dgcount = digit_count(num);
    int index = dgcount - 1;
    char x;
    if (num == 0 && dgcount == 1) {
        number[0] = '0';
        number[1] = '\0';
    } else {
        while (num != 0) {
            x = num % 10;
            number[index] = x + '0';
            index--;
            num = num / 10;
        }
        number[dgcount] = '\0';
    }
}

int atoi(char* str)
{
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
    return res;
}

int itoan(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
    while (i < d)
        str[i++] = '0';
    reverse(str, i);
    str[i] = '\0';
    return i;
}

char* ftoa(float n, char* res, int afterpoint)
{
    int ipart = (int)n;
    float fpart = n - (float)ipart;
    int i = itoan(ipart, res, 0);
    if (afterpoint != 0) {
        res[i] = '.';
        fpart = fpart * pow(10, afterpoint);
        itoan((int)fpart, res + i + 1, afterpoint);
    }
    return res;
}

div_t div(int numerator, int denominator)
{
    div_t res;
    res.quot = numerator / denominator;
    res.rem = numerator % denominator;
    return res;
}
