[BITS 16]
[ORG 0x7E00]
; first instruction adress

; bootloader begin
.start:

mov	sp, 0x9c00
; BIOS boot drive in dl
mov [BOOT_DRIVE], dl

call verify_a20
call boot_main ; call C bootloader

trampoline:
    call e820_memory_map
    call vbe_init
    call kernel_load

    ; set multiboot header
    mov eax, [vbe_screen.width]
    mov [multiboot.vesa_width], eax
    mov eax, [vbe_screen.height]
    mov [multiboot.vesa_height], eax 
    mov eax, [vbe_screen.bpp]
    mov dword [multiboot.vesa_bpp], eax
    mov eax, [vbe_screen.bytes_per_line]
    mov dword [multiboot.vesa_pitch], eax
    mov eax, [vbe_screen.framebuffer]
    mov dword [multiboot.vesa_framebuffer], eax
    mov eax, [total_continuous_memory]
    mov dword [multiboot.upper_memory], eax

    ; set the ds register
    cli
    xor ax, ax
    mov ds, ax

    ; load segments table
    lgdt [gtd_desc]

    ; go to protected mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; go to 32-bit code
    jmp 0x08:trampoline32

; serial print using BIOS VGA
print:
    push bp
    mov bp, sp

    mov si, [bp+4]
    mov bl, [bp+6]
	mov ah, 0x0E
.print_repeat:
	lodsb
	int 0x10
	cmp al, 0
	jne .print_repeat

    pop bp
    ret

; verify a20 line else hang
verify_a20:
	pushf
	push si
	push di
	push ds
	push es
	cli
 
	mov ax, 0x0000
	mov ds, ax
	mov si, 0x0500
 
	not ax
	mov es, ax
	mov di, 0x0510
 
	mov al, [ds:si]
	mov byte [.belowmb], al
	mov al, [es:di]
	mov byte [.overmb], al
 
	mov ah, 1
	mov byte [ds:si], 0
	mov byte [es:di], 1
	mov al, [ds:si]
	cmp al, [es:di]
	jne .exit
	dec ah
.exit:
	mov al, [.belowmb]
	mov [ds:si], al
	mov al, [.overmb]
	mov [es:di], al
	shr ax, 8
	; sti
	pop es
	pop ds
	pop di
	pop si
	popf

    cmp ax, 0
    jne .final
    jmp $

.final:
	ret
 
	.belowmb: db 0
	.overmb: db 0

; standard read from disk using BIOS
disk_load:
disk_load__:
    push bp
    mov bp, sp
    pusha

    mov dl, [BOOT_DRIVE]
    mov cl, [bp+6]        ; sector index
    mov ah, 0x02          ; BIOS read
    mov bx, [bp+8]        ; memory location
    mov al, [bp+4]        ; number of sectors
    mov dh, 0
    mov ch, 0
    int 0x13

    or ah, ah             ; error flag
    jnz disk_load__       ; failed = hang

    cmp al, [bp+4]        ; how many sectors read?
    jne disk_load__       ; failed = hang

    popa
    pop bp
    ret

; LBA extended disk read using BIOS LBA
kernel_load:
    pusha
    mov eax, [KERNEL_SIZ]
    mov [KERNEL_SIZ], eax

