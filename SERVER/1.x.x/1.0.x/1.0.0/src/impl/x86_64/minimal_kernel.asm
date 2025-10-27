
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
    mov byte [0xb8000], 'Z' ; Show Z if booted
    hlt
