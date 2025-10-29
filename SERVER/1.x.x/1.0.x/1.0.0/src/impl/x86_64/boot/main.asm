global start

section .text
bits 64

extern kernel_main

start:
    ; Initialize stack pointer
    mov rsp, stack_top

    ; Call into C kernel_main
    call kernel_main

    ; Halt forever when kernel_main returns
    cli
.halt:
    hlt
    jmp .halt

section .bss

align 4096
stack_bottom:   resb 4096 * 4
stack_top:
