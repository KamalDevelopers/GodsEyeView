static char *text = "Hello GevOS (^:";
void main()
{
        /* syscall 4: write()      eax=4  ebx=file ecx=buffer edx=size RET: ecx=size*/
	asm volatile("int $0x80": :"a"(4), "b"(1), "c"(text));
	/* syscall 1: exit()       eax=1 ebx=err */
	asm volatile("xor %eax, %eax\n");
	asm volatile("int $0x80": :"a"(1), "b"(0));
	while(1);
}
