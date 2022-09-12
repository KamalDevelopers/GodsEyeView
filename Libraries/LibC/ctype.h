#ifndef CTYPE_H
#define CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

static int isdigit(int c)
{
    if (c >= '0' && c <= '9')
        return c;
    return 0;
}

static int isblank(int c)
{
    if (c == '\t' || c == ' ')
        return 1;
    return 0;
}

static int isalpha(int c)
{
    char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz";
    char* letter = alphabet;

    while (*letter != '\0' && *letter != c)
        ++letter;

    if (*letter)
        return 1;

    return 0;
}

static int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return ('a' + c - 'A');
    return c;
}

static int toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        return ('A' + c - 'a');
    return c;
}

#ifdef __cplusplus
}
#endif

#endif
