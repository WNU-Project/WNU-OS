global start

section .text
bits 64

; WNU SERVER 1.0.0 (BASED ON WNU OS 1.0.1 UPDATE 2) (NO GUI)
; KERNEL
; 64 BITS ARE ONLY SUPPORTED
; THE BOOTLADER IS GRUB

start:
    ; Init stack
    mov rsp, stack_top

    ; Optional short delay (nicer to CPU with pause)
    mov rcx, 200000
.boot_delay:
    pause
    loop .boot_delay

    ; Clear VGA text screen (80x25 = 2000 chars -> 4000 bytes)
    mov rdi, 0xB8000
    mov rcx, 2000
.clear_screen:
    mov byte [rdi], ' '
    mov byte [rdi+1], 0x0E
    add rdi, 2
    dec rcx
    jnz .clear_screen
    ; Reset cursor to top-left
    mov rdi, 0xB8000

    ; --- Banner ---
    mov rsi, banner
    mov rdi, 0xB8000
    mov rcx, banner_len
.print_banner:
    mov al, [rsi]
    cmp al, 0x0A              ; LF newline
    je .banner_newline
    mov [rdi], al             ; char
    mov byte [rdi+1], 0x0E    ; yellow on black
    add rdi, 2
    inc rsi
    loop .print_banner
    jmp .after_banner
.banner_newline:
    add rdi, 160
    cmp rdi, 0xB8000 + 4000
    jb .ok_b
    mov rdi, 0xB8000
.ok_b:
    inc rsi
    loop .print_banner
.after_banner:

    ; Advance to next line after banner
    add rdi, 160
    cmp rdi, 0xB8000 + 4000
    jb .ok_after_banner
    mov rdi, 0xB8000
.ok_after_banner:

    ; --- Prompt ---
    mov rsi, prompt
    mov rcx, prompt_len
.print_prompt:
    mov al, [rsi]
    cmp al, 0x0A
    je .prompt_newline
    mov [rdi], al
    mov byte [rdi+1], 0x0E
    add rdi, 2
    inc rsi
    loop .print_prompt
    jmp .prompt_done
.prompt_newline:
    add rdi, 160
    cmp rdi, 0xB8000 + 4000
    jb .ok_p
    mov rdi, 0xB8000
.ok_p:
    inc rsi
    loop .print_prompt
.prompt_done:

    ; Cursor after prompt
    mov rbx, rdi
    mov byte [input_pos], 0

    ; --- Long mode capability check (correct order) ---
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    bt edx, 29            ; LM bit
    jnc .no_long_mode

    ; --- Input loop ---
.input_loop:
    in  al, 0x64          ; status
    test al, 1
    jz .input_loop

    in  al, 0x60          ; scancode
    cmp al, 0x80
    jae .input_loop       ; ignore releases
    movzx rsi, al

    ; Backspace (0x0E)
    cmp rsi, 0x0E
    jne .not_backspace
    mov al, [input_pos]
    test al, al
    jz .input_loop
    dec byte [input_pos]
    sub rbx, 2
    mov byte [rbx], ' '
    mov byte [rbx+1], 0x0E
    jmp .input_loop
.not_backspace:

    ; Map scancode -> ASCII
    mov rcx, scancode_table_len
    mov rdx, scancode_table
.find_scancode:
    test rcx, rcx
    jz .input_loop            ; not found
    mov bl, byte [rdx]
    cmp bl, sil
    je .found_key
    add rdx, 2
    dec rcx
    jmp .find_scancode
.found_key:
    mov al, byte [rdx+1]
    test al, al
    jz .input_loop            ; non-printable

    ; Echo and buffer
    movzx rcx, byte [input_pos]
    mov [input_buffer + rcx], al
    inc byte [input_pos]

    ; Handle Enter (ASCII 0x0D)
    cmp al, 0x0D
    je .do_enter

    ; Normal char echo
    mov byte [rbx], al
    mov byte [rbx+1], 0x0E
    add rbx, 2
    jmp .input_loop

.do_enter:
    ; Move to next line
    add rbx, 160
    cmp rbx, 0xB8000 + 4000
    jb .ok_enter
    mov rbx, 0xB8000
