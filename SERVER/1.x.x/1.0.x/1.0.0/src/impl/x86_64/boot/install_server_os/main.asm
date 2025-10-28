; Simple VGA-only installer proof-of-concept
; - Displays install instructions
; - Presents prompt: root@install:~#
; - Echoes typed characters, supports Backspace and Enter
; - Recognizes a small set of commands and prints simulated outputs
;
; Minimal, robust VGA-only installer POC
section .data
banner db "WNU OS SERVER 1.0.0 Installer (SIMULATED)",0
prompt db "root@install:~# ",0
no_disk db "NO DISK FOUND AT HARDWARE",0
done_msg db "Operation completed.",0
ok_msg db "OK",0

fdisk_l db "fdisk -l",0
fdisk_l_len equ $-fdisk_l
fdisk_main db "fdisk /dev/main",0
fdisk_main_len equ $-fdisk_main

section .bss
buf resb 128
buf_len resb 1

section .text
global _start
bits 64

; write ASCIIZ at RSI to VGA position RDI
print_vga:
    xor rcx, rcx
.pv_loop:
    mov al, [rsi+rcx]
    test al, al
    jz .pv_done
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rdi, 2
    inc rcx
    jmp .pv_loop
.pv_done:
    ret

; put char in AL to VGA at RDI and advance RDI
putc_vga:
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rdi, 2
    ret

; get scancode from keyboard controller in AL
get_scancode:
.gs_wait:
    in al, 0x64
    test al, 1
    jz .gs_wait
    in al, 0x60
    ret

; small scancode->ascii mapper (covers letters, numbers, space, backspace, enter)
; input: AL=scancode, output: AL=ascii or 0
sc_to_ascii:
    cmp al, 0x1C
    je .sc_enter
    cmp al, 0x0E
    je .sc_bs
    cmp al, 0x39
    je .sc_space
    cmp al, 0x02
    je .sc_1
    cmp al, 0x03
    je .sc_2
    cmp al, 0x04
    je .sc_3
    cmp al, 0x10
    je .sc_q
    cmp al, 0x11
    je .sc_w
    cmp al, 0x12
    je .sc_e
    cmp al, 0x13
    je .sc_r
    cmp al, 0x1E
    je .sc_a
    cmp al, 0x1F
    je .sc_s
    cmp al, 0x20
    je .sc_d
    cmp al, 0x21
    je .sc_f
    cmp al, 0x2C
    je .sc_z
    cmp al, 0x2D
    je .sc_x
    cmp al, 0x2E
    je .sc_c
    cmp al, 0x2F
    je .sc_v
    xor al, al
    ret
.sc_enter: mov al, 10; ret
.sc_bs: mov al, 8; ret
.sc_space: mov al, ' '; ret
.sc_1: mov al, '1'; ret
.sc_2: mov al, '2'; ret
.sc_3: mov al, '3'; ret
.sc_q: mov al, 'q'; ret
.sc_w: mov al, 'w'; ret
.sc_e: mov al, 'e'; ret
.sc_r: mov al, 'r'; ret
.sc_a: mov al, 'a'; ret
.sc_s: mov al, 's'; ret
.sc_d: mov al, 'd'; ret
.sc_f: mov al, 'f'; ret
.sc_z: mov al, 'z'; ret
.sc_x: mov al, 'x'; ret
.sc_c: mov al, 'c'; ret
.sc_v: mov al, 'v'; ret

_start:
    ; VGA base
    mov rdi, 0xb8000
    ; print banner
    lea rsi, [rel banner]
    call print_vga
    ; new line
    mov byte [rdi], 10
    mov byte [rdi+1], 0x07
    add rdi, 2

    ; set prompt line: move down ~8 rows
    add rdi, 160*8

mainloop:
    ; print prompt
    lea rsi, [rel prompt]
    call print_vga
    ; reset buffer
    mov byte [buf_len], 0
    lea rbx, [buf]

read_loop:
    call get_scancode
    push rax
    call sc_to_ascii
    pop rax
    cmp al, 0
    je read_loop
    cmp al, 8
    je do_backspace
    cmp al, 10
    je do_enter
    ; echo
    mov [rbx], al
    mov rdi, 0 ; placeholder adjust below
    ; compute current VGA cursor: reuse rdi set earlier by prompt; to simplify
    ; we'll just write sequentially by maintaining a local pointer in r12
    ; if r12 not set, initialize to current rdi
    cmp r12, 0
    jne .have_r12
    lea r12, [0xb8000 + 160*9]
.have_r12:
    mov rdi, r12
    mov al, [rbx]
    call putc_vga
    mov r12, rdi
    inc rbx
    inc byte [buf_len]
    jmp read_loop

do_backspace:
    mov al, [buf_len]
    test al, al
    jz read_loop
    dec byte [buf_len]
    dec rbx
    ; move r12 back two bytes
    sub r12, 2
    mov byte [r12], ' '
    mov byte [r12+1], 0x07
    jmp read_loop

do_enter:
    ; null-terminate
    mov byte [rbx], 0
    ; simple command handling
    lea rsi, [rel buf]
    ; check fdisk -l
    lea rdi, [rel fdisk_l]
    mov rcx, fdisk_l_len
    push rbx
    push r12
    ; compare
    mov rdx, rsi
    mov rax, 0
    mov rbx, 0
    ; quick memcmp
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rdx
    mov rcx, rax
    ; naive: compare first byte
    mov al, [buf]
    cmp al, 'f'
    jne .other_cmd
    ; print no_disk
    lea rsi, [rel no_disk]
    mov rdi, r12
    call print_vga
    jmp .after_cmd
.other_cmd:
    ; default OK
    lea rsi, [rel ok_msg]
    mov rdi, r12
    call print_vga
.after_cmd:
    add r12, 160
    add r12, 160
    pop r12
    pop rbx
    jmp mainloop
