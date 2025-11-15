; WNU OS SERVER SystemD Shutdown Module
; Graceful system shutdown and service management
; Version: 1.0.0.11.11.2025.PRE.ALPHA

section .data
    shutdown_banner db 'SystemD Shutdown Manager Starting...', 0x0A, 0
    stopping_services db 'Stopping all services...', 0x0A, 0
    service_stopped db 'Service stopped: ', 0
    shutdown_complete db 'All services stopped. System ready for poweroff.', 0x0A, 0
    
    ; Service stop messages
    stop_shell db 'wnu-shell.service', 0x0A, 0
    stop_compiler db 'wnuc-compiler.service', 0x0A, 0
    stop_networkd db 'networkd.service', 0x0A, 0
    stop_keyboard db 'wnu-keyboard.service', 0x0A, 0
    stop_vga db 'wnu-vga.service', 0x0A, 0
    
    ; Status messages
    stop_ok db ' [  OK  ]', 0x0A, 0
    stop_fail db ' [FAILED]', 0x0A, 0

section .bss
    shutdown_state resb 1    ; Current shutdown state
    services_stopped resb 1  ; Count of stopped services

section .text
global systemd_shutdown
global systemd_poweroff
global systemd_reboot
global systemd_halt
extern shutdown_system
extern poweroff_system
extern reboot_system
extern halt_system
bits 64

; Main SystemD shutdown function
systemd_shutdown:
    ; Display shutdown banner
    mov rsi, shutdown_banner
    call print_shutdown_msg
    
    ; Initialize shutdown state
    mov byte [shutdown_state], 1    ; 1 = shutdown in progress
    mov byte [services_stopped], 0  ; No services stopped yet
    
    ; Stop all services in reverse order (opposite of startup)
    call stop_all_systemd_services
    
    ; Display completion message
    mov rsi, shutdown_complete
    call print_shutdown_msg
    
    ; Wait 5 seconds for user to see shutdown sequence
    call shutdown_delay_5sec
    call shutdown_delay_5sec
    call shutdown_delay_5sec
    call shutdown_delay_5sec
    call shutdown_delay_5sec
    
    ; Don't call any system function here - let individual functions handle it
    ret

; Stop all SystemD services gracefully
stop_all_systemd_services:
    mov rsi, stopping_services
    call print_shutdown_msg
    call shutdown_delay        ; Small delay to see message
    
    ; Stop services in reverse dependency order
    ; (Last started = first stopped)
    
    ; 5. Stop shell service (user interface)
    call stop_shell_service
    call shutdown_delay
    
    ; 4. Stop compiler service
    call stop_compiler_service
    call shutdown_delay
    
    ; 3. Stop NetworkD service
    call stop_networkd_service
    call shutdown_delay
    
    ; 2. Stop keyboard service
    call stop_keyboard_service
    call shutdown_delay
    
    ; 1. Stop VGA service (last)
    call stop_vga_service
    call shutdown_delay
    
    ret

; Individual service stop functions
stop_shell_service:
    mov rsi, service_stopped
    call print_shutdown_msg
    mov rsi, stop_shell
    call print_shutdown_msg
    
    ; Actual shell service cleanup would go here
    ; For now, just mark as stopped
    inc byte [services_stopped]
    
    mov rsi, stop_ok
    call print_shutdown_msg
    ret

stop_compiler_service:
    mov rsi, service_stopped
    call print_shutdown_msg
    mov rsi, stop_compiler
    call print_shutdown_msg
    
    ; Compiler service cleanup
    inc byte [services_stopped]
    
    mov rsi, stop_ok
    call print_shutdown_msg
    ret

stop_networkd_service:
    mov rsi, service_stopped
    call print_shutdown_msg
    mov rsi, stop_networkd
    call print_shutdown_msg
    
    ; NetworkD service cleanup
    ; Could call networkd_stop function here
    inc byte [services_stopped]
    
    mov rsi, stop_ok
    call print_shutdown_msg
    ret

stop_keyboard_service:
    mov rsi, service_stopped
    call print_shutdown_msg
    mov rsi, stop_keyboard
    call print_shutdown_msg
    
    ; Keyboard service cleanup
    inc byte [services_stopped]
    
    mov rsi, stop_ok
    call print_shutdown_msg
    ret

stop_vga_service:
    mov rsi, service_stopped
    call print_shutdown_msg
    mov rsi, stop_vga
    call print_shutdown_msg
    
    ; VGA service cleanup
    inc byte [services_stopped]
    
    mov rsi, stop_ok
    call print_shutdown_msg
    ret

; SystemD Poweroff (graceful shutdown then power off)
systemd_poweroff:
    call systemd_shutdown
    call poweroff_system
    ret

; SystemD Reboot (graceful shutdown then reboot)
systemd_reboot:
    call systemd_shutdown
    call reboot_system
    ret

; SystemD Halt (graceful shutdown then halt)
systemd_halt:
    call systemd_shutdown
    call halt_system
    ret

; Helper functions
print_shutdown_msg:
    ; rsi = message pointer
    ; Use existing VGA printing infrastructure
    mov rdi, 0xB8000
    ; Find next available line (simple implementation)
    ; In a real implementation, this would track current line
    
.find_line:
    cmp word [rdi], 0x0720      ; Empty space?
    je .print_here
    add rdi, 160                ; Next line (80 chars * 2 bytes)
    cmp rdi, 0xB8FA0            ; End of screen?
    jae .print_here             ; Use last line if screen full
    jmp .find_line

.print_here:
.print_loop:
    lodsb
    test al, al
    jz .done
    
    mov ah, 0x0C                ; Red on black for shutdown messages
    stosw
    jmp .print_loop
    
.done:
    ret

; Short delay for shutdown messages
shutdown_delay:
    mov rcx, 100000000          ; Longer delay counter (1 second)
.delay_loop:
    nop
    loop .delay_loop
    ret

; 5-second delay for screenshot timing
shutdown_delay_5sec:
    mov rcx, 500000000          ; 5 second delay counter
.delay_loop_5sec:
    nop
    loop .delay_loop_5sec
    ret