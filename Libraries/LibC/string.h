#ifndef STRING_H
#define STRING_H

#include "ctype.h"
#include "mem.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t strlen(const char* s);
char* strchrnul(const char* s, int c_in);
size_t strspn(const char* s1, const char* s2);
size_t strcspn(const char* s1, const char* s2);
char* strcpy(char* s1, const char* s2);
char* strncpy(char* s1, const char* s2, int l);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int l);
int strncasecmp(const char* s1, const char* s2, int l);
int strcasecmp(const char* s1, const char* s2);
char* strcat(char* dest, const char* src);
char* strtok(char* s, const char* sep);
const char* strstr(const char* s1, const char* s2);
char* strchr(const char* s, int c);
char strpbrk(char* s, const char* cmp);
float stof(const char* s);
void uppercase(char* s);
void lowercase(char* s);
char* findchar(const char* s, int c);
void itos(int n, char s[]);

#ifdef __cplusplus
}
#endif

#endif
