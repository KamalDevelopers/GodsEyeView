#define FAIL() for(;;);
extern long KERNEL_SIZ;
extern long KERNEL_ENTRY;
void trampoline();

 /* [sp+4]  char* message,  [sp+6] color */
void print(const char*, short c);
 /* [sp+4]  num sectors */
 /* [sp+6]  sector index */
 /* [sp+8]  memory location */
void disk_load(short, short, short);
void kernel_load();

void boot_main()
{
    print("[god's bootloader]:  ", 15);
    print("dl, ", 10);
    disk_load(1, 12, 0x1000);

    /* load kernel header */
    const char* magic = "gods";
    if ((((long*)magic)[0]) != ((long*)0x01000)[0]) {
        print("!fk ", 12);
        FAIL();
    }

    /* kernel size and entry stored in header */
    KERNEL_SIZ = ((long*)0x01000)[1];
    KERNEL_ENTRY = ((long*)0x01000)[2];

    print("fk, ", 10);
    if (KERNEL_SIZ < 1) {
        print("!kz ", 12);
        FAIL();
    }

    /* trampoline into 32 bit protected mode */
    print("tr, ", 10);
    trampoline();

    while (1)
        ;
    return;
}
