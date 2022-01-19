
.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager9interruptEhj


.macro exception num
.global _ZN16InterruptManager13exception\num\()Ev
_ZN16InterruptManager13exception\num\()Ev:
    movb $\num, (interruptnumber)
    jmp int_bottom
.endm


.macro request num
.global _ZN16InterruptManager11request\num\()Ev
_ZN16InterruptManager11request\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    pushl $0
    jmp int_bottom
.endm


exception 0x00
exception 0x01
exception 0x02
exception 0x03
exception 0x04
exception 0x05
exception 0x06
exception 0x07
exception 0x08
exception 0x09
exception 0x0A
exception 0x0B
exception 0x0C
exception 0x0D
exception 0x0E
exception 0x0F
exception 0x10
exception 0x11
exception 0x12
exception 0x13

request 0x00
request 0x01
request 0x02
request 0x03
request 0x04
request 0x05
request 0x06
request 0x07
request 0x08
request 0x09
request 0x0A
request 0x0B
request 0x0C
request 0x0D
request 0x0E
request 0x0F
request 0x31
request 0x80

int_bottom:

    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    pushl %esp
    push (interruptnumber)
    call _ZN16InterruptManager9interruptEhj
    mov %eax, %esp

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp

    add $4, %esp

.global _ZN16InterruptManager16interrupt_ignoreEv
_ZN16InterruptManager16interrupt_ignoreEv:

    iret


.data
    interruptnumber: .byte 0
