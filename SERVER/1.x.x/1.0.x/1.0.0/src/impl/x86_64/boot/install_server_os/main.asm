; WNU OS SERVER 1.0.0 Installer - Based on Official Instructions
; Follows the exact steps from installinstuct.txt
section .data
; Main installer screens
title_line db "WNU OS SERVER 1.0.0 Installation Instructions", 0
subtitle_line db "(PLEASE UNDERSTAND THIS IS A PRE-ALPHA VERSION)", 0
divider db "================================================================", 0

; Step 1: Welcome
welcome_title db "Step 1: Installation Overview", 0
welcome_1 db "Welcome to WNU OS Server 1.0.0 Installer!", 0
welcome_2 db "This installer will guide you through the process.", 0
welcome_3 db "Press any key to continue...", 0

; Step 2: Disk Check  
disk_title db "Step 2: Disk Detection", 0
disk_1 db "Checking for available disks...", 0
disk_2 db "Command: fdisk -l", 0
disk_3 db "Result: NO DISK FOUND AT HARDWARE", 0
disk_4 db "", 0
disk_5 db "NOTICE: Like TempleOS, WNU OS SERVER runs at Ring 0.", 0
disk_6 db "Everything you do WRONG is YOUR FAULT, not OURS.", 0
disk_7 db "READ CAREFULLY and follow instructions.", 0

; Step 3: Partitioning
part_title db "Step 3: Disk Partitioning", 0
part_1 db "WARNING: Installing WNU OS SERVER will DELETE ALL DATA!", 0
part_2 db "Make sure you have backed up your data first.", 0
part_3 db "", 0
part_4 db "Running: fdisk /dev/main", 0
part_5 db "[OK] Deleting all existing partitions...", 0
part_6 db "[OK] Creating new PRIMARY partition...", 0
part_7 db "[OK] Making partition BOOTABLE...", 0
part_8 db "[OK] Writing changes to disk...", 0

; Step 4: Filesystem
fs_title db "Step 4: Filesystem Creation", 0
fs_1 db "Using UNIX Filesystem (UNFS) - WNU OS native format", 0
fs_2 db "", 0
fs_3 db "Running: mkfs.unfs /dev/main1", 0
fs_4 db "[OK] Formatting partition with UNFS...", 0
fs_5 db "[OK] Filesystem created successfully", 0
fs_6 db "", 0
fs_7 db "Running: mount /dev/main1 /mnt", 0
fs_8 db "[OK] Partition mounted to /mnt", 0

; Step 5: System Files
files_title db "Step 5: Installing System Files", 0
files_1 db "Copying WNU OS Server system files...", 0
files_2 db "", 0
files_3 db "Running: cp -r ~/filesystem/* /mnt/", 0
files_4 db "[OK] Kernel files copied", 0
files_5 db "[OK] System libraries copied", 0
files_6 db "[OK] Configuration files copied", 0
files_7 db "[OK] User space utilities copied", 0
files_8 db "[OK] All system files installed", 0

; Step 6: Package Manager
pkg_title db "Step 6: Package Manager Installation", 0
pkg_1 db "Installing PACMAN Package Manager...", 0
pkg_2 db "", 0
pkg_3 db "Running: cp ~/pacman/* /mnt/usr/bin/", 0
pkg_4 db "[OK] PACMAN binary installed", 0
pkg_5 db "[OK] Package database initialized", 0
pkg_6 db "[OK] Package manager ready", 0

; Step 7: Bootloader
boot_title db "Step 7: Bootloader Installation (CRITICAL STEP)", 0
boot_1 db "IF YOU DO THIS WRONG, REINSTALL EVERYTHING!", 0
boot_2 db "", 0
boot_3 db "Running: grub-install --root-directory=/mnt /dev/main", 0
boot_4 db "[OK] GRUB bootloader installed", 0
boot_5 db "[OK] Boot configuration created", 0
boot_6 db "[OK] System is now bootable", 0

