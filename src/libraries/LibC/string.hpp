#ifndef STRING_HPP
#define STRING_HPP
#include "types.hpp"

static int str_len(char arr[])
{
	int l = 0;
	while (arr[l] != '\0')
	{
		l++;
	}
	return l;
}

static int len(const char* arr)
{
    int l = 0;
    while (arr[l] != '\0')
    {
        l++;
    }
    return l;
}

static void * strcpy(char *arr, const char *str)
{
	while (*str)
	{
		*arr++ = *str++;
	}
	*arr = 0;
	return 0;
}

static void * strncpy(char *arr, const char *str, int l)
{
	int x = 0;

	while (x != l)
	{
		*arr++ = *str++;
		x++;
	}
	*arr = 0;
	return 0;
}

static int strcmp(const char *s1, const char *s2) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (*p1 != '\0') {
        if (*p2 == '\0') return  1;
        if (*p2 > *p1)   return -1;
        if (*p1 > *p2)   return  1;

        p1++;
        p2++;
    }

    if (*p2 != '\0') return -1;

    return 0;
}

static char * findchar(const char* str, int c){
    const char* position = NULL;
    int i = 0;
    for(i = 0; ;i++) {
        if((unsigned char) str[i] == c) {
            position = &str[i];
            break;
        }
        if (str[i]=='\0') break;
    }
    return (char *) position;
}

static char * strcat(char* destination,char* source) {
 int c = 0;
 int sc;

 while(destination[c] != 0) {  c++; }

 for(sc = 0; sc < str_len(source); sc++) {
  destination[sc+c] = source[sc];
 }

 destination[sc+c] = 0;

 return destination;
}

static void memcpy(char *dest, char *src, int count)
{
    char *cdest = (char *) dest;
    char *csrc = (char *) src;

    int x = 0;
    while (x <= count)
    {
      cdest[x] = csrc[x];
      x++;
    }
    return;
}

static char * strtok(char * str, char *delimiter)
{
    static int pos;
    static char *s; 
    int start = pos;

    if (str != 0)
        s = str;

    int j = 0;
    while(s[pos] != '\0')
    {
        j = 0;  
        while(delimiter[j] != '\0')
        {       
            if(s[pos] == delimiter[j])
            {
                s[pos] = '\0';
                pos = pos+1;                
                if(s[start] != '\0')
                    return (&s[start]);
                else
                {
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

static char strpbrk(char * str, char * cmp)
{
    int l = str_len(str);
    int x = 0;
    int y = 0;
    
    while(x < l)
    {
        while(y < str_len(cmp)){
                if( str[x] == cmp[y]){return str[x];}
                y++;
        } y = 0;
        x++;
    } return '\0';
}

static void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

}
#endif