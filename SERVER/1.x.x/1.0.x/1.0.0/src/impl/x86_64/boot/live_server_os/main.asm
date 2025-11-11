; WNU OS SERVER Installer - SAFE WITH INPUT
section .text
global main
extern get_input
extern handle_keypress
bits 64

main:
    ; Clear just the first two lines manually (no loops!)
    mov word [0xB8000], 0x0720
    mov word [0xB8002], 0x0720
    mov word [0xB8004], 0x0720
    mov word [0xB8006], 0x0720
    mov word [0xB8008], 0x0720
    mov word [0xB800A], 0x0720
    mov word [0xB800C], 0x0720
    mov word [0xB800E], 0x0720
    mov word [0xB8010], 0x0720
    mov word [0xB8012], 0x0720
    mov word [0xB8014], 0x0720
    mov word [0xB8016], 0x0720
    mov word [0xB8018], 0x0720
    mov word [0xB801A], 0x0720
    mov word [0xB801C], 0x0720
    mov word [0xB801E], 0x0720
    
    ; Clear second line too
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    mov word [0xB80B0], 0x0720
    mov word [0xB80B2], 0x0720
    mov word [0xB80B4], 0x0720
    mov word [0xB80B6], 0x0720
    
    ; Write the prompt manually
    mov word [0xB8000], 0x0A72  ; 'r' green
    mov word [0xB8002], 0x0A6F  ; 'o' green  
    mov word [0xB8004], 0x0A6F  ; 'o' green
    mov word [0xB8006], 0x0A74  ; 't' green
    mov word [0xB8008], 0x0A40  ; '@' green
    mov word [0xB800A], 0x0A6C  ; 'l' green
    mov word [0xB800C], 0x0A69  ; 'i' green
    mov word [0xB800E], 0x0A76  ; 'v' green
    mov word [0xB8010], 0x0A65  ; 'e' green
    mov word [0xB8012], 0x073A  ; ':' white
    mov word [0xB8014], 0x097E  ; '~' light blue
    mov word [0xB8016], 0x0C23  ; '#' red
    mov word [0xB8018], 0x0720  ; ' ' white


    ; Initialize cursor position
    mov rdi, 0xB8020            ; After prompt

input_loop:
    ; Get input safely
    call get_input
    cmp al, 0
    je input_loop               ; No input, keep checking
    
    ; Handle the keypress
    call handle_keypress
    
    ; Continue input loop
    jmp input_loop