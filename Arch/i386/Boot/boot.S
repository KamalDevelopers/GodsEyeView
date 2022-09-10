.set MULTIBOOT_MAGIC,         0x1badb002
.set MULTIBOOT_PAGE_ALIGN,    0x1
.set MULTIBOOT_MEMORY_INFO,   0x2
.set MULTIBOOT_VIDEO_MODE,    0x4  /* 4 for graphics */
.set multiboot_flags,         MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE
.set multiboot_checksum,      -(MULTIBOOT_MAGIC + multiboot_flags)

.section .multiboot
    .align 4
    .long MULTIBOOT_MAGIC
    .long multiboot_flags
    .long multiboot_checksum

    .long 0x00000000    /* header_addr */
    .long 0x00000000    /* load_addr */
    .long 0x00000000    /* load_end_addr */
    .long 0x00000000    /* bss_end_addr */
    .long 0x00000000    /* entry_addr */

    .long 0x00000000    /* mode_type */
    .long 1920          /* width */
    .long 1080          /* height */
    .long 32            /* depth */


.section .text
.extern kernel_main 
.extern call_constructors
.global loader

.extern multiboot_info_ptr
.type multiboot_info_ptr, @object

loader:
    cli
    cld

    pushl %eax
    pushl %ebx
    movl %ebx, multiboot_info_ptr

    mov $kernel_stack, %esp
    call call_constructors
    call kernel_main 


_stop:
    cli
    hlt
    jmp _stop


.section .bss
.space 2*1024*1024; # 2 MiB
kernel_stack:
