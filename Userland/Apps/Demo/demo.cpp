//static const char* text = "Hello GevOS! (^:";

int main()
{

    int result;
    char buffer[22];

    /* syscall 5: open() */
    asm volatile("int $0x80"
                 : "=a"(result)
                 : "a"(5), "b"("root/welcome"));

    /* syscall 3: read() */
    asm volatile("int $0x80"
                 :
                 : "a"(3), "b"(result), "c"(buffer), "d"(22));

    /* syscall 6: close() */
    asm volatile("int $0x80"
                 :
                 : "a"(6), "b"(result));

    /* syscall 4: write() */
    asm volatile("int $0x80"
                 :
                 : "a"(4), "b"(1), "c"(buffer), "d"(22));

    /* syscall 1: exit() */
    asm volatile("xor %eax, %eax\n");
    asm volatile("int $0x80"
                 :
                 : "a"(1), "b"(0));

    while (1)
        ;
}
