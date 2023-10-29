#include "string.h"

size_t _strlen(const char* str)
{
    int l = 0;
    while (str[l] != '\0')
        l++;
    return l;
}

size_t strlen(const char* str)
{
    const char* char_ptr = str;
    const size_t* longword_ptr;
    size_t longword, magic_bits, himagic, lomagic;

    /* if (sizeof(longword) > 4)
        return _strlen(str); */

    for (; ((size_t)char_ptr & (sizeof(longword) - 1)) != 0; ++char_ptr)
        if (*char_ptr == '\0')
            return char_ptr - str;

    longword_ptr = (size_t*)char_ptr;
    magic_bits = 0x7efefeffL;
    himagic = 0x80808080L;
    lomagic = 0x01010101L;

    for (;;) {
        longword = *longword_ptr++;

        if (((longword - lomagic) & himagic) != 0) {
            const char* cp = (const char*)(longword_ptr - 1);

            if (cp[0] == 0)
                return cp - str;
            if (cp[1] == 0)
                return cp - str + 1;
            if (cp[2] == 0)
                return cp - str + 2;
            if (cp[3] == 0)
                return cp - str + 3;
        }
    }
}

char* strchrnul(const char* s, int c_in)
{
    const unsigned char* char_ptr;
    const unsigned long int* longword_ptr;
    unsigned long int longword, magic_bits, charmask;
    unsigned char c;

    c = (unsigned char)c_in;

    for (char_ptr = (const unsigned char*)s;
         ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
         ++char_ptr)
        if (*char_ptr == c || *char_ptr == '\0')
            return (char*)char_ptr;
    longword_ptr = (unsigned long int*)char_ptr;
    magic_bits = -1;
    magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;

    charmask = c | (c << 8);
    charmask |= charmask << 16;
    if (sizeof(longword) > 4)
        charmask |= (charmask << 16) << 16;
    if (sizeof(longword) > 8)
        return 0;

    for (;;) {
        longword = *longword_ptr++;

        if ((((longword + magic_bits)
                 ^ ~longword)
                & ~magic_bits)
                != 0
            ||

            ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
                & ~magic_bits)
                != 0) {
            const unsigned char* cp = (const unsigned char*)(longword_ptr - 1);

            if (*cp == c || *cp == '\0')
                return (char*)cp;
            if (*++cp == c || *cp == '\0')
                return (char*)cp;
            if (*++cp == c || *cp == '\0')
                return (char*)cp;
            if (*++cp == c || *cp == '\0')
                return (char*)cp;
            if (sizeof(longword) > 4) {
                if (*++cp == c || *cp == '\0')
                    return (char*)cp;
                if (*++cp == c || *cp == '\0')
                    return (char*)cp;
                if (*++cp == c || *cp == '\0')
                    return (char*)cp;
                if (*++cp == c || *cp == '\0')
                    return (char*)cp;
            }
        }
    }

    return NULL;
}

#define BITOP(a, b, op) \
    ((a)[(size_t)(b) / (8 * sizeof *(a))] op(size_t) 1 << ((size_t)(b) % (8 * sizeof *(a))))

size_t strcspn(const char* s, const char* c)
{
    const char* a = s;
    size_t byteset[32 / sizeof(size_t)];

    if (!c[0] || !c[1])
        return strchrnul(s, *c) - a;

    memset(byteset, 0, sizeof byteset);
    for (; *c && BITOP(byteset, *(unsigned char*)c, |=); c++)
        ;
    for (; *s && !BITOP(byteset, *(unsigned char*)s, &); s++)
        ;
    return s - a;
}

size_t strspn(const char* str1, const char* str2)
{
    const char* a = str1;
    size_t byteset[32 / sizeof(size_t)] = { 0 };

    if (!str2[0])
        return 0;
    if (!str2[1]) {
        for (; *str1 == *str2; str1++)
            ;
        return str1 - a;
    }

    for (; *str2 && BITOP(byteset, *(unsigned char*)str2, |=); str2++)
        ;
    for (; *str1 && BITOP(byteset, *(unsigned char*)str1, &); str1++)
        ;
    return str1 - a;
}

char* strcpy(char* s1, const char* s2)
{
    char* save = s1;
    for (; (*s1 = *s2); ++s2, ++s1)
        ;
    return save;
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

char* strcat(char* dest, const char* src)
{
    char* ptr = dest + strlen(dest);
    while (*src != '\0')
        *ptr++ = *src++;
    *ptr = '\0';
    return dest;
}

char* strchr(const char* s, int c)
{
    if (s == NULL)
        return NULL;

    while (*s != 0) {
        if (*s == (char)c)
            return (char*)s;
        s++;
    }
    return NULL;
}

const char* strstr(const char* str1, const char* str2)
{
    size_t n = strlen(str2);
    while (*str1) {
        if (!memcmp(str1, str2, n))
            return str1;
        str1++;
    }
    return 0;
}

char* strtok(char* str, const char* sep)
{
    static char* p;
    if (!str && !(str = p))
        return NULL;
    str += strspn(str, sep);
    if (!*str)
        return p = 0;
    p = str + strcspn(str, sep);
    if (*p)
        *p++ = 0;
    else
        p = 0;
    return str;
}

char strpbrk(char* str, const char* cmp)
{
    int l = strlen(str);
    int x = 0;
    int y = 0;

    while (x < l) {
        while (y < strlen(cmp)) {
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
}

void itos(int n, char str[])
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
