; bootloader first stage, load rest of bootloader
[BITS 16]
[ORG 0x7c00]
; 0x7c00 first instruction adress

; Bootloader first stage
; begins here.
; dl is set by BIOS
.start:
xor ax, ax
mov ds, ax
mov ss, ax
mov [BOOT_DRIVE], dl      ; save ident of boot drive

reset_screen:             ; BIOS VGA screen clear
    mov ah, 0
    mov al, 0x10
    int 0x10

reset_drive:
    mov ah, 0
    int 0x13
    or ah, ah
    jnz reset_drive

; By entering unreal mode
; we can access memory beyond
; the normal ~1mb scope
; and correcly relocate
; the kernel.
enable_unreal_mode:
   xor ax, ax
   mov ds, ax
   mov ss, ax
   mov sp, 0x9c00         ; 2000h past code start, 
                          ; making the stack 7.5k in size
   cli
   push ds                ; save real mode
   lgdt [gdtinfo]
   mov  eax, cr0
   or al, 1
   mov  cr0, eax
   jmp 0x8:pmode
pmode:
   mov bx, 0x10           ; select descriptor 2
   mov ds, bx             ; 10h = 10000b
   and al, 0xFE           
   mov cr0, eax
   jmp 0x0:unreal         ; return to real mode
unreal:
    pop ds                ; get back old segment
    ; sti
    mov bx, 0x0f01
    mov eax, 0x0b8000
    mov word [ds:eax], bx
    cli

; enable a20 line by ioee
; FIXME: add more a20 methods
enable_a20_ioee:
	push	bp
	mov	bp, sp
	in	al, 0xee
	mov	sp, bp
	pop	bp

call stage_load

mov dl, [BOOT_DRIVE]
jmp dword 0x7e00

stage_load:
stage_load__:
    mov dl, [BOOT_DRIVE]
    mov cl, 2             ; sector index
    mov ah, 0x02          ; BIOS read
    mov bx, 0x7e00        ; memory location
    mov al, 10            ; number of sectors [10]
    mov dh, 0
    mov ch, 0
    int 0x13

    or ah, ah             ; error flag
    jnz stage_load__      ; failed = hang

    cmp al, 10            ; how many sectors?
    jne stage_load__      ; failed = hang

    ret

BOOT_DRIVE db 0
gdtinfo:
   dw gdt_end - gdt - 1   ; last byte in table
   dd gdt                 ; start of table
gdt:        dd 0,0        ; entry 0 is always unused
codedesc:   db 0xff, 0xff, 0, 0, 0, 10011010b, 00000000b, 0
flatdesc:   db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:



TIMES 510 - ($ - $$) db 0 ; Fill the rest of sector with 0
DW 0xaa55 ; Add boot signature at the end of bootloader
