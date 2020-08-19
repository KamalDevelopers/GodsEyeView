#include "stdlib.hpp"

unsigned rand(unsigned int seed = 918, unsigned int max = 36000)
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
    char *pszDest = (char*)dst;
    const char *pszSource = (const char*)src;
    while(cnt)
    {
        *(pszDest++)= *(pszSource++);
        --cnt;
    }
    return dst;
}

void* memset(void *b, char c, int len)
{
    char *b_char = (char *)b;
    while(*b_char && len > 0)
    {
        *b_char = c;
        b_char++;
        len--;
    }
    return b;
}

int atoi(char* str)
{
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
     return res;
}

unsigned int abs(int num)
{
    if (num >= 0) return num;
    return num-(num*2);
}

div_t div(int numerator, int denominator)
{
    div_t res;
    res.quot = numerator / denominator;
    res.rem = numerator % denominator;
    return res;
}
