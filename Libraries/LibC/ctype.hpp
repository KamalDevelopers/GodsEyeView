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

#endif
