#include <LibC/stdio.h>
#include <LibC/stdlib.h>
#include <LibC/string.h>

void tonysay(char* buffer)
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
    const char* default_msg = "I'm Tony the dog!";
    char msg[50];
    memset(msg, 0, 50);

    if (!argc) {
        tonysay((char*)default_msg);
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if ((strlen(argv[i]) + strlen(msg)) > 50)
            break;
        strcat(msg, argv[i]);
        if (i != argc - 1)
            strcat(msg, " ");
    }

    tonysay(msg);
    return 0;
}
