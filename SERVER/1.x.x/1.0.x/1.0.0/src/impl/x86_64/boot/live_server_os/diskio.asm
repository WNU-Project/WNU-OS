; WNU OS SERVER - Disk Reading Module for ISO 9660
; Following OSDev Wiki standards for disk access
; Author: WNU Project
; License: MIT

section .data
    ; BIOS Disk Reading Constants
    DISK_READ_SUCCESS equ 0
    DISK_READ_ERROR equ 1
    BIOS_DISK_READ equ 0x13            ; BIOS interrupt for disk services
    
    ; Drive information (set during boot)
    boot_drive db 0                    ; Boot drive number from bootloader

section .bss
    ; Reserved space for future disk operations

section .text
    global read_sector
    global set_boot_drive
    global read_multiple_sectors

; Set boot drive number (called from bootloader)
; Input: DL = boot drive number
set_boot_drive:
    push rbp
    mov rbp, rsp
    
    mov [boot_drive], dl
    
    pop rbp
    ret

; Read single sector from disk
; Input: EAX = LBA sector number, RSI = buffer address
; Output: EAX = 0 on success, 1 on error
read_sector:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx
    push rdx
    push rdi
    
    ; Convert LBA to CHS for older BIOS compatibility
    ; For now, use LBA extended read (more reliable for ISO 9660)
    call read_sector_lba
    
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rbp
    ret

; Read sector using LBA extended read (INT 13h, AH=42h)
; Input: EAX = LBA, RSI = buffer
; Output: EAX = status
read_sector_lba:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; For now, return failure to indicate no real disk I/O available
    ; In a real implementation, we would need to set up proper BIOS calls
    ; or use AHCI/IDE drivers for disk access
    
    ; Fill buffer with zeros but return error to indicate no real data
    mov rdi, rsi                       ; Destination buffer
    mov rcx, 2048                      ; ISO 9660 sector size (2KB, not 512)
    xor al, al                         ; Fill with zeros
    rep stosb                          ; Fill the buffer
    
    mov eax, 1                         ; Return error (no real disk I/O)
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rbp
    ret

; Read sector using CHS (stub implementation for 64-bit compatibility)
; Input: EAX = LBA, RSI = buffer  
; Output: EAX = status
read_sector_chs:
    push rbp
    mov rbp, rsp
    
    ; Stub implementation - just return success
    ; In a real OS, this would implement CHS disk reading
    xor eax, eax                       ; Success
    
    pop rbp
    ret

; Read multiple consecutive sectors
; Input: EAX = starting LBA, ECX = sector count, RSI = buffer
; Output: EAX = 0 on success, 1 on error
read_multiple_sectors:
    push rbp
    mov rbp, rsp
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    mov ebx, eax                       ; Starting LBA
    mov edi, ecx                       ; Sector count
    
.read_next_sector:
    test edi, edi
    jz .read_complete
    
    ; Read one sector
    mov eax, ebx
    call read_sector
    test eax, eax
    jnz .read_multiple_error
    
    ; Move to next sector
    add rsi, 512                       ; Advance buffer (assuming 512 byte sectors)
    inc ebx                            ; Next LBA
    dec edi                            ; Decrement count
    jmp .read_next_sector
    
.read_complete:
    xor eax, eax                       ; Success
    jmp .read_multiple_done
    
.read_multiple_error:
    mov eax, DISK_READ_ERROR
    
.read_multiple_done:
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rbp
    ret

section .data
    ; Additional debug and status messages
    debug_disk_init db 'DEBUG: Disk reading module initialized', 0
    debug_lba_read db 'DEBUG: Using LBA extended read', 0
    debug_chs_read db 'DEBUG: Falling back to CHS read', 0