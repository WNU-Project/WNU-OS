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
boot_msg db "[dbg] BOOT\n",0
prompt_msg db "[dbg] PROMPT\n",0
enter_msg db "[dbg] ENTER\n",0

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

; -----------------
; Serial (COM1) helpers for debugging
; -----------------
; serial_putc: write AL to COM1 (0x3F8)
serial_putc:
    push rdx
.sp_wait:
    mov dx, 0x3FD        ; Line Status Register (COM1 + 5)
    in al, dx
    test al, 0x20      ; Transmitter Holding Register Empty
    jz .sp_wait
    ; write byte in AL to COM1
    mov dx, 0x3F8
    out dx, al
    pop rdx
    ret

; serial_print: RSI -> NUL-terminated string
serial_print:
    push rax
    push rsi
    .sp_loop:
        mov al, [rsi]
        test al, al
        jz .sp_done
        call serial_putc
        inc rsi
        jmp .sp_loop
    .sp_done:
    pop rsi
    pop rax
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
    ; initialize VGA cursor in r12 to a deterministic place
    mov r12, 0xb8000
    ; print banner at r12
    mov rdi, r12
    lea rsi, [rel banner]
    call print_vga
    ; serial log: boot reached
    lea rsi, [rel boot_msg]
    call serial_print
    ; advance r12 two lines (160 bytes per line)
    add r12, 160*2

mainloop:
    ; print prompt at current cursor (r12)
    mov rdi, r12
    lea rsi, [rel prompt]
    call print_vga
    ; serial log: prompt printed
    lea rsi, [rel prompt_msg]
    call serial_print

    ; reset buffer pointer and length
    lea rsi, [rel buf]    ; buffer base in rsi
    xor rcx, rcx          ; rcx = buf_len

read_loop:
    call get_scancode
    call sc_to_ascii        ; result in AL
    test al, al
    jz read_loop
    cmp al, 8
    je do_backspace
    cmp al, 10
    je do_enter
    ; echo character and store in buffer
    mov [rsi+rcx], al
    mov rdi, r12
    call putc_vga
    mov r12, rdi
    inc rcx
    cmp rcx, 127
    jb read_loop
    jmp read_loop

do_backspace:
    cmp rcx, 0
    je read_loop
    dec rcx
    ; move VGA cursor back two bytes and clear
    sub r12, 2
    mov byte [r12], ' '
    mov byte [r12+1], 0x07
    jmp read_loop

do_enter:
    ; null-terminate buffer
    mov byte [rsi+rcx], 0
    ; print newline (ASCII 10) as a character
    mov al, 10
    mov rdi, r12
    call putc_vga
    mov r12, rdi
    ; serial log: enter pressed
    lea rsi, [rel enter_msg]
    call serial_print

    ; compare buffer with "fdisk -l"
    lea rsi, [rel fdisk_l]
    lea rdx, [rel buf]
    mov rcx, fdisk_l_len
    ; simple memcmp
.cmp_loop:
    cmp rcx, 0
    je .fdisk_match
    mov al, [rsi]
    mov bl, [rdx]
    cmp al, bl
    jne .not_fdisk
    inc rsi
    inc rdx
    dec rcx
    jmp .cmp_loop
.fdisk_match:
    lea rsi, [rel no_disk]
    mov rdi, r12
    call print_vga
    jmp .after_cmd
.not_fdisk:
    lea rsi, [rel ok_msg]
    mov rdi, r12
    call print_vga
.after_cmd:
    ; advance two lines for next prompt
    add r12, 160
    add r12, 160
    jmp mainloop
