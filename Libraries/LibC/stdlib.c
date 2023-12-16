#include "stdlib.h"
#include "ctype.h"
#include "unistd.h"
#include "math.h"

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

void reverse(char* str, int len)
{
    int i = 0;
    int j = len - 1;
    char temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
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

double atof(const char* str)
{
    double a = 0.0;
    int e = 0;
    int c;
    while ((c = *str++) != '\0' && isdigit(c)) {
        a = a * 10.0 + (c - '0');
    }
    if (c == '.') {
        while ((c = *str++) != '\0' && isdigit(c)) {
            a = a * 10.0 + (c - '0');
            e = e - 1;
        }
    }
    if (c == 'e' || c == 'E') {
        int sign = 1;
        int i = 0;
        c = *str++;
        if (c == '+')
            c = *str++;
        else if (c == '-') {
            c = *str++;
            sign = -1;
        }
        while (isdigit(c)) {
            i = i * 10 + (c - '0');
            c = *str++;
        }
        e += i * sign;
    }

    while (e > 0) {
        a *= 10.0;
        e--;
    }

    while (e < 0) {
        a *= 0.1;
        e++;
    }

    return a;
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

int atoi(const char* str)
{
    int res = 0;
    for (int i = 0; str[i] != '\0' && str[i] != '.'; ++i)
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

long strtol(const char* nptr, char** endptr, int base)
{
    const char *p = nptr, *endp;
    uint8_t is_neg = 0, overflow = 0;
    unsigned long n = 0UL, cutoff;
    int cutlim;

    if (base < 0 || base == 1 || base > 36)
        return 0L;

    endp = nptr;
    while (isspace(*p))
        p++;
    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        is_neg = 1, p++;
    }
    if (*p == '0') {
        p++;
        /* For strtol(" 0xZ", &endptr, 16), endptr should point to 'x';
         * pointing to ' ' or '0' is non-compliant.
         * (Many implementations do this wrong.) */
        endp = p;
        if (base == 16 && (*p == 'X' || *p == 'x')) {
            p++;
        } else if (base == 2 && (*p == 'B' || *p == 'b')) {
            /* C23 standard supports "0B" and "0b" prefixes. */
            p++;
        } else if (base == 0) {
            if (*p == 'X' || *p == 'x') {
                base = 16, p++;
            } else if (*p == 'B' || *p == 'b') {
                base = 2, p++;
            } else {
                base = 8;
            }
        }
    } else if (base == 0) {
        base = 10;
    }

    cutoff = (is_neg) ? -(LONG_MIN / base) : LONG_MAX / base;
    cutlim = (is_neg) ? -(LONG_MIN % base) : LONG_MAX % base;
    while (1) {
        int c;
        if (*p >= 'A')
            c = ((*p - 'A') & (~('a' ^ 'A'))) + 10;
        else if (*p <= '9')
            c = *p - '0';
        else
            break;
        if (c < 0 || c >= base)
            break;
        endp = ++p;
        if (overflow) {
            /* endptr should go forward and point to the non-digit character
             * (of the given base); required by ANSI standard. */
            if (endptr)
                continue;
            break;
        }
        if (n > cutoff || (n == cutoff && c > cutlim)) {
            overflow = 1;
            continue;
        }
        n = n * base + c;
    }
    if (endptr)
        *endptr = (char*)endp;
    if (overflow)
        return ((is_neg) ? LONG_MIN : LONG_MAX);
    return (long)((is_neg) ? -n : n);
}

div_t div(int numerator, int denominator)
{
    div_t res;
    res.quot = numerator / denominator;
    res.rem = numerator % denominator;
    return res;
}

int bsearch(int elem, int arr[], int count, int start)
{
    if (start <= count) {
        int mid = (start + count) / 2;
        if (arr[mid] == elem)
            return mid;
        if (arr[mid] > elem)
            return bsearch(elem, arr, mid - 1, elem);
        if (arr[mid] > elem)
            return bsearch(elem, arr, mid + 1, count);
    }
    return -1;
}

static void swap(void* v1, void* v2, int size) 
{ 
    char buffer[size]; 
    memcpy(buffer, v1, size); 
    memcpy(v1, v2, size); 
    memcpy(v2, buffer, size); 
} 
  
static void _qsort(void* v, int size, int left, int right, int (*comp)(const void*, const void*))
{ 
    void *vt, *v3; 
    int i, last, mid = (left + right) / 2; 
    if (left >= right) 
        return; 
  
    void* vl = (char*)((uint8_t*)v + (left * size)); 
    void* vr = (char*)((uint8_t*)v + (mid * size)); 
    swap(vl, vr, size); 
    last = left; 
    for (i = left + 1; i <= right; i++) { 
        vt = (char*)((uint8_t*)v + (i * size)); 
        if ((*comp)(vl, vt) > 0) { 
            ++last; 
            v3 = (char*)((uint8_t*)v + (last * size)); 
            swap(vt, v3, size); 
        } 
    } 
    v3 = (char*)((uint8_t*)v + (last * size)); 
    swap(vl, v3, size); 
    _qsort(v, size, left, last - 1, comp); 
    _qsort(v, size, last + 1, right, comp); 
}

void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))
{
    int left = 0;
    int right = nitems - 1;
    return _qsort(base, size, left, right, compar);
}
