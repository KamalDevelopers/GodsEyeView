#ifndef STRING_H
#define STRING_H

#include "ctype.h"
#include "types.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int len(const char* arr);
extern size_t strlen(const char* str);

extern size_t strspn(const char* str1, const char* str2);
extern char* strcpy(char* s1, const char* s2);
extern char* strncpy(char* arr, const char* str, int l);
extern int strcmp(const char* s1, const char* s2);
extern int strncmp(const char* s1, const char* s2, int count);
extern char* strcat(char* dest, const char* src);
extern char* strtok(char* str, const char* delimiter);
extern const char* strstr(const char* str1, const char* str2);
extern char* strchr(const char *s, int c);
extern char strpbrk(char* str, const char* cmp);
extern float stof(const char* str);
extern void uppercase(char* str);
extern void lowercase(char* str);

extern char* findchar(const char* str, int c);
extern void int_to_ascii(int n, char str[]);

#ifdef __cplusplus
}
#endif

#endif
