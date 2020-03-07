int len(const char *format)
{
    int index = 0;
    while (format[index] != '\0')
    {
        index++;
    }
    return index;
}