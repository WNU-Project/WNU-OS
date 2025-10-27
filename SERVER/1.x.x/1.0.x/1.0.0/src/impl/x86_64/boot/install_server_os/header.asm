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