section .multiboot_header
align 4
multiboot_header:
    dd 0x1BADB002                ; Multiboot1 magic number
    dd 0x00000000                ; Flags
    dd -(0x1BADB002 + 0x00000000); Checksum

section .text
global _start
extern main

_start:
    ; Set up minimal 64-bit environment
    cli                          ; Disable interrupts
    
    ; Set up stack
    mov rsp, stack_top
    mov word [0xB8000], 0x0753 ; S
    mov word [0xB8002], 0x0754 ; T
    mov word [0xB8004], 0x0741 ; A
    mov word [0xB8006], 0x0752 ; R
    mov word [0xB8008], 0x0754 ; T
    mov word [0xB800A], 0x0749 ; I
    mov word [0xB800C], 0x074E ; N
    mov word [0xB800E], 0x0747 ; G
    
    ; Call our assembly main function
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