kernel_load__:
    ; read 1 sector at a time
    mov ax, 1
    mov [K_DAP.sector_count], ax

	mov ah, 0x42		  ; al is unused
	mov al, 0x42		  ; al is unused
	mov dl, [BOOT_DRIVE]  ; drive number 0 (OR the drive # with 0x80)
    mov si, K_DAP         ; address of "disk address packet"
	int 0x13
	jc .error
    cmp ah, 0
    jne .error

    ; relocate single sector
    call kernel_relocate

    ; increment sector index
    mov eax, [K_DAP.lower_lba]
    inc eax
    mov [K_DAP.lower_lba], eax

    ; decrement kernel size and loop
    mov eax, [KERNEL_SIZ]
    dec eax
    mov [KERNEL_SIZ], eax
    test eax, eax
    jne kernel_load__

    popa
    ret
.error:
    jmp $

kernel_relocate:
	 ; number of dwords to move [512/4]
     mov ecx, 128
	.relocation_loop_start__:
		mov edx, dword [KERNEL_ADDRESS]
		mov ebx, 0x1000
	.relocation_loop__:
		mov eax, dword [ebx]
		mov dword [edx], eax
		add ebx, 4
		add edx, 4

        mov eax, 0
        mov dword [edx], eax

		loop .relocation_loop__
	
	mov dword [KERNEL_ADDRESS], edx
    ret

; =================================================================== ;
; disk address packet format:                                         ;
;                                                                     ;
; Offset | Size | Desc                                                ;
;      0 |    1 | Packet size                                         ;
;      1 |    1 | Zero                                                ;
;      2 |    2 | Sectors to read/write                               ;
;      4 |    4 | transfer-buffer 0xffff:0xffff                       ;
;      8 |    4 | lower 32-bits of 48-bit starting LBA                ;
;     12 |    4 | upper 32-bits of 48-bit starting LBAs               ;
; =================================================================== ;
K_DAP:
.size:
    db 	0x10
.zero:
    db 	0x00
.sector_count:
    dw 	0x0000
.transfer_buffer:
    dw 	0x1000          ; temporary location
.transfer_buffer_seg:
    dw 	0x0
.lower_lba:
    dd 	0xB             ; sector index 11
.higher_lba:
    dd 	0x00000000


BOOT_DRIVE db 0
KERNEL_SIZ dd 0
VESA_LOADED db 0
KERNEL_ENTRY dd 0
KERNEL_ADDRESS dd 0x0100000

[BITS 32]
; support for 32-bit trampoline
trampoline32:
    ; set segment registers
    mov 	ax, 0x10
    mov 	es, ax
    mov 	fs, ax
    mov 	ds, ax
    mov 	gs, ax
    mov 	ss, ax
    mov esp, 0x090000 ; set up stack pointer

    mov byte [0xB8000], 88
    mov byte [0xB8000+1], 0x1B

    mov ebx, multiboot
    call dword [KERNEL_ENTRY]

    mov byte [0xB8000+4], 89
    mov byte [0xB8000+5], 0x1B

    jmp $

[BITS 64]
; support for 64-bit trampoline
trampoline64:
    ; set segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    mov esp, 0x090000 ; set up stack pointer

    mov byte [0xB8000], 88
    mov byte [0xB8000+1], 0x1B

    call qword [KERNEL_ENTRY] ; go to C code

    mov byte [0xB8000+4], 89
    mov byte [0xB8000+5], 0x1B
    jmp $


[BITS 16]

multiboot:
    .reserv            dd 0
	.vesa_width        dd 0
	.vesa_height       dd 0
	.vesa_bpp          dd 0
	.vesa_pitch        dd 0
	.vesa_framebuffer  dd 0
    .upper_memory      dd 0

; vesa BIOS functions
%include "vesa.asm"
%include "memory.asm"

; Disassembly of file: bootloader.o
; Sun Nov  5 19:50:42 2023
; Type: ELF32
; Syntax: NASM
; Instruction set: 80386
boot_main:; Function begin
        push    ebp                                     ; 0000 _ 55
        mov     ebp, esp                                ; 0001 _ 89. E5
        sub     esp, 24                                 ; 0003 _ 83. EC, 18
        sub     esp, 8                                  ; 0006 _ 83. EC, 08
        push    15                                      ; 0009 _ 6A, 0F
        push    ?_006                                   ; 000B _ 68, 00000000(d)
        call    print                                   ; 0010 _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 0015 _ 83. C4, 10
        sub     esp, 8                                  ; 0018 _ 83. EC, 08
        push    10                                      ; 001B _ 6A, 0A
        push    ?_007                                   ; 001D _ 68, 00000016(d)
        call    print                                   ; 0022 _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 0027 _ 83. C4, 10
        sub     esp, 4                                  ; 002A _ 83. EC, 04
        push    4096                                    ; 002D _ 68, 00001000
        push    12                                      ; 0032 _ 6A, 0C
        push    1                                       ; 0034 _ 6A, 01
        call    disk_load                               ; 0036 _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 003B _ 83. C4, 10
        mov     dword [ebp-0CH], ?_008                  ; 003E _ C7. 45, F4, 0000001B(d)
        mov     eax, dword [ebp-0CH]                    ; 0045 _ 8B. 45, F4
        mov     edx, dword [eax]                        ; 0048 _ 8B. 10
        mov     eax, 4096                               ; 004A _ B8, 00001000
        mov     eax, dword [eax]                        ; 004F _ 8B. 00
        cmp     edx, eax                                ; 0051 _ 39. C2
        jz      ?_002                                   ; 0053 _ 74, 14
        sub     esp, 8                                  ; 0055 _ 83. EC, 08
        push    12                                      ; 0058 _ 6A, 0C
        push    ?_009                                   ; 005A _ 68, 00000020(d)
        call    print                                   ; 005F _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 0064 _ 83. C4, 10
?_001:  jmp     ?_001                                   ; 0067 _ EB, FE
; boot_main End of function
?_002:  ; Local function
        mov     eax, 4100                               ; 0069 _ B8, 00001004
        mov     eax, dword [eax]                        ; 006E _ 8B. 00
        mov     dword [KERNEL_SIZ], eax                 ; 0070 _ A3, 00000000(d)
        mov     eax, 4104                               ; 0075 _ B8, 00001008
        mov     eax, dword [eax]                        ; 007A _ 8B. 00
        mov     dword [KERNEL_ENTRY], eax               ; 007C _ A3, 00000000(d)
        sub     esp, 8                                  ; 0081 _ 83. EC, 08
        push    10                                      ; 0084 _ 6A, 0A
        push    ?_010                                   ; 0086 _ 68, 00000025(d)
        call    print                                   ; 008B _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 0090 _ 83. C4, 10
        mov     eax, dword [KERNEL_SIZ]                 ; 0093 _ A1, 00000000(d)
        test    eax, eax                                ; 0098 _ 85. C0
        jg      ?_004                                   ; 009A _ 7F, 14
        sub     esp, 8                                  ; 009C _ 83. EC, 08
        push    12                                      ; 009F _ 6A, 0C
        push    ?_011                                   ; 00A1 _ 68, 0000002A(d)
        call    print                                   ; 00A6 _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 00AB _ 83. C4, 10
?_003:  jmp     ?_003                                   ; 00AE _ EB, FE
?_004:  ; Local function
        sub     esp, 8                                  ; 00B0 _ 83. EC, 08
        push    10                                      ; 00B3 _ 6A, 0A
        push    ?_012                                   ; 00B5 _ 68, 0000002F(d)
        call    print                                   ; 00BA _ E8, FFFFFFFC(rel)
        add     esp, 16                                 ; 00BF _ 83. C4, 10
        call    trampoline                              ; 00C2 _ E8, FFFFFFFC(rel)
?_005:  jmp     ?_005                                   ; 00C7 _ EB, FE
?_006:                                                  ; byte
        db 5BH, 67H, 6FH, 64H, 27H, 73H, 20H, 62H       ; 0000 _ [god's b
        db 6FH, 6FH, 74H, 6CH, 6FH, 61H, 64H, 65H       ; 0008 _ ootloade
        db 72H, 5DH, 3AH, 20H, 20H, 00H                 ; 0010 _ r]:  .
?_007:                                                  ; byte
        db 64H, 6CH, 2CH, 20H, 00H                      ; 0016 _ dl, .
?_008:                                                  ; byte
        db 67H, 6FH, 64H, 73H, 00H                      ; 001B _ gods.
?_009:                                                  ; byte
        db 21H, 66H, 6BH, 20H, 00H                      ; 0020 _ !fk .
?_010:                                                  ; byte
        db 66H, 6BH, 2CH, 20H, 00H                      ; 0025 _ fk, .
?_011:                                                  ; byte
        db 21H, 6BH, 7AH, 20H, 00H                      ; 002A _ !kz .
?_012:                                                  ; byte
        db 74H, 72H, 2CH, 20H, 00H                      ; 002F _ tr, .
; gdt
gdt:

gdt_null:
    dd 0
    dd 0

gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gtd_desc:
    dw gdt_end - gdt - 1
    dd gdt

; Make sure that size is 10 sectors
TIMES (512 * 10) - ($ - $$) db 0
