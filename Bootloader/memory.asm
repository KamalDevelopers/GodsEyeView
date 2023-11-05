; BIOS e820 calculate continuous usable memory area

e820_memory_map:
    mov di, e820_map_structure
	xor ebx, ebx
	xor bp, bp
	mov edx, 0x0534D4150
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15

	jc short .e820_failed
	mov edx, 0x0534D4150
	cmp eax, edx
	jne short .e820_failed
	test ebx, ebx
	je short .e820_failed
	jmp short .e820_

; loop next memory region
.e820lp:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short .e820_final
	mov edx, 0x0534D4150

.e820_:
	jcxz .e820_skip
	cmp cl, 20
	jbe short .e820_notext
	test byte [es:di + 20], 1
	je short .e820_skip

.e820_notext:
	mov ecx, [es:di + 8]
	or ecx, [es:di + 12]
	jz .e820_skip
	inc bp

    ; check acpi flag, bit 1
    mov eax, [e820_map_structure.acpi]
    and eax, 0x1
    cmp eax, 1
    ; disregard this memory area
    jne .e820_clear_skip

    ; check type flag, bit 1
    mov eax, [e820_map_structure.type]
    and eax, 0x1
    cmp eax, 1
    ; disregard this memory area
    jne .e820_clear_skip

    ; TODO: support length_high
    mov eax, [total_memory]
    mov ecx, [e820_map_structure.length_low]
    add eax, ecx
    mov [total_memory], eax

.e820_skip:
	test ebx, ebx
	jne short .e820lp
.e820_final:
	ret
; halt on fail
.e820_failed:
    jmp .e820_failed

.e820_clear_skip:
    mov eax, [total_continuous_memory]
    cmp dword [total_memory], eax
    jbe .e820_skip_
    mov eax, [total_memory]
    mov dword [total_continuous_memory], eax
.e820_skip_:
    mov dword [total_memory], 0
	test ebx, ebx
	jne .e820lp
    ret

; total size of continuous memory area
total_continuous_memory:
    dd 0
; total size of memory area
total_memory:
    dd 0

; e820 defined structure
; about: http://www.uruk.org/orig-grub/mem64mb.html
e820_map_structure:
.base_addr_low:
    dd 0
.base_addr_high:
    dd 0
.length_low:
    dd 0
.length_high:
    dd 0
.type:
    dd 0
.acpi:
    dd 0
