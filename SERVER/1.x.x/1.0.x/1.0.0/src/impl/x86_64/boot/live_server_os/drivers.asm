; WNU OS SERVER Driver Management System
; Hardware and System Drivers
; Version: 1.0.0.11.11.2025.PRE.ALPHA

section .data
    driver_banner db 'WNU Driver Manager v1.0.0 Starting...', 0x0A, 0
    
    ; Driver definitions
    driver_shutdown db 'Loading shutdown.driver...', 0
    driver_power db 'Loading power-management.driver...', 0
    driver_acpi db 'Loading acpi.driver...', 0
    driver_pci db 'Loading pci-bus.driver...', 0
    driver_storage db 'Loading storage.driver...', 0
    driver_network_hw db 'Loading network-hardware.driver...', 0
    driver_timer db 'Loading system-timer.driver...', 0
    
    ; Driver status messages
    driver_ok db ' [  OK  ]', 0x0A, 0
    driver_fail db ' [FAILED]', 0x0A, 0
    
    ; Shutdown messages
    shutdown_msg db 'System shutdown initiated...', 0x0A, 0
    poweroff_msg db 'Power off sequence starting...', 0x0A, 0
    reboot_msg db 'System reboot initiated...', 0x0A, 0
    halt_msg db 'System halted. Safe to power off.', 0x0A, 0
    acpi_button_msg db 'ACPI Power Button pressed - shutting down...', 0x0A, 0
    
    ; Panic and error messages
    panic_msg db 'KERNEL PANIC! System error detected.', 0x0A, 0
    panic_vga_msg db 'Error: VGA memory corruption detected!', 0x0A, 0
    panic_stack_msg db 'Error: Stack corruption detected!', 0x0A, 0

section .bss
    driver_states resb 8     ; Driver state flags
    power_state resb 1       ; System power state
    acpi_enabled resb 1      ; ACPI subsystem status

section .text
global init_drivers
global shutdown_system
global reboot_system
global halt_system
global poweroff_system
bits 64

; Initialize all system drivers
init_drivers:
    ; Display driver manager banner
    mov rsi, driver_banner
    call print_driver_msg
    
    ; Initialize driver tracking
    mov byte [driver_states], 0
    mov byte [power_state], 1    ; 1 = powered on
    
    ; Load essential drivers in order
    call load_shutdown_driver
    call load_power_driver
    call load_pci_driver
    call load_storage_driver
    call load_network_hw_driver
    call load_timer_driver
    
    ret

; Load shutdown driver
load_shutdown_driver:
    mov rsi, driver_shutdown
    call print_driver_msg
    
    ; Initialize shutdown capabilities
    ; In real implementation: setup ACPI shutdown, APM, etc.
    or byte [driver_states], 0x01   ; Set shutdown driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load power management driver
load_power_driver:
    mov rsi, driver_power
    call print_driver_msg
    
    ; Initialize power management
    ; In real implementation: CPU frequency scaling, power states
    or byte [driver_states], 0x02   ; Set power driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load ACPI driver
load_acpi_driver:
    mov rsi, driver_acpi
    call print_driver_msg
    
    ; Initialize ACPI subsystem
    call setup_acpi_power_button
    or byte [driver_states], 0x04   ; Set ACPI driver bit
    mov byte [acpi_enabled], 1      ; Mark ACPI as enabled
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load PCI bus driver
load_pci_driver:
    mov rsi, driver_pci
    call print_driver_msg
    
    ; Initialize PCI bus scanning
    ; In real implementation: PCI configuration space access
    or byte [driver_states], 0x08   ; Set PCI driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load storage driver
load_storage_driver:
    mov rsi, driver_storage
    call print_driver_msg
    
    ; Initialize storage subsystem
    ; In real implementation: ATA, SATA, NVME drivers
    or byte [driver_states], 0x10   ; Set storage driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load network hardware driver
load_network_hw_driver:
    mov rsi, driver_network_hw
    call print_driver_msg
    
    ; Initialize network hardware
    ; In real implementation: Ethernet, WiFi driver initialization
    or byte [driver_states], 0x20   ; Set network HW driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; Load system timer driver
load_timer_driver:
    mov rsi, driver_timer
    call print_driver_msg
    
    ; Initialize system timers
    ; In real implementation: PIT, HPET, TSC initialization
    or byte [driver_states], 0x40   ; Set timer driver bit
    
    mov rsi, driver_ok
    call print_driver_msg
    ret

; System shutdown function
shutdown_system:
    mov rsi, shutdown_msg
    call print_driver_msg
    
    ; Shutdown sequence
    call stop_all_services
    call sync_filesystems
    call poweroff_system
    ret

; System reboot function
reboot_system:
    mov rsi, reboot_msg
    call print_driver_msg
    
    ; Reboot sequence
    call stop_all_services
    call sync_filesystems
    
    ; Trigger reboot (keyboard controller method)
    mov al, 0xFE
    out 0x64, al
    
    ; Fallback: triple fault reboot
    cli
    lidt [invalid_idt]
    int 3
    ret

; System halt function
halt_system:
    mov rsi, halt_msg
    call print_driver_msg
    
    ; Clean halt
    call stop_all_services
    mov byte [power_state], 0       ; Mark as halted
    
halt_loop:
    cli
    hlt
    jmp halt_loop

