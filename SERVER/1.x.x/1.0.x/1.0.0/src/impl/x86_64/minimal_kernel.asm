
section .multiboot_header
    align 8
header_start:
    dd 0xe85250d6          ; multiboot2 magic
    dd 0                   ; architecture (protected mode)
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; checksum
    dw 0                   ; reserved
    dw 0                   ; reserved
    dd 8                   ; end tag type
header_end:

section .text
    global _start
_start:
    ; Write a text message to VGA text buffer at 0xb8000
    ; Characters go at even offsets, attribute bytes at odd offsets.
    mov rdi, 0xb8000          ; VGA text buffer pointer
    lea rsi, [rel welcome_msg] ; pointer to message bytes

.write_loop:
    mov al, [rsi]
    test al, al
    je .halt
    mov [rdi], al            ; character
    mov byte [rdi+1], 0x07   ; attribute: white on black
    add rsi, 1
    add rdi, 2
    jmp .write_loop

.halt:
    cli
.hlt_loop:
    hlt
    jmp .hlt_loop

section .rodata
welcome_msg: db "WELCOME TO WNU OS SERVER!", 0
