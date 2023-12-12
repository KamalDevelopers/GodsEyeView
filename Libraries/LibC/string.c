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

size_t strcspn(const char* s1, const char* s2)
{
    const char* a = s1;
    size_t byteset[32 / sizeof(size_t)];

    if (!s2[0] || !s2[1])
        return strchrnul(s1, *s2) - a;

    memset(byteset, 0, sizeof byteset);
    for (; *s2 && BITOP(byteset, *(unsigned char*)s2, |=); s2++)
        ;
    for (; *s1 && !BITOP(byteset, *(unsigned char*)s1, &); s1++)
        ;
    return s1 - a;
}

size_t strspn(const char* s1, const char* s2)
{
    const char* a = s1;
    size_t byteset[32 / sizeof(size_t)] = { 0 };

    if (!s2[0])
        return 0;
    if (!s2[1]) {
        for (; *s1 == *s2; s1++)
            ;
        return s1 - a;
    }

    for (; *s2 && BITOP(byteset, *(unsigned char*)s2, |=); s2++)
        ;
    for (; *s1 && BITOP(byteset, *(unsigned char*)s1, &); s1++)
        ;
    return s1 - a;
}

char* strcpy(char* s1, const char* s2)
{
    char* save = s1;
    for (; (*s1 = *s2); ++s2, ++s1)
        ;
    return save;
}

char* strncpy(char* s1, const char* s2, int l)
{
    int x = 0;
    while (x != l) {
        *s1++ = *s2++;
        x++;
    }
    *s1 = 0;
    return s1;
}

int strncasecmp(const char* s1, const char* s2, int l)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int result;
    if (p1 == p2)
        return 0;
    for (; ((result = tolower(*p1) - tolower(*p2++)) == 0) && l; l--)
        if (*p1++ == '\0' || !l)
            break;
    return result;
}

int strcasecmp(const char* s1, const char* s2)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int result;
    if (p1 == p2)
        return 0;
    while ((result = tolower(*p1) - tolower(*p2++)) == 0)
        if (*p1++ == '\0')
            break;
    return result;
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

int strncmp(const char* s1, const char* s2, int l)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    int index = 0;

    while (*p1 != '\0') {
        if (index == l)
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

const char* strstr(const char* s1, const char* s2)
{
    size_t n = strlen(s2);
    while (*s1) {
        if (!memcmp(s1, s2, n))
            return s1;
        s1++;
    }
    return 0;
}

char* strtok(char* s, const char* sep)
{
    static char* p;
    if (!s && !(s = p))
        return NULL;
    s += strspn(s, sep);
    if (!*s)
        return p = 0;
    p = s + strcspn(s, sep);
    if (*p)
        *p++ = 0;
    else
        p = 0;
    return s;
}

char strpbrk(char* s, const char* cmp)
{
    int l = strlen(s);
    int x = 0;
    int y = 0;

    while (x < l) {
        while (y < strlen(cmp)) {
            if (s[x] == cmp[y]) {
                return s[x];
            }
            y++;
        }
        y = 0;
        x++;
    }
    return '\0';
}

float stof(const char* s)
{
    const char* a = s;
    float rez = 0, fact = 1;
    if (*a == '-') {
        a++;
        fact = -1;
    };
    for (int point_seen = 0; *a; a++) {
        if (*a == '.') {
            point_seen = 1;
            continue;
        };
        int d = *a - '0';
        if (d >= 0 && d <= 9) {
            if (point_seen)
                fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        };
    };
    return rez * fact;
}

void itos(int n, char s[])
{
    int i, sign;
    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
}

void uppercase(char* s)
{
    for (int i = 0; i < strlen(s); i++)
        s[i] = toupper(s[i]);
}

void lowercase(char* s)
{
    for (int i = 0; i < strlen(s); i++)
        s[i] = tolower(s[i]);
}
