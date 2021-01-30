#ifndef CTYPE_HPP
#define CTYPE_HPP

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
    char *letter = alphabet;

    while(*letter != '\0' && *letter != c)
        ++letter;

    if (*letter)
        return 1;

    return 0;
}

int tolower(int c)
{
    if(c >= 'A' && c <= 'Z')
    	return ('a' + c - 'A');
    else
    	return c;
}

int toupper(int c)
{
        if(c >= 'a' && c <= 'z')
                return ('A' + c - 'a');
        else
                return c;
}

#endif
