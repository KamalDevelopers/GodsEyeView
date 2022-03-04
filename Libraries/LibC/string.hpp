#ifndef STRING_HPP
#define STRING_HPP

#include "ctype.hpp"
#include "types.hpp"

int len(const char* arr);
size_t strlen(const char* str);

size_t strspn(char* str1, char* str2);
char* strcpy(char* arr, char* str);
char* strncpy(char* arr, const char* str, int l);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int count);
char* strcat(char* dest, char* src);
char* strtok(char* str, char* delimiter);
char strpbrk(char* str, char* cmp);
float stof(const char* str);
void* memchr(const void* str, int c, size_t n);
void uppercase(char* str);
void lowercase(char* str);

char* findchar(const char* str, int c);
void memcpy(char* dest, char* src, int count);
void int_to_ascii(int n, char str[]);

#endif
