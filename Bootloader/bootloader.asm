
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
