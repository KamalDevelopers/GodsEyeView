#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>

void draw(char* buffer)
{
    int size = strlen(buffer);
    for (int i = 0; i < 32 - (size + 2) / 2; i++)
        printf(" ");

    for (int i = 0; i < size + 2; i++)
        printf("_");

    printf("\n");
    for (int i = 0; i < 31 - (size + 2) / 2; i++)
        printf(" ");

    printf("< %s >\n ", buffer);

    for (int i = 0; i < 31 - (size + 2) / 2; i++)
        printf(" ");

    for (int i = 0; i < size + 2; i++)
        printf("-");

    const char* img = "                                |\n"
                      "        _(,_/ \\ \\____________   |\n"
                      "        |`. \\_@_@   `.     ,'  -'\n"
                      "        |\\ \\ .        `-,-'\n"
                      "        || |  `-.____,-'\n"
                      "        || /  /\n"
                      "        |/ |  |\n"
                      "   `..     /   \\\n"
                      "     \\   /     |\n"
                      "     ||  |      \\\n"
                      "      \\ /-.     |\n"
                      "      ||/  /_   |\n"
                      "      \\(_____)-'_)\n";

    printf("\n%s", img);
}

int main(int argc, char** argv)
{
    const char* default_msg = "I'm Tony the dog!\0";
    if (argc && strlen(argv[0]) < 50)
        draw(argv[0]);
    else
        draw((char*)default_msg);

    return 0;
}