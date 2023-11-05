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
