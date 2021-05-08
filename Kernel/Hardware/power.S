.section .text
.global shutdown
.type shutdown, @function
.global reboot
.type reboot, @function

shutdown:
    mov $0x5301, %ax
    xor %bx, %bx
    int $0x15

    /* Try to set apm version (to 1.2). */
    mov $0x530e, %ax
    xor %bx, %bx
    mov $0x0102, %cx
    int $0x15

    /* Turn off the system. */
    mov $0x5307, %ax
    mov $0x0001, %bx
    mov $0x0003, %cx
    int $0x15

    ret
