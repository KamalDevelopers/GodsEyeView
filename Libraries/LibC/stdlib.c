#include "stdlib.h"
#include "unistd.h"

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

uint16_t flip_short(uint16_t short_int)
{
    uint32_t first_byte = *((uint8_t*)(&short_int));
    uint32_t second_byte = *((uint8_t*)(&short_int) + 1);
    return (first_byte << 8) | (second_byte);
}

uint32_t flip_long(uint32_t long_int)
{
    uint32_t first_byte = *((uint8_t*)(&long_int));
    uint32_t second_byte = *((uint8_t*)(&long_int) + 1);
    uint32_t third_byte = *((uint8_t*)(&long_int)  + 2);
    uint32_t fourth_byte = *((uint8_t*)(&long_int) + 3);
    return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

uint8_t flip_byte(uint8_t byte, int num_bits)
{
    uint8_t t = byte << (8 - num_bits);
    return t | (byte >> num_bits);
}

uint8_t htonb(uint8_t byte, int num_bits)
{
    return flip_byte(byte, num_bits);
}

uint8_t ntohb(uint8_t byte, int num_bits)
{
    return flip_byte(byte, 8 - num_bits);
}

uint16_t htons(uint16_t hostshort)
{
    return flip_short(hostshort);
}

uint32_t htonl(uint32_t hostlong)
{
    return flip_long(hostlong);
}

uint16_t ntohs(uint16_t netshort)
{
    return flip_short(netshort);
}

uint32_t ntohl(uint32_t netlong)
{
    return flip_long(netlong);
}
