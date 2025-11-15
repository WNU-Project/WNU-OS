; WNU OS SERVER SystemD Implementation
; Service Management and Init System
; Version: 1.0.0.11.11.2025.PRE.ALPHA

section .data
    systemd_banner db 'WNU SystemD v1.0.0 Starting...', 0x0A, 0
    systemd_ready db 'SystemD: All services loaded', 0x0A, 0
    systemd_complete db 'SystemD initialization complete. Starting WNU OS...', 0x0A, 0
    
    ; Service definitions
    service_networkd db 'Starting networkd.service...', 0
    service_shell db 'Starting wnu-shell.service...', 0
    service_vga db 'Starting wnu-vga.service...', 0
    service_keyboard db 'Starting wnu-keyboard.service...', 0
    service_filesystem db 'Starting wnu-filesystem.service...', 0
    service_compiler db 'Starting wnuc-compiler.service...', 0
    
    ; Filesystem mounting message
    mounting_fs db 'Mounting Filesystem.....', 0x0A, 0
    
    ; Service status messages
    service_ok db ' [  OK  ]', 0x0A, 0
    service_error db ' [FAILED]', 0x0A, 0
    service_fail db ' [FAILED]', 0x0A, 0

section .bss
    service_states resb 8    ; Bit flags for service states
    active_services resb 1   ; Count of active services
    current_line resb 1      ; Track current VGA line

section .text
global init_systemd
global systemd_start_service
global systemd_stop_service  
global systemd_status
extern networkd_init
extern networkd_start
extern init_drivers
extern init_filesystem
extern init_iso9660_filesystem
bits 64

; SystemD initialization - PID 1 equivalent
init_systemd:
    ; Initialize VGA cursor position
    mov byte [current_line], 0
    ; Clear Screen If GRUB Autoboots
    call clear_screen
    
    ; Display SystemD banner
    mov rsi, systemd_banner
    call print_systemd_msg
    call wait_delay
    
    ; Initialize service tracking
    mov byte [service_states], 0
    mov byte [active_services], 0
    
    ; Initialize drivers first
    call init_drivers
    
    ; Start essential services in order
    call start_essential_services
    
    ; Mark SystemD as ready
    mov rsi, systemd_ready
    call print_systemd_msg
    call wait_delay
    
    ; Show completion message
    mov rsi, systemd_complete
    call print_systemd_msg
    call wait_5_seconds
    
    ; Clear screen before proceeding to main
    call clear_screen
    
    ret

; Start essential services in dependency order
start_essential_services:
    ; 1. VGA Service (display)
    call start_vga_service
    
    ; 2. Keyboard Service (input)
    call start_keyboard_service
    
    ; 3. File System Service (storage)
    call start_filesystem_service
    
    ; 4. NetworkD Service (networking)
    call start_networkd_service
    
    ; 5. Compiler Service (development)
    call start_compiler_service
    
    ; 5. Shell Service (user interface) - last
    call start_shell_service
    
    ret

; Start NetworkD as a systemd service
start_networkd_service:
    mov rsi, service_networkd
    call print_systemd_msg
    
    ; Initialize and start NetworkD
    call networkd_init
    call networkd_start
    
    ; Mark NetworkD service as active
    or byte [service_states], 0x01  ; Set bit 0 for networkd
    inc byte [active_services]
    
    mov rsi, service_ok
    call print_systemd_msg
    
    ret

; Start VGA service
start_vga_service:
    mov rsi, service_vga
    call print_systemd_msg
    
    ; VGA is already initialized, just mark as active
    or byte [service_states], 0x02  ; Set bit 1 for vga
    inc byte [active_services]
    
    mov rsi, service_ok
    call print_systemd_msg
    ret

; Start Keyboard service  
start_keyboard_service:
    mov rsi, service_keyboard
    call print_systemd_msg
    
    ; Keyboard is already initialized, just mark as active
    or byte [service_states], 0x04  ; Set bit 2 for keyboard
    inc byte [active_services]
    
    mov rsi, service_ok
    call print_systemd_msg
    ret