; Final step
final_title db "Installation Complete!", 0
final_1 db "GOOD JOB! YOU HAVE INSTALLED WNU OS SERVER 1.0.0!", 0
final_2 db "", 0
final_3 db "Next steps:", 0
final_4 db "1. REBOOT YOUR SYSTEM", 0
final_5 db "2. REMOVE INSTALLATION MEDIA/ISO", 0
final_6 db "3. BOOT TO YOUR NEW WNU OS SERVER 1.0.0!", 0
final_7 db "", 0
final_8 db "System will halt. Power off VM to continue.", 0

; Progress indicators
progress_0 db "[    ] Step 0/7: Initializing", 0
progress_1 db "[=   ] Step 1/7: Welcome", 0
progress_2 db "[==  ] Step 2/7: Disk Detection", 0
progress_3 db "[=== ] Step 3/7: Partitioning", 0
progress_4 db "[====] Step 4/7: Filesystem", 0
progress_5 db "[====] Step 5/7: System Files", 0
progress_6 db "[====] Step 6/7: Package Manager", 0
progress_7 db "[====] Step 7/7: Bootloader - COMPLETE!", 0

section .text
global _start
bits 64

; Safe VGA printing function
print_line:
    push rax
    push rcx
    push rdi
    push rsi
    
    xor rcx, rcx
.print_char:
    mov al, [rsi + rcx]
    test al, al
    jz .done
    
    ; Bounds check
    cmp rdi, 0xB8FA0
    jge .done
    
    mov [rdi], al
    mov byte [rdi + 1], 0x0F    ; Bright white text
    add rdi, 2
    inc rcx
    cmp rcx, 79                 ; Max 79 chars per line
    jge .done
    jmp .print_char

.done:
    pop rsi
    pop rdi
    pop rcx
    pop rax
    ret

; Print line with specific color
print_line_color:
    push rax
    push rcx
    push rdi
    push rsi
    push rbx
    
    mov bl, al                  ; Save color in BL
    xor rcx, rcx
.print_char_color:
    mov al, [rsi + rcx]
    test al, al
    jz .done_color
    
    cmp rdi, 0xB8FA0
    jge .done_color
    
    mov [rdi], al
    mov [rdi + 1], bl           ; Use saved color
    add rdi, 2
    inc rcx
    cmp rcx, 79
    jge .done_color
    jmp .print_char_color

.done_color:
    pop rbx
    pop rsi
    pop rdi
    pop rcx
    pop rax
    ret

; Installation delay simulation
install_delay:
    push rcx
    mov rcx, 80000000           ; Slower delay for realism
.delay_loop:
    nop
    dec rcx
    jnz .delay_loop
    pop rcx
    ret

; Quick delay
quick_delay:
    push rcx
    mov rcx, 30000000
.quick_loop:
    nop
    dec rcx
    jnz .quick_loop
    pop rcx
    ret

; Clear screen
clear_screen:
    push rax
    push rcx
    push rdi
    
    mov rdi, 0xB8000
    mov rcx, 4000
    mov ax, 0x0720              ; Space with white on black
.clear_loop:
    mov [rdi], ax
    add rdi, 2
    dec rcx
    jnz .clear_loop
    
    pop rdi
    pop rcx
    pop rax
    ret