.ok_enter:

    ; Reprint prompt
    mov rsi, prompt
    mov rcx, prompt_len
.print_prompt2:
    mov al, [rsi]
    cmp al, 0x0A
    je .prompt2_newline
    mov [rbx], al
    mov byte [rbx+1], 0x0E
    add rbx, 2
    inc rsi
    loop .print_prompt2
    jmp .after_prompt2
.prompt2_newline:
    add rbx, 160
    cmp rbx, 0xB8000 + 4000
    jb .ok_p2
    mov rbx, 0xB8000
.ok_p2:
    inc rsi
    loop .print_prompt2
.after_prompt2:
    mov byte [input_pos], 0
    jmp .input_loop

.no_long_mode:
    ; Minimal error indicator (prints 'L' at current cursor)
    mov byte [rdi], 'L'
    mov byte [rdi+1], 0x0E
    add rdi, 2
    ret

; --- (Optional) page tables + paging enable if transitioning) ---
; Note: Only use this if you are not already in long mode
; and have identity-mapped 0xB8000 and your code region.
setup_page_tables:
    ; L4[0] -> L3
    lea rax, [page_tables_l3]
    or  rax, 0b11
    mov [page_tables_l4], rax

    ; L3[0] -> L2
    lea rax, [page_tables_l2]
    or  rax, 0b11
    mov [page_tables_l3], rax

    ; Map first 4 MiB with 2 MiB pages (covers 0xB8000)
    mov rcx, 2
    xor rbx, rbx
    lea rdi, [page_tables_l2]
.map2m:
    mov rax, rbx
    or  rax, (1 << 0) | (1 << 1) | (1 << 7) ; P|RW|PS
    mov [rdi], rax
    add rdi, 8
    add rbx, 0x200000
    loop .map2m
    ret

enable_paging:
    ; Load PML4
    lea rax, [page_tables_l4]
    mov cr3, rax

    ; Enable PAE
    mov rax, cr4
    or  rax, (1 << 5)
    mov cr4, rax

    ; Enable LME
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    ; Enable paging
    mov rax, cr0
    or  rax, (1 << 31)
    mov cr0, rax
    ret

section .bss

align 4096
page_tables_l4: resb 4096
page_tables_l3: resb 4096
page_tables_l2: resb 4096

stack_bottom:   resb 4096 * 4
stack_top:      ; label at end of reserved stack region

input_buffer:   resb 128
input_pos:      resb 1

section .rodata

hex_digits db '0123456789ABCDEF'

banner     db 'BANNER', 10
banner_len equ $ - banner
; clear screen here
prompt     db 'root@wnu-server:~#', 0
prompt_len equ $ - prompt - 1

; Scancode -> ASCII table (pairs)
scancode_table:
    db 0x1e, 'a'
    db 0x30, 'b'
    db 0x2e, 'c'
    db 0x20, 'd'
    db 0x12, 'e'
    db 0x21, 'f'
    db 0x22, 'g'
    db 0x23, 'h'
    db 0x17, 'i'
    db 0x24, 'j'
    db 0x25, 'k'
    db 0x26, 'l'
    db 0x32, 'm'
    db 0x31, 'n'
    db 0x18, 'o'
    db 0x19, 'p'
    db 0x10, 'q'
    db 0x13, 'r'
    db 0x1f, 's'
    db 0x14, 't'
    db 0x16, 'u'
    db 0x2f, 'v'
    db 0x11, 'w'
    db 0x2d, 'x'
    db 0x15, 'y'
    db 0x2c, 'z'
    db 0x02, '1'
    db 0x03, '2'
    db 0x04, '3'
    db 0x05, '4'
    db 0x06, '5'
    db 0x07, '6'
    db 0x08, '7'
    db 0x09, '8'
    db 0x0a, '9'
    db 0x0b, '0'
    db 0x39, ' '
    db 0x1c, 0x0d       ; Enter
    db 0, 0
scancode_table_len equ ($ - scancode_table) / 2

; Minimal 64-bit GDT (if you ever need to reload segments)
gdt64:
    dq 0
.code_segment: equ $ - gdt64
    dq (1 << 43 | 1 << 44 | 1 << 47) | (1 << 53)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
