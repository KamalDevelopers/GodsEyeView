#define FAIL() for(;;);

extern long KERNEL_SIZ;
extern long KERNEL_ENTRY;

 /* [sp+4]  char* message,  [sp+6] color */
void print(const char*, short c);
 /* [sp+4]  num sectors */
 /* [sp+6]  sector index */
 /* [sp+8]  memory location */
void disk_load(short, short, short);
void trampoline();
/* stack sackers */
void e820_memory_map();
void vbe_init();
void kernel_load();

void boot_main()
{
    disk_load(1, 12, 0x1000);
    print("[+] Disk load sector #1 to 0x1000\n", 10);

    /* load kernel header */
    const char* magic = "gods";
    if ((((long*)magic)[0]) != ((long*)0x01000)[0]) {
        print("\xD[-] Could not find kernel header magic\n\xD", 12);
        FAIL();
    }

    /* kernel size and entry stored in header */
    KERNEL_SIZ = ((long*)0x01000)[1];
    KERNEL_ENTRY = ((long*)0x01000)[2];

    print("\xD[+] Found kernel header magic at sector #1\n\xD", 10);
    if (KERNEL_SIZ < 1) {
        print("\xD[-] Invalid kernel size\n\xD", 12);
        FAIL();
    }

    /* trampoline into 32 bit protected mode */
    print("\xD[+] Preparing for trampoline\n\xD", 10);

    /* stack sackers! */
    e820_memory_map();
    print("\xD[+] Stored memory map\n\xD", 10);
    vbe_init();
    kernel_load();
    trampoline();

    while (1)
        ;
    return;
}

/* halt enducing errors */
void err_verify_a20()
{
    print("\xD[-] A20 verification returned error\n\xD", 12);
    FAIL();
}

void err_e820()
{
    print("\xD[-] Memory mapping returned error\n\xD", 12);
    FAIL();
}

void err_kernel_load()
{
    print("\xD[-] Kernel load returned error\n\xD", 12);
    FAIL();
}

/* non halt enducing errors */
void err_vbe()
{
    print("\xD[-] VBE might not be supported, continuing\n\xD", 14);
}

void err_vbe2()
{
    print("\xD[-] VBE2 might not be supported, continuing\n\xD", 14);
}

void err_vbe_version()
{
    print("\xD[-] VBE version 0x200 not supported, continuing\n\xD", 14);
}
