#include <stdio.h>
#include <unistd.h>

void cipher_char(int* c)
{
    *c = *c + 3;
    if (*c < 97) *c = *c + 26;
    if (*c > 97 + 26) *c = *c - 26;
}

int main() {
    int s, c;
    char* inp;
    inp = malloc(100);

    printf("= caesar cipher enter lowercase string =\n");
    s = read(0, inp, 100);

    while (s > 1) {
        s--;
        c = *inp;
        if (c != ' ') cipher_char(&c);
        printf("%c", c);
        inp++;
    }

    printf("\n");
    return 0;
}
