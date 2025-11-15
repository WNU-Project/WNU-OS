section .multiboot_header
header_start:
    dd 0xe85250d6  ; Magic number for multiboot
    dd 0           ; Flags
    dd header_end - header_start  ; Header size
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))  ; Checksum
    dw 0
    dw 0
    dd 8
header_end:

section .text
bits 64
global _start
extern main
extern init_systemd

_start:
    ; Set up minimal environment
    cli                          ; Disable interrupts
    
    ; Set up stack pointer
    mov esp, stack_top
    
    ; Display early boot message
    mov word [0xB8000], 0x0753 ; S
    mov word [0xB8002], 0x0754 ; T
    mov word [0xB8004], 0x0741 ; A
    mov word [0xB8006], 0x0752 ; R
    mov word [0xB8008], 0x0754 ; T
    mov word [0xB800A], 0x0749 ; I
    mov word [0xB800C], 0x074E ; N
    mov word [0xB800E], 0x0747 ; G
    mov word [0xB8010], 0x0720 ; Space
    mov word [0xB8012], 0x0753 ; S
    mov word [0xB8014], 0x0759 ; Y
    mov word [0xB8016], 0x0753 ; S
    mov word [0xB8018], 0x0754 ; T
    mov word [0xB801A], 0x0745 ; E
    mov word [0xB801C], 0x074D ; M
    mov word [0xB801E], 0x0744 ; D
    
    ; Initialize SystemD (PID 1)
    call init_systemd
    
    ; Call main kernel function
    call main
    
    ; Halt if we return
halt_loop:
    cli
    hlt
    jmp halt_loop

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16KB stack
stack_top: