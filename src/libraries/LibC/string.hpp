#ifndef STRING_HPP
#define STRING_HPP
#include "types.hpp"

extern int len(const char* arr);
extern int str_len(char arr[]);
extern size_t strlen(char* str);

extern size_t strspn(char* str1, char* str2);
extern char* strcpy(char* arr, char* str);
extern char* strncpy(char* arr, const char* str, int l);
extern int strcmp(const char* s1, const char* s2);
extern int strncmp(const char* s1, const char* s2, int count);
extern char* strcat(char* dest, char* src);
extern char* strtok(char* str, char* delimiter);
extern char strpbrk(char* str, char* cmp);

extern char* findchar(const char* str, int c);
extern void memcpy(char* dest, char* src, int count);
extern void int_to_ascii(int n, char str[]);

#endif