; Start File System service
start_filesystem_service:
    ; Print service start message (safely)
    mov rsi, service_filesystem
    call print_systemd_msg
    
    ; Display mounting message (safely)
    mov rsi, mounting_fs
    call print_systemd_msg
    
    ; Initialize ISO 9660 filesystem (with error handling)
    call init_iso9660_filesystem
    test eax, eax              ; Check if initialization succeeded
    jnz .filesystem_error     ; Jump if error occurred
    
    ; Mark filesystem service as active only if successful
    or byte [service_states], 0x10  ; Set bit 4 for filesystem
    inc byte [active_services]
    
    ; Print success message
    mov rsi, service_ok
    call print_systemd_msg
    ret

.filesystem_error:
    ; Filesystem initialization failed, but don't crash
    ; Mark service as failed but continue boot process
    mov rsi, service_error
    call print_systemd_msg
    ret

; Start Compiler service
start_compiler_service:
    mov rsi, service_compiler
    call print_systemd_msg
    
    ; WNUC compiler service (always available)
    or byte [service_states], 0x08  ; Set bit 3 for compiler
    inc byte [active_services]
    
    mov rsi, service_ok
    call print_systemd_msg
    ret

; Start Shell service (last - user interface)
start_shell_service:
    mov rsi, service_shell
    call print_systemd_msg
    
    ; Shell service (main user interface)
    or byte [service_states], 0x10  ; Set bit 4 for shell
    inc byte [active_services]
    
    mov rsi, service_ok
    call print_systemd_msg
    ret

; Generic service control
systemd_start_service:
    ; rdi = service name (future parameter)
    ret

systemd_stop_service:
    ; rdi = service name (future parameter)
    ret

; Get system status
systemd_status:
    ; Return number of active services
    movzx rax, byte [active_services]
    ret

; Helper function for SystemD messages (Safe VGA access)
print_systemd_msg:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; rsi = message pointer
    ; Validate current_line to prevent buffer overflow
    movzx rax, byte [current_line]
    cmp rax, 25                 ; Max 25 lines on VGA screen
    jl .valid_line
    mov byte [current_line], 0  ; Reset to top if overflow
    movzx rax, byte [current_line]
    
.valid_line:
    ; Calculate VGA position based on current line
    mov rbx, 160                 ; 80 characters * 2 bytes per character
    mul rbx
    
    ; Verify VGA buffer address is valid before writing
    add rax, 0xB8000            ; VGA text buffer base
    mov rdi, rax
    
    ; Limit message length to prevent overflow
    mov rcx, 79                 ; Max characters per line - 1
    
.print_loop:
    test rcx, rcx               ; Check character count limit
    jz .line_full
    lodsb
    test al, al
    jz .done
    
    ; Safe character output with bounds check
    cmp rdi, 0xB8000 + (25 * 160) ; End of VGA buffer
    jge .done                   ; Stop if beyond buffer
    
    mov ah, 0x07                ; Light gray on black (standard VGA attribute)
    stosw
    dec rcx
    jmp .print_loop
    
.line_full:
.done:
    ; Advance to next line safely
    inc byte [current_line]
    movzx rax, byte [current_line]
    cmp rax, 25
    jl .no_reset
    mov byte [current_line], 0  ; Wrap around if at bottom
    
.no_reset:
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret
    inc byte [current_line]     ; Move to next line
    ret

; Short delay function
wait_delay:
    mov rcx, 0x8FFFFF           ; Delay counter
.delay_loop:
    nop
    loop .delay_loop
    ret

; Longer delay for completion message
wait_longer_delay:
    mov rcx, 0x1FFFFFF          ; Longer delay counter
.delay_loop:
    nop
    loop .delay_loop
    ret

; 5-second delay after SystemD completion (perfect for screenshots!)
wait_5_seconds:
    ; Approximate 5-second delay (CPU dependent)
    mov rcx, 0x9FFFFFFF         ; ~5 second delay counter
.delay_loop:
    nop
    loop .delay_loop
    ret

; Clear entire screen
clear_screen:
    push rax
    push rcx
    push rdi
    
    mov rdi, 0xB8000            ; VGA text buffer
    mov rcx, 2000               ; 80x25 = 2000 characters
    mov ax, 0x0720              ; Space with light gray on black (same as print function)
    
.clear_loop:
    stosw                       ; Store ax to [rdi] and increment rdi by 2
    loop .clear_loop
    
    ; Reset line counter
    mov byte [current_line], 0
    
    pop rdi
    pop rcx
    pop rax
    ret