_start:
    cli                         ; Disable interrupts for safety
    
    ; === STEP 0: Initialize ===
    call clear_screen
    mov rdi, 0xB8000           ; Top of screen
    
    lea rsi, [rel progress_0]
    mov al, 0x0E               ; Yellow
    call print_line_color
    call quick_delay
    
    ; === STEP 1: Welcome Screen ===
    call clear_screen
    mov rdi, 0xB8000
    
    ; Progress
    lea rsi, [rel progress_1]
    mov al, 0x0A               ; Green
    call print_line_color
    add rdi, 160               ; Next line
    add rdi, 160               ; Skip line
    
    ; Title
    lea rsi, [rel title_line]
    mov al, 0x0F               ; Bright white
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel subtitle_line]
    mov al, 0x0C               ; Red
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel divider]
    call print_line
    add rdi, 160
    add rdi, 160
    
    ; Welcome content
    lea rsi, [rel welcome_title]
    mov al, 0x0B               ; Cyan
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel welcome_1]
    call print_line
    add rdi, 160
    
    lea rsi, [rel welcome_2]
    call print_line
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel welcome_3]
    mov al, 0x0E               ; Yellow
    call print_line_color
    
    call install_delay
    
    ; === STEP 2: Disk Detection ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_2]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel disk_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel disk_1]
    call print_line
    add rdi, 160
    
    lea rsi, [rel disk_2]
    mov al, 0x0D               ; Purple
    call print_line_color
    add rdi, 160
    
    call install_delay
    
    lea rsi, [rel disk_3]
    mov al, 0x0C               ; Red
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel disk_5]
    mov al, 0x0E               ; Yellow warning
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel disk_6]
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel disk_7]
    call print_line_color
    
    call install_delay
    
    ; === STEP 3: Partitioning ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_3]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel part_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel part_1]
    mov al, 0x0C               ; Red warning
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel part_2]
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel part_4]
    mov al, 0x0D               ; Purple command
    call print_line_color
    add rdi, 160
    
    call install_delay
    
    lea rsi, [rel part_5]
    mov al, 0x0A               ; Green success
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel part_6]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel part_7]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel part_8]
    call print_line_color
    
    call install_delay
    
    ; === STEP 4: Filesystem ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_4]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel fs_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel fs_1]
    call print_line
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel fs_3]
    mov al, 0x0D
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel fs_4]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel fs_5]
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel fs_7]
    mov al, 0x0D
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel fs_8]
    mov al, 0x0A
    call print_line_color
    
    call install_delay
    
    ; === STEP 5: System Files ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_5]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel files_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel files_1]
    call print_line
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel files_3]
    mov al, 0x0D
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel files_4]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel files_5]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel files_6]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel files_7]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel files_8]
    call print_line_color
    
    call install_delay
    
    ; === STEP 6: Package Manager ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_6]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel pkg_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel pkg_1]
    call print_line
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel pkg_3]
    mov al, 0x0D
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel pkg_4]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel pkg_5]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel pkg_6]
    call print_line_color
    
    call install_delay
    
    ; === STEP 7: Bootloader (CRITICAL) ===
    call clear_screen
    mov rdi, 0xB8000
    
    lea rsi, [rel progress_7]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel boot_title]
    mov al, 0x0B
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel boot_1]
    mov al, 0x0C               ; Red warning
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel boot_3]
    mov al, 0x0D
    call print_line_color
    add rdi, 160
    call install_delay
    call install_delay          ; Extra delay for critical step
    
    lea rsi, [rel boot_4]
    mov al, 0x0A
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel boot_5]
    call print_line_color
    add rdi, 160
    call install_delay
    
    lea rsi, [rel boot_6]
    call print_line_color
    
    call install_delay
    call install_delay
    
    ; === FINAL: Installation Complete ===
    call clear_screen
    mov rdi, 0xB8000
    add rdi, 320               ; Start at line 2
    
    lea rsi, [rel final_title]
    mov al, 0x0F               ; Bright white
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel final_1]
    mov al, 0x0A               ; Green success
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel final_3]
    mov al, 0x0B               ; Cyan
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel final_4]
    mov al, 0x0E               ; Yellow
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel final_5]
    call print_line_color
    add rdi, 160
    
    lea rsi, [rel final_6]
    call print_line_color
    add rdi, 160
    add rdi, 160
    
    lea rsi, [rel final_8]
    mov al, 0x08               ; Gray
    call print_line_color
    
    ; Safe halt
    cli
.final_halt:
    hlt
    jmp .final_halt
