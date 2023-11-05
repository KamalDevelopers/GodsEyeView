.section .text
.global tss_flush
.type tss_flush, @function
.global gdt_flush
.type gdt_flush, @function

tss_flush:
    mov $0x28, %ax
    ltr %ax
    ret

gdt_flush:
    add $4, %esp
    mov (%esp), %eax
    lgdt (%eax)
    sub $4, %esp

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    jmp $0x08,$flush 
flush:
    ret
