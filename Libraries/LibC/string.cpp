#include "LibC/string.hpp"

int str_len(char arr[])
{
    int l = 0;
    while (arr[l] != '\0') {
        l++;
    }
    return l;
}

size_t strlen(const char* str)
{
    return str_len((char*)str);
}

size_t strspn(char* str1, char* str2)
{
    int res = 0;
    while ((*str1) && (*str2)) {
        if (*str1++ != *str2++)
            return res;
        res++;
    }
    return res;
}

int len(const char* arr)
{
    int l = 0;
    while (arr[l] != '\0') {
        l++;
    }
    return l;
}

void* memchr(const void* str, int c, size_t n)
{
    unsigned char* p = (unsigned char*)str;
    while (n--)
        if (*p != (unsigned char)c)
            p++;
        else
            return p;
    return 0;
}

char* strcpy(char* arr, char* str)
{
    while (*str) {
        *arr++ = *str++;
    }
    *arr = 0;
    return arr;
}

char* strncpy(char* arr, const char* str, int l)
{
    int x = 0;

    while (x != l) {
        *arr++ = *str++;
        x++;
    }
    *arr = 0;
    return arr;
}

int strcmp(const char* s1, const char* s2)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;

    while (*p1 != '\0') {
        if (*p2 == '\0')
            return 1;
        if (*p2 > *p1)
            return -1;
        if (*p1 > *p2)
            return 1;

        p1++;
        p2++;
    }

    if (*p2 != '\0')
        return -1;

    return 0;
}

int strncmp(const char* s1, const char* s2, int count)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int index = 0;

    while (*p1 != '\0') {
        if (index == count)
            break;
        if (*p2 == '\0')
            return 1;
        if (*p2 > *p1)
            return -1;
        if (*p1 > *p2)
            return 1;

        p1++;
        p2++;
        index++;
    }

    if (*p2 != '\0')
        return -1;

    return 0;
}

char* findchar(const char* str, int c)
{
    const char* position = NULL;
    int i = 0;
    for (i = 0;; i++) {
        if ((unsigned char)str[i] == c) {
            position = &str[i];
            break;
        }
        if (str[i] == '\0')
            break;
    }
    return (char*)position;
}

char* strcat(char* dest, char* src)
{
    size_t i, j;
    i = str_len(dest);
    for (j = 0; src[j] != '\0'; j++)
        dest[i + j] = src[j];
    dest[i + j] = '\0';
    return dest;
}

void memcpy(char* dest, char* src, int count)
{
    char* cdest = (char*)dest;
    char* csrc = (char*)src;

    int x = 0;
    while (x <= count) {
        cdest[x] = csrc[x];
        x++;
    }
    return;
}

char* strtok(char* str, char* delimiter)
{
    static int pos;
    static char* s;
    int start = pos;

    if (str != 0)
        s = str;

    int j = 0;
    while (s[pos] != '\0') {
        j = 0;
        while (delimiter[j] != '\0') {
            if (s[pos] == delimiter[j]) {
                s[pos] = '\0';
                pos = pos + 1;
                if (s[start] != '\0')
                    return (&s[start]);
                else {
                    start = pos;
                    pos--;
                    break;
                }
            }
            j++;
        }
        pos++;
    }
    s[pos] = '\0';
    if (s[start] == '\0')
        return 0;
    else
        return &s[start];
}

char strpbrk(char* str, char* cmp)
{
    int l = str_len(str);
    int x = 0;
    int y = 0;

    while (x < l) {
        while (y < str_len(cmp)) {
            if (str[x] == cmp[y]) {
                return str[x];
            }
            y++;
        }
        y = 0;
        x++;
    }
    return '\0';
}

float stof(const char* str)
{
    const char* s = str;
    float rez = 0, fact = 1;
    if (*s == '-') {
        s++;
        fact = -1;
    };
    for (int point_seen = 0; *s; s++) {
        if (*s == '.') {
            point_seen = 1;
            continue;
        };
        int d = *s - '0';
        if (d >= 0 && d <= 9) {
            if (point_seen)
                fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        };
    };
    return rez * fact;
};

void int_to_ascii(int n, char str[])
{
    int i, sign;
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0)
        str[i++] = '-';
    str[i] = '\0';
}

void uppercase(char* str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = toupper(str[i]);
}

void lowercase(char* str)
{
    for (int i = 0; i < strlen(str); i++)
        str[i] = tolower(str[i]);
}