; Power off function (VirtualBox/ACPI compatible)
poweroff_system:
    mov rsi, poweroff_msg
    call print_driver_msg
    
    ; Method 1: Standard ACPI S5 state (VirtualBox)
    mov ax, 0x2001      ; SLP_TYPa=0x00, SLP_TYPb=0x00, SLP_EN=1
    mov dx, 0x604       ; PM1a_CNT_BLK port
    out dx, ax
    
    ; Method 2: Alternative VirtualBox shutdown
    mov ax, 0x0400      ; Alternative shutdown command
    mov dx, 0x4004      ; VirtualBox debug port
    out dx, ax
    
    ; Method 3: APM (Advanced Power Management) shutdown
    mov ax, 0x5307      ; APM function: Set Power State
    mov bx, 0x0001      ; All devices
    mov cx, 0x0003      ; Power state: Off
    int 0x15            ; APM BIOS interrupt
    
    ; Method 4: Keyboard controller shutdown (fallback)
    mov al, 0xFE
    out 0x64, al
    
    ; If all else fails: display message and halt
    mov rsi, halt_msg
    call print_driver_msg
    jmp halt_system

; Helper functions
stop_all_services:
    ; Stop all SystemD services gracefully
    ; Implementation depends on service architecture
    ret

sync_filesystems:
    ; Sync all mounted filesystems
    ; Implementation depends on filesystem drivers
    ret

; Helper function for driver messages
print_driver_msg:
    ; rsi = message pointer
    ; Use existing VGA printing infrastructure
    mov rdi, 0xB8000
    
.print_loop:
    lodsb
    test al, al
    jz .done
    
    mov ah, 0x0E        ; Yellow on black for drivers
    stosw
    jmp .print_loop
    
.done:
    ret

; ACPI Power Button Support
setup_acpi_power_button:
    ; Setup ACPI power button event handling
    ; Enable ACPI power button interrupt
    
    ; Set up interrupt handler for IRQ 9 (ACPI SCI)
    ; In a real implementation, this would involve setting up the IDT
    ; and registering our ACPI interrupt handler
    
    ; Enable ACPI power management events
    mov dx, 0x600       ; ACPI PM1a_CNT_BLK
    in ax, dx
    or ax, 0x0001       ; Set SCI_EN (bit 0)
    out dx, ax
    
    ; Enable power button events in PM1_EN
    mov dx, 0x602       ; ACPI PM1a_EN
    in ax, dx
    or ax, 0x0100       ; Set PWRBTN_EN (bit 8)
    out dx, ax
    
    ret

; ACPI Power Button Interrupt Handler
acpi_power_button_handler:
    ; Save registers
    push rax
    push rdx
    push rsi
    
    ; Check if this is a power button event
    mov dx, 0x600       ; ACPI PM1a_STS
    in ax, dx
    test ax, 0x0100     ; Check PWRBTN_STS (bit 8)
    jz .not_power_button
    
    ; Clear the power button status
    mov dx, 0x600       ; ACPI PM1a_STS
    mov ax, 0x0100      ; Clear PWRBTN_STS
    out dx, ax
    
    ; Display power button message
    mov rsi, acpi_button_msg
    call print_driver_msg
    
    ; Restore registers
    pop rsi
    pop rdx
    pop rax
    
    ; Initiate shutdown
    call shutdown_system
    
.not_power_button:
    ; Restore registers
    pop rsi
    pop rdx
    pop rax
    
    ; Return from interrupt
    ret

; Polling-based ACPI power button check (alternative method)
check_acpi_power_button:
    ; Check if ACPI is enabled
    cmp byte [acpi_enabled], 0
    je .no_acpi
    
    ; Check power button status
    mov dx, 0x600       ; ACPI PM1a_STS
    in ax, dx
    test ax, 0x0100     ; Check PWRBTN_STS (bit 8)
    jz .no_button_press
    
    ; Power button was pressed!
    ; Clear the status
    mov dx, 0x600       ; ACPI PM1a_STS
    mov ax, 0x0100      ; Clear PWRBTN_STS
    out dx, ax
    
    ; Display message and shutdown
    mov rsi, acpi_button_msg
    call print_driver_msg
    call shutdown_system
    
.no_button_press:
.no_acpi:
    ret

; Invalid IDT for triple fault reboot
invalid_idt:
    dw 0
    dd 0

; Kernel Panic Handler
kernel_panic:
    ; Disable interrupts immediately
    cli
    
    ; Display panic message
    mov rsi, panic_msg
    call print_driver_msg
    
    ; Try to save system state if possible
    ; In a real kernel, this would dump registers, stack trace, etc.
    
    ; Halt the system
    jmp halt_system

; VGA Memory Corruption Check
check_vga_corruption:
    ; Check if VGA memory is accessible
    mov rax, 0xB8000
    mov rbx, [rax]              ; Try to read from VGA memory
    cmp rbx, rbx                ; This should always be equal
    jne .vga_panic
    
    ; Check if we can write and read back
    mov word [rax], 0x0741      ; Write 'A' with white on black
    mov cx, [rax]               ; Read it back
    cmp cx, 0x0741              ; Check if it matches
    jne .vga_panic
    ret

.vga_panic:
    mov rsi, panic_vga_msg
    call print_driver_msg
    call kernel_panic