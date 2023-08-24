#include <LibC/stat.h>
#include <LibC/stdio.h>
#include <LibC/unistd.h>

int main(int argc, char** argv)
{
    if (!argc) {
        printf("No input file(s)\n");
        return 0;
    }

    for (uint32_t i = 0; i < argc; i++) {
        if (unlink(argv[i]) != -1) {
            if (i != 0)
                printf("\n");
            printf("Unlinking %s\n", argv[i]);
            continue;
        }

        if (i != 0)
            printf("\n");
        printf("File '%s' does not exist\n", argv[i]);
    }

    return 0;
}
