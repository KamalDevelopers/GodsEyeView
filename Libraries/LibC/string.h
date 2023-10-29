#ifndef STRING_H
#define STRING_H

#include "ctype.h"
#include "mem.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char* str);

char* strchrnul(const char* s, int c_in);
size_t strspn(const char* str1, const char* str2);
size_t strcspn(const char* str1, const char* str2);
char* strcpy(char* s1, const char* s2);
char* strncpy(char* arr, const char* str, int l);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int count);
char* strcat(char* dest, const char* src);
char* strtok(char* str, const char* sep);
const char* strstr(const char* str1, const char* str2);
char* strchr(const char* s, int c);
char strpbrk(char* str, const char* cmp);
float stof(const char* str);
void uppercase(char* str);
void lowercase(char* str);

char* findchar(const char* str, int c);
void itos(int n, char str[]);

#ifdef __cplusplus
}
#endif

#endif
