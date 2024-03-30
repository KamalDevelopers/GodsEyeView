.extern __kernel_size
.global loader

.section .multiboot
    .ascii "gods"
    .long __kernel_size
    .long loader

.section .text
.extern kernel_main 
.extern call_constructors

.extern sbss
.extern ebss

.extern multiboot_info_ptr
.type multiboot_info_ptr, @object

loader:
    movl %ebx, multiboot_info_ptr
    cli
    cld

.enable_sse:
    mov %cr0, %eax
    and $0xFFFB, %ax
    or $0x2, %ax
    mov %eax, %cr0
    mov %cr4, %eax
    or $0x600, %ax
    mov %eax, %cr4


    mov sbss, %ebx


.zloop:
    movb $0, (%ebx)
    inc %ebx
    cmp ebss, %ebx
    jb .zloop


    pushl %eax
    pushl %ebx

    mov $kernel_stack, %esp
    call call_constructors
    pushl %eax
    pushl %ebx
    call kernel_main 


_stop:
    cli
    hlt
    jmp _stop


.section .bss
.space 2*1024*1024; # 2 MiB
kernel_stack:
