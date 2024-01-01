; set VESA vbe mode, OUT: vbe_screen structure

align 32
vbe_width           dw 1440 ; config vesa width
vbe_height          dw 900  ; config vesa height
vbe_bpp             dw 32

vbe_init:
    push es
    mov dword[vbe_info_block], "VBE2"
    mov ax, 0x4F00
    mov di, vbe_info_block
    int 0x10
    pop es

    cmp ax, 0x4F
    jne err_vbe2

    cmp dword[vbe_info_block], "VESA"
    jne .no_vbe

    cmp dword[vbe_info_block.version], 0x200
    jl err_vbe_version

    cmp ax, 0x4F
    jne .no_vbe

    ; FIXME: find closest mode?
    mov ax, [vbe_width]
    mov bx, [vbe_height]
    mov cl, [vbe_bpp]

    call vbe_set_mode
    ret
    jc .no_vbe
    ret

.no_vbe:
    ; halt on fail
    jmp err_vbe
    jmp .no_vbe


vbe_set_mode:
    mov [.width], ax
    mov [.height], bx
    mov [.bpp], cl

    ; sti
    push es 
    ; VBE BIOS info
    mov ax, 0x4F00
    mov di, vbe_info_block
    int 0x10
    pop es

    ; does BIOS support VBE
    cmp ax, 0x4F 
    jne .vbe_error

    mov ax, word[vbe_info_block.video_modes]
    mov [.offset], ax
    mov ax, word[vbe_info_block.video_modes+2]
    mov [.segment], ax

    mov ax, [.segment]
    mov fs, ax
    mov si, [.offset]
 
.vbe_find_mode:
    mov dx, [fs:si]
    add si, 2
    mov [.offset], si
    mov [.mode], dx
    mov ax, 0
    mov fs, ax

    mov eax, [.mode]
    cmp eax, 0xFFFF ; eol 
    je .vbe_error

    ; VBE mode info
    push es
    mov ax, 0x4F01
    mov cx, [.mode]
    mov di, mode_info_block
    int 0x10
    pop es

    cmp ax, 0x4F
    jne .vbe_error

    ; find desired width
    mov ax, [.width]
    cmp ax, [mode_info_block.width]
    jne .vbe_next_mode

    ; find desired height
    mov ax, [.height]
    cmp ax, [mode_info_block.height]
    jne .vbe_next_mode

    ; find desired bpp
    mov al, [.bpp]
    cmp al, [mode_info_block.bpp]
    jne .vbe_next_mode

    ; set mode
    push es
    mov ax, 0x4F02
    mov bx, [.mode]
    or bx, 0x4000
    mov di, 0
    int 0x10
    pop es

    cmp ax, 0x4F
    jne .vbe_error

    ; save mode width & height
    mov ax, [vbe_width]
    mov word[vbe_screen.width], ax
    mov ax, [vbe_height]
    mov word[vbe_screen.height], ax

    ; save mode framebuffer
    mov eax, [mode_info_block.framebuffer]
    mov dword[vbe_screen.framebuffer], eax

    ; save mode pitch
    mov ax, [mode_info_block.pitch]
    mov word[vbe_screen.bytes_per_line], ax

    ; save mode bpp
    mov eax, 0
    mov al, [.bpp]
    mov byte[vbe_screen.bpp], al
    shr eax, 3
    mov dword[vbe_screen.bytes_per_pixel], eax

    ; save mode max x & max y
    mov ax, [.width]
    shr ax, 3
    dec ax
    mov word[vbe_screen.x_max], ax
    mov ax, [.height]
    shr ax, 4
    dec ax
    mov word[vbe_screen.y_max], ax

    clc
    ret
     
.vbe_next_mode:
    mov ax, [.segment]
    mov fs, ax
    mov si, [.offset]
    jmp .vbe_find_mode
 
.vbe_error:
    jmp err_vbe
    ret

.width       dw 0
.height      dw 0
.bpp         db 0
.segment     dw 0
.offset      dw 0
.mode        dw 0

align 16
vbe_info_block:
.signature           db "VBE2"
.version             dw 0
.oem                 dd 0
.capabilities        dd 0
.video_modes         dd 0
.memory              dw 0
.software_rev        dw 0
.vendor              dd 0
.product_name        dd 0
.product_rev         dd 0
.reserved: times 222 db 0
.oem_data: times 256 db 0

align 16
mode_info_block:
.attributes          dw 0
.window_a            db 0
.window_b            db 0
.granularity         dw 0
.window_size         dw 0
.segmentA            dw 0
.segmentB            dw 0
.win_func_ptr        dd 0
.pitch               dw 0

.width               dw 0
.height              dw 0

.w_char              db 0
.y_char              db 0
.planes              db 0
.bpp                 db 0
.banks               db 0

.memory_model        db 0
.bank_size           db 0
.image_pages         db 0

.reserved0           db 0

.red                 dw 0
.green               dw 0
.blue                dw 0
.reserved_mask       dw 0
.direct_color        db 0

.framebuffer         dd 0
.off_screen_mem      dd 0
.off_screen_mem_size dw 0
.reserved1:times 206 db 0

align 16
align 32
vbe_screen:
.width               dd 0
.height              dd 0
.bpp                 dd 0
.bytes_per_pixel     dd 0
.bytes_per_line      dd 0
.screen_size         dd 0
.screen_size_dqwords dd 0
.framebuffer         dd 0
.x                   dd 0
.y                   dd 0
.x_max               dd 0
.y_max               dd 0
