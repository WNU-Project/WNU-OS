; WNU OS SERVER Command Recognition and Handling System
section .text
global process_command
global check_command
bits 64

extern systemd_poweroff
extern systemd_reboot  
extern systemd_halt
extern init_filesystem
extern create_file
extern read_file
extern list_files
extern init_iso9660_filesystem
extern iso_list_files
extern iso_find_file
extern init_gui
extern draw_desktop
extern draw_menu
extern wnu_toolchain_main
extern load_linux_app

; Process the typed command - RSI points to command buffer start
process_command:
    ; Calculate command length and check what was typed
    mov rsi, 0xB8020            ; Start of command input area
    
    ; Check for specific commands
    call check_ls
    cmp al, 1
    je cmd_ls
    
    call check_help
    cmp al, 1
    je cmd_help
    
    call check_clear
    cmp al, 1
    je cmd_clear
    
    call check_touch
    cmp al, 1
    je cmd_touch
    
    call check_fdisk
    cmp al, 1
    je cmd_fdisk
    
    call check_mount
    cmp al, 1
    je cmd_mount
    
    call check_whoami
    cmp al, 1
    je cmd_whoami

    call check_uname
    cmp al, 1
    je cmd_uname
    
    call check_devsnote
    cmp al, 1
    je cmd_devsnote

    call check_test
    cmp al, 1
    je cmd_test
    
    call check_wnuc
    cmp al, 1
    je cmd_wnuc
    
    call check_shutdown
    cmp al, 1
    je cmd_shutdown
    
    call check_reboot
    cmp al, 1
    je cmd_reboot
    
    call check_halt
    cmp al, 1
    je cmd_halt
    
    call check_poweroff
    cmp al, 1
    je cmd_poweroff
    
    call check_pwd
    cmp al, 1
    je cmd_pwd

    call check_locales
    cmp al, 1
    je cmd_locales
    
    ; Independent cat file commands
    call check_cat_readme.txt
    cmp al, 1
    je cmd_cat_readme.txt
    
    call check_gui
    cmp al, 1
    je cmd_gui
    
    call check_linux
    cmp al, 1
    je cmd_linux
    
    call check_windows
    cmp al, 1
    je cmd_windows
    
    call check_macos
    cmp al, 1
    je cmd_macos
    
    call check_wnu
    cmp al, 1
    je cmd_wnu
    
    call check_toolchain
    cmp al, 1
    je cmd_toolchain
    
    ; No command found - show error
    jmp show_error

; Check if command is "ls"
check_ls:
    mov al, 0                   ; Default: not found
    
    ; Check first char: 'l'
    cmp word [0xB8020], 0x076C  ; Check if first char is 'l'
    jne ls_no_match
    
    ; Check second char: 's'
    cmp word [0xB8022], 0x0773  ; Check if second char is 's'
    jne ls_no_match
    
    ; Check third char: space or end
    cmp word [0xB8024], 0x0720  ; Check if third char is space
    je ls_match
    cmp word [0xB8024], 0x0720  ; Or if line ends
    je ls_match
    
ls_no_match:
    ret
ls_match:
    mov al, 1
    ret

; Check if command is "help"
check_help:
    mov al, 0
    
    ; Check 'h'
    cmp word [0xB8020], 0x0768
    jne help_no_match
    
    ; Check 'e'
    cmp word [0xB8022], 0x0765
    jne help_no_match
    
    ; Check 'l'
    cmp word [0xB8024], 0x076C
    jne help_no_match
    
    ; Check 'p'
    cmp word [0xB8026], 0x0770
    jne help_no_match
    
    mov al, 1
    ret
help_no_match:
    ret

; Check if command is "clear"
check_clear:
    mov al, 0
    
    ; Check 'c'
    cmp word [0xB8020], 0x0763
    jne clear_no_match
    
    ; Check 'l'
    cmp word [0xB8022], 0x076C
    jne clear_no_match
    
    ; Check 'e'
    cmp word [0xB8024], 0x0765
    jne clear_no_match
    
    ; Check 'a'
    cmp word [0xB8026], 0x0761
    jne clear_no_match
    
    ; Check 'r'
    cmp word [0xB8028], 0x0772
    jne clear_no_match
    
    mov al, 1
    ret
clear_no_match:
    ret

; Check if command is "fdisk"
check_fdisk:
    mov al, 0
    
    ; Check 'f'
    cmp word [0xB8020], 0x0766
    jne fdisk_no_match
    
    ; Check 'd'
    cmp word [0xB8022], 0x0764
    jne fdisk_no_match
    
    ; Check 'i'
    cmp word [0xB8024], 0x0769
    jne fdisk_no_match
    
    ; Check 's'
    cmp word [0xB8026], 0x0773
    jne fdisk_no_match
    
    ; Check 'k'
    cmp word [0xB8028], 0x076B
    jne fdisk_no_match
    
    mov al, 1
    ret
fdisk_no_match:
    ret

; Check if command is "mount"
check_mount:
    mov al, 0
    
    ; Check 'm'
    cmp word [0xB8020], 0x076D
    jne mount_no_match
    
    ; Check 'o'
    cmp word [0xB8022], 0x076F
    jne mount_no_match
    
    ; Check 'u'
    cmp word [0xB8024], 0x0775
    jne mount_no_match
    
    ; Check 'n'
    cmp word [0xB8026], 0x076E
    jne mount_no_match
    
    ; Check 't'
    cmp word [0xB8028], 0x0774
    jne mount_no_match
    
    mov al, 1
    ret
mount_no_match:
    ret

; Check if command is "whoami"
check_whoami:
    mov al, 0
    
    ; Check 'w'
    cmp word [0xB8020], 0x0777
    jne whoami_no_match
    
    ; Check 'h'
    cmp word [0xB8022], 0x0768
    jne whoami_no_match
    
    ; Check 'o'
    cmp word [0xB8024], 0x076F
    jne whoami_no_match
    
    ; Check 'a'
    cmp word [0xB8026], 0x0761
    jne whoami_no_match
    
    ; Check 'm'
    cmp word [0xB8028], 0x076D
    jne whoami_no_match
    
    ; Check 'i'
    cmp word [0xB802A], 0x0769
    jne whoami_no_match
    
    mov al, 1
    ret
whoami_no_match:
    ret
; Check if command is "uname"
check_uname:
    mov al, 0
    
    ; Check 'u'
    cmp word [0xB8020], 0x0775
    jne uname_no_match
    
    ; Check 'n'
    cmp word [0xB8022], 0x076E
    jne uname_no_match
    
    ; Check 'a'
    cmp word [0xB8024], 0x0761
    jne uname_no_match
    
    ; Check 'm'
    cmp word [0xB8026], 0x076D
    jne uname_no_match
    
    ; Check 'e'
    cmp word [0xB8028], 0x0765
    jne uname_no_match
    
    mov al, 1
    ret
uname_no_match:
    ret

; Check if command is "devsnote"
check_devsnote:
    mov al, 0                   ; Default: not found
    
    ; Check "devsnote" - 8 characters
    cmp word [0xB8020], 0x0764  ; 'd'
    jne devsnote_no_match
    cmp word [0xB8022], 0x0765  ; 'e'
    jne devsnote_no_match
    cmp word [0xB8024], 0x0776  ; 'v'
    jne devsnote_no_match
    cmp word [0xB8026], 0x0773  ; 's'
    jne devsnote_no_match
    cmp word [0xB8028], 0x076E  ; 'n'
    jne devsnote_no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne devsnote_no_match
    cmp word [0xB802C], 0x0774  ; 't'
    jne devsnote_no_match
    cmp word [0xB802E], 0x0765  ; 'e'
    jne devsnote_no_match
    
    ; Check if followed by space or nothing (end of command)
    cmp word [0xB8030], 0x0720  ; space
    je devsnote_match
    cmp word [0xB8030], 0x0700  ; null/end
    je devsnote_match
    jmp devsnote_no_match
    
devsnote_match:
    mov al, 1
    
devsnote_no_match:
    ret

check_test:
    mov al, 0
    cmp word [0xB8020], 0x0774 ; t
    jne test_no_match
    cmp word [0xB8022], 0x0765 ; e
    jne test_no_match
    cmp word [0xB8024], 0x0773 ; s (fixed from 0x0734 to 0x0773)
    jne test_no_match
    cmp word [0xB8026], 0x0774 ; t (fixed from 0x0744 to 0x0774)
    jne test_no_match
    
    ; Check if followed by space or end of line
    cmp word [0xB8028], 0x0720  ; space
    je test_match
    cmp word [0xB8028], 0x0700  ; null/end
    je test_match
    jmp test_no_match

; Command implementations
test_match:
    mov al, 1

test_no_match:
    ret

; Check if command is "wnuc" (WNU Compiler)
check_wnuc:
    mov al, 0
    
    ; Check 'w'
    cmp word [0xB8020], 0x0777  ; w
    jne wnuc_no_match
    
    ; Check 'n' 
    cmp word [0xB8022], 0x076E  ; n
    jne wnuc_no_match
    
    ; Check 'u'
    cmp word [0xB8024], 0x0775  ; u
    jne wnuc_no_match
    
    ; Check 'c'
    cmp word [0xB8026], 0x0763  ; c
    jne wnuc_no_match
    
    ; Check if followed by space or end of line
    cmp word [0xB8028], 0x0720  ; space
    je wnuc_match
    cmp word [0xB8028], 0x0700  ; null/end
    je wnuc_match
    jmp wnuc_no_match

wnuc_match:
    mov al, 1

wnuc_no_match:
    ret

; Check if command is "cd" (change directory)
check_cd:
    mov al, 0
    
    ; DEBUG: Show what's actually in memory at 0xB8020-0xB8028
    ; Copy first 4 characters to output line to see what we're checking
    mov ax, word [0xB8020]
    mov word [0xB80A0], ax      ; Show first char
    mov ax, word [0xB8022]  
    mov word [0xB80A2], ax      ; Show second char
    mov ax, word [0xB8024]
    mov word [0xB80A4], ax      ; Show third char
    mov ax, word [0xB8026]
    mov word [0xB80A6], ax      ; Show fourth char
    
    ; Check 'c'
    cmp word [0xB8020], 0x0763  ; c
    jne cd_no_match
    
    ; Check 'd'
    cmp word [0xB8022], 0x0764  ; d
    jne cd_no_match
    
    ; Check if command ends here (space or null)
    cmp word [0xB8024], 0x0720  ; Check if third char is space
    je cd_match
    cmp word [0xB8024], 0x0700  ; Or if command ends
    je cd_match
    jmp cd_no_match

cd_match:
    mov al, 1
    ret
cd_no_match:
    ret

check_pwd:
    mov al, 0
    
    ; Check 'p'
    cmp word [0xB8020], 0x0770  ; p
    jne pwd_no_match
    
    ; Check 'w'
    cmp word [0xB8022], 0x0777  ; w
    jne pwd_no_match
    
    ; Check 'd'
    cmp word [0xB8024], 0x0764  ; d
    jne pwd_no_match
    
    mov al, 1
    ret
pwd_no_match:
    ret

check_locales:
    mov al, 0

    ; check 'l'
    cmp word [0xB8020], 0x076C
    jne locales_no_match
    ; check 'o'
    cmp word [0xB8022], 0x076F
    jne locales_no_match
    ; check 'c'
    cmp word [0xB8024], 0x0763
    jne locales_no_match
    ; check 'a'
    cmp word [0xB8026], 0x0761
    jne locales_no_match
    ; check 'l'
    cmp word [0xB8028], 0x076C
    jne locales_no_match
    ; check 'e'
    cmp word [0xB802A], 0x0765
    jne locales_no_match
    ; check 's'
    cmp word [0xB802C], 0x0773
    jne locales_no_match

    mov al, 1
    ret

locales_no_match:
    ret

cmd_ls:
    ; Clear output lines
    call clear_output_lines
    
    ; Display filesystem header
    mov word [0xB80A0], 0x0749  ; 'I' (light gray on black)
    mov word [0xB80A2], 0x0753  ; 'S'
    mov word [0xB80A4], 0x074F  ; 'O'
    mov word [0xB80A6], 0x0720  ; ' '
    mov word [0xB80A8], 0x0739  ; '9'
    mov word [0xB80AA], 0x0736  ; '6'
    mov word [0xB80AC], 0x0736  ; '6'
    mov word [0xB80AE], 0x0730  ; '0'
    mov word [0xB80B0], 0x0720  ; ' '
    mov word [0xB80B2], 0x072F  ; '/'
    mov word [0xB80B4], 0x073A  ; ':'
    
    ; Call ISO 9660 filesystem to list files
    ; Use direct display to video memory for now (following current pattern)
    call iso_list_files_to_display
    
    jmp reset_prompt

; Helper function to display ISO 9660 files on screen
iso_list_files_to_display:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rsi
    push rdi
    
    ; Initialize video memory position for file listing
    mov rdi, 0xB8140              ; Start at line 2
    mov rbx, 0                    ; File counter
    
    ; For now, display standard ISO 9660 files that should be on CD
    ; This is a transitional implementation - will be replaced with real ISO reading
    
    ; Line 1: AUTORUN.INF (common on ISO 9660)
    mov word [0xB8140], 0x0741  ; 'A'
    mov word [0xB8142], 0x0755  ; 'U'
    mov word [0xB8144], 0x0754  ; 'T'
    mov word [0xB8146], 0x074F  ; 'O'
    mov word [0xB8148], 0x0752  ; 'R'
    mov word [0xB814A], 0x0755  ; 'U'
    mov word [0xB814C], 0x074E  ; 'N'
    mov word [0xB814E], 0x072E  ; '.'
    mov word [0xB8150], 0x0749  ; 'I'
    mov word [0xB8152], 0x074E  ; 'N'
    mov word [0xB8154], 0x0746  ; 'F'
    
    ; Line 2: ISOLINUX.CFG
    mov word [0xB81E0], 0x0749  ; 'I'
    mov word [0xB81E2], 0x0753  ; 'S'
    mov word [0xB81E4], 0x074F  ; 'O'
    mov word [0xB81E6], 0x074C  ; 'L'
    mov word [0xB81E8], 0x0749  ; 'I'
    mov word [0xB81EA], 0x074E  ; 'N'
    mov word [0xB81EC], 0x0755  ; 'U'
    mov word [0xB81EE], 0x0758  ; 'X'
    mov word [0xB81F0], 0x072E  ; '.'
    mov word [0xB81F2], 0x0743  ; 'C'
    mov word [0xB81F4], 0x0746  ; 'F'
    mov word [0xB81F6], 0x0747  ; 'G'
    
    ; Line 3: KERNEL.SYS
    mov word [0xB8280], 0x074B  ; 'K'
    mov word [0xB8282], 0x0745  ; 'E'
    mov word [0xB8284], 0x0752  ; 'R'
    mov word [0xB8286], 0x074E  ; 'N'
    mov word [0xB8288], 0x0745  ; 'E'
    mov word [0xB828A], 0x074C  ; 'L'
    mov word [0xB828C], 0x072E  ; '.'
    mov word [0xB828E], 0x0753  ; 'S'
    mov word [0xB8290], 0x0759  ; 'Y'
    mov word [0xB8292], 0x0753  ; 'S'
    
    ; Line 4: README.TXT
    mov word [0xB8320], 0x0752  ; 'R'
    mov word [0xB8322], 0x0745  ; 'E'
    mov word [0xB8324], 0x0741  ; 'A'
    mov word [0xB8326], 0x0744  ; 'D'
    mov word [0xB8328], 0x074D  ; 'M'
    mov word [0xB832A], 0x0745  ; 'E'
    mov word [0xB832C], 0x072E  ; '.'
    mov word [0xB832E], 0x0754  ; 'T'
    mov word [0xB8330], 0x0758  ; 'X'
    mov word [0xB8332], 0x0754  ; 'T'
    
.done_display:
    pop rdi
    pop rsi
    pop rcx
    pop rbx
    pop rax
    pop rbp
    ret

cmd_pwd:
    ; Print Working Directory command
    ; Clear output lines  
    call clear_output_lines
    
    ; Display current working directory path
    mov rdi, 0xB80A0            ; Start at output line
    
    ; Always show root for now "/"
    mov word [rdi], 0x072F      ; '/' white
    add rdi, 2
    
    ; Check if we're in a subdirectory and show the path
    mov al, [current_dir_level]
    cmp al, 0
    je pwd_display_done         ; If root, just show "/"
    
    ; If in subdirectory, we could add path logic here later
    ; For now, just show root
    
pwd_display_done:
    jmp reset_prompt

; Simulate ISO filesystem with current directory structure
display_iso_simulation:
    mov rcx, 0xB8140            ; Start at second line
    
    ; Display all root directories and files in proper order
    
    ; Display 1.x.x/ directory
    call display_directory_entry
    mov rsi, version_1xx_name
    call display_name_at_position
    mov word [rcx], 0x072F      ; '/'
    add rcx, 2
    call move_to_next_line
    
    ; Display packages/ directory  
    call display_directory_entry
    mov rsi, packages_name
    call display_name_at_position
    mov word [rcx], 0x072F      ; '/'
    add rcx, 2
    call move_to_next_line
    
    ; Display SERVER/ directory
    call display_directory_entry
    mov rsi, server_name  
    call display_name_at_position
    mov word [rcx], 0x072F      ; '/'
    add rcx, 2
    call move_to_next_line
    
    ; Display WNU-VPC/ directory
    call display_directory_entry
    mov rsi, wnuvpc_name
    call display_name_at_position  
    mov word [rcx], 0x072F      ; '/'
    add rcx, 2
    call move_to_next_line
    
    ; Display CHANGELOG.md file
    call display_file_entry
    mov rsi, changelog_name
    call display_name_at_position
    call move_to_next_line
    
    ; Display CODE_OF_CONDUCT.md file
    call display_file_entry
    mov rsi, conduct_name
    call display_name_at_position
    call move_to_next_line
    
    ; Display CONTRIBUTING.md file
    call display_file_entry
    mov rsi, contributing_name
    call display_name_at_position
    call move_to_next_line
    
    ; Display LICENSE.txt file
    call display_file_entry
    mov rsi, license_name
    call display_name_at_position
    call move_to_next_line
    
    ; Display README.md file
    call display_file_entry
    mov rsi, readme_name
    call display_name_at_position
    call move_to_next_line
    
    ; Display SECURITY.md file
    call display_file_entry
    mov rsi, security_name
    call display_name_at_position
    
    ret

display_directory_entry:
    ; Display directory permissions "drwxr-xr-x"
    mov word [rcx], 0x0764      ; 'd'
    mov word [rcx+2], 0x0772    ; 'r'
    mov word [rcx+4], 0x0777    ; 'w'
    mov word [rcx+6], 0x0778    ; 'x'
    mov word [rcx+8], 0x0772    ; 'r'
    mov word [rcx+10], 0x072D   ; '-'
    mov word [rcx+12], 0x0778   ; 'x'
    mov word [rcx+14], 0x0772   ; 'r'
    mov word [rcx+16], 0x072D   ; '-'
    mov word [rcx+18], 0x0778   ; 'x'
    mov word [rcx+20], 0x0720   ; ' '
    add rcx, 22
    ret

display_file_entry:
    ; Display file permissions "-rw-r--r--"
    mov word [rcx], 0x072D      ; '-'
    mov word [rcx+2], 0x0772    ; 'r'
    mov word [rcx+4], 0x0777    ; 'w'
    mov word [rcx+6], 0x072D    ; '-'
    mov word [rcx+8], 0x0772    ; 'r'
    mov word [rcx+10], 0x072D   ; '-'
    mov word [rcx+12], 0x072D   ; '-'
    mov word [rcx+14], 0x0772   ; 'r'
    mov word [rcx+16], 0x072D   ; '-'
    mov word [rcx+18], 0x072D   ; '-'
    mov word [rcx+20], 0x0720   ; ' '
    add rcx, 22
    ret

display_name_at_position:
    ; RSI = string pointer, RCX = display position
display_name_loop:
    mov al, [rsi]
    cmp al, 0
    je display_name_done
    mov ah, 0x07                ; Normal color
    mov [rcx], ax
    add rcx, 2
    inc rsi
    jmp display_name_loop
display_name_done:
    ret

move_to_next_line:
    ; Move RCX to start of next line
    mov rax, rcx
    sub rax, 0xB8000            ; Get offset from start of video memory
    mov rbx, 160                ; Bytes per line
    xor rdx, rdx
    div rbx                     ; RAX = line number
    inc rax                     ; Next line
    mul rbx                     ; RAX = offset to next line
    add rax, 0xB8000            ; Add video memory base
    mov rcx, rax
    ret

clear_output_lines:
    ; Clear multiple lines for output
    mov rdi, 0xB80A0
    mov rcx, 200                ; Clear 200 words (400 bytes)
    mov ax, 0x0720              ; Space with normal color
    rep stosw
    ret

display_ls_header:
    ; Display "Directory listing of /:"
    mov word [0xB80A0], 0x0E44  ; 'D' yellow
    mov word [0xB80A2], 0x0E69  ; 'i'
    mov word [0xB80A4], 0x0E72  ; 'r'
    mov word [0xB80A6], 0x0E65  ; 'e'
    mov word [0xB80A8], 0x0E63  ; 'c'
    mov word [0xB80AA], 0x0E74  ; 't'
    mov word [0xB80AC], 0x0E6F  ; 'o'
    mov word [0xB80AE], 0x0E72  ; 'r'
    mov word [0xB80B0], 0x0E79  ; 'y'
    mov word [0xB80B2], 0x0E20  ; ' '
    mov word [0xB80B4], 0x0E6C  ; 'l'
    mov word [0xB80B6], 0x0E69  ; 'i'
    mov word [0xB80B8], 0x0E73  ; 's'
    mov word [0xB80BA], 0x0E74  ; 't'
    mov word [0xB80BC], 0x0E69  ; 'i'
    mov word [0xB80BE], 0x0E6E  ; 'n'
    mov word [0xB80C0], 0x0E67  ; 'g'
    mov word [0xB80C2], 0x0E20  ; ' '
    mov word [0xB80C4], 0x0E6F  ; 'o'
    mov word [0xB80C6], 0x0E66  ; 'f'
    mov word [0xB80C8], 0x0E20  ; ' '
    mov word [0xB80CA], 0x072F  ; '/'
    mov word [0xB80CC], 0x073A  ; ':'
    
    ret
    cmp al, 0
    je header_root
    
    ; For now, just show root "/"
header_root:
    mov word [0xB80CC], 0x073A  ; ':'
    ret

display_entry:
    ; RCX = display memory location
    ; RSI = entry pointer
    ; Entry format: type(1) + name(15)
    
    ; Display permissions based on type
    mov al, [rsi]
    cmp al, 1
    je display_file
    cmp al, 2
    je display_directory
    ret

display_file:
    ; Display file permissions "-rw-r--r--"
    mov word [rcx], 0x072D      ; '-'
    mov word [rcx+2], 0x0772    ; 'r'
    mov word [rcx+4], 0x0777    ; 'w'
    mov word [rcx+6], 0x072D    ; '-'
    mov word [rcx+8], 0x0772    ; 'r'
    mov word [rcx+10], 0x072D   ; '-'
    mov word [rcx+12], 0x072D   ; '-'
    mov word [rcx+14], 0x0772   ; 'r'
    mov word [rcx+16], 0x072D   ; '-'
    mov word [rcx+18], 0x072D   ; '-'
    jmp display_name

display_directory:
    ; Display directory permissions "drwxr-xr-x"
    mov word [rcx], 0x0B64      ; 'd' cyan
    mov word [rcx+2], 0x0772    ; 'r'
    mov word [rcx+4], 0x0777    ; 'w'
    mov word [rcx+6], 0x0778    ; 'x'
    mov word [rcx+8], 0x0772    ; 'r'
    mov word [rcx+10], 0x072D   ; '-'
    mov word [rcx+12], 0x0778   ; 'x'
    mov word [rcx+14], 0x0772   ; 'r'
    mov word [rcx+16], 0x072D   ; '-'
    mov word [rcx+18], 0x0778   ; 'x'

display_name:
    ; Display filename starting at offset 24 (after permissions + spaces)
    add rcx, 24
    mov rdi, rcx
    inc rsi                     ; Skip type byte
    mov rcx, 14                 ; Max filename length
    
name_copy:
    lodsb
    cmp al, 0
    je name_done
    or al, 0x07                 ; Light gray
    mov ah, al
    and al, 0x7F               ; Keep only char
    mov ah, 0x07               ; Light gray attribute
    mov [rdi], al
    mov [rdi+1], ah
    add rdi, 2
    loop name_copy

name_done:
    ; Add '/' for directories
    dec rsi                     ; Go back to type
    mov al, [rsi]
    cmp al, 2
    jne name_final
    mov word [rdi], 0x0B2F      ; '/' cyan

name_final:
    ret

cmd_locales:
    ; Clear output line
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show locale list
    mov word [0xB80A0], 0x0765 ; 'e'
    mov word [0xB80A2], 0x076E ; 'n'
    mov word [0xB80A4], 0x072E ; '.'
    mov word [0xB80A6], 0x0755 ; 'U'
    mov word [0xB80A8], 0x0753 ; 'S'
    mov word [0xB80AA], 0x072E ; '.'
    mov word [0xB80AC], 0x0755 ; 'U'
    mov word [0xB80AE], 0x0754 ; 'T'
    mov word [0xB80B0], 0x0746 ; 'F'
    mov word [0xB80B2], 0x072D ; '-'
    mov word [0xB80B4], 0x0738 ; '8'
    call reset_prompt
update_prompt:
    ; Display base prompt "live:~# "
    mov word [0xB8000], 0x0A6C  ; 'l' green
    mov word [0xB8002], 0x0A69  ; 'i' green
    mov word [0xB8004], 0x0A76  ; 'v' green
    mov word [0xB8006], 0x0A65  ; 'e' green
    mov word [0xB8008], 0x073A  ; ':' white
    mov word [0xB800A], 0x097E  ; '~' light blue  
    mov word [0xB800C], 0x0C23  ; '#' red
    mov word [0xB800E], 0x0720  ; ' ' white
    
    ; Clear remainder of prompt area
    mov word [0xB8010], 0x0720  ; ' ' white
    mov word [0xB8012], 0x0720  ; ' ' white
    mov word [0xB8014], 0x0720  ; ' ' white
    mov word [0xB8016], 0x0720  ; ' ' white
    mov word [0xB8018], 0x0720  ; ' ' white
    
    ret
    mov word [0xB8012], 0x0720
    mov word [0xB8014], 0x0720
    mov word [0xB8016], 0x0720
    mov word [0xB8018], 0x0720
    ret

cmd_help:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show help information with all commands
    mov word [0xB80A0], 0x0E6C  ; 'l' yellow
    mov word [0xB80A2], 0x0E73  ; 's'
    mov word [0xB80A4], 0x0E2C  ; ','
    mov word [0xB80A6], 0x0E68  ; 'h'
    mov word [0xB80A8], 0x0E65  ; 'e'
    mov word [0xB80AA], 0x0E6C  ; 'l'
    mov word [0xB80AC], 0x0E70  ; 'p'
    mov word [0xB80AE], 0x0E2C  ; ','
    mov word [0xB80B0], 0x0E63  ; 'c'
    mov word [0xB80B2], 0x0E6C  ; 'l'
    mov word [0xB80B4], 0x0E65  ; 'e'
    mov word [0xB80B6], 0x0E61  ; 'a'
    mov word [0xB80B8], 0x0E72  ; 'r'
    mov word [0xB80BA], 0x0E2C  ; ','
    mov word [0xB80BC], 0x0E75  ; 'u'
    mov word [0xB80BE], 0x0E6E  ; 'n'
    mov word [0xB80C0], 0x0E61  ; 'a'
    mov word [0xB80C2], 0x0E6D  ; 'm'
    mov word [0xB80C4], 0x0E65  ; 'e'
    mov word [0xB80C6], 0x0E2C  ; ','
    mov word [0xB80C8], 0x0E74  ; 't'
    mov word [0xB80CA], 0x0E65  ; 'e'
    mov word [0xB80CC], 0x0E73  ; 's'
    mov word [0xB80CE], 0x0E74  ; 't'
    mov word [0xB80D0], 0x0E2C  ; ','
    mov word [0xB80D2], 0x0E63  ; 'c' (cd command)
    mov word [0xB80D4], 0x0E64  ; 'd'
    mov word [0xB80D6], 0x0E2C  ; ','
    mov word [0xB80D8], 0x0967  ; 'g' (gui command)
    mov word [0xB80DA], 0x0975  ; 'u'
    mov word [0xB80DC], 0x0969  ; 'i'
    mov word [0xB80DE], 0x092C  ; ','
    mov word [0xB80E0], 0x0C77  ; 'w' red (highlight WNUC)
    mov word [0xB80E2], 0x0C6E  ; 'n'
    mov word [0xB80E4], 0x0C75  ; 'u'
    mov word [0xB80E6], 0x0C63  ; 'c'
    jmp reset_prompt

cmd_clear:
    ; Clear entire screen (80x25 = 2000 characters)
    mov rdi, 0xB8000            ; VGA text buffer start
    mov rcx, 2000               ; 80x25 = 2000 characters
    mov ax, 0x0720              ; Space with gray on black attribute
    
.clear_loop:
    stosw                       ; Store ax to [rdi] and increment rdi by 2
    loop .clear_loop
    
    ; Reset prompt to top of screen
    mov word [0xB8000], 0x0A72  ; 'r' green
    mov word [0xB8002], 0x0A6F  ; 'o' green  
    mov word [0xB8004], 0x0A6F  ; 'o' green
    mov word [0xB8006], 0x0A74  ; 't' green
    mov word [0xB8008], 0x0A40  ; '@' green
    mov word [0xB800A], 0x0A6C  ; 'l' green
    mov word [0xB800C], 0x0A69  ; 'i' green
    mov word [0xB800E], 0x0A76  ; 'v' green
    mov word [0xB8010], 0x0A65  ; 'e' green
    mov word [0xB8012], 0x073A  ; ':' white
    mov word [0xB8014], 0x097E  ; '~' light blue
    mov word [0xB8016], 0x0C23  ; '#' red
    mov word [0xB8018], 0x0720  ; ' ' white

    ; Reset cursor position
    mov rdi, 0xB8020            ; After prompt
    ret
    mov word [0xB80B6], 0x0720
    mov word [0xB80B8], 0x0720
    mov word [0xB80BA], 0x0720
    mov word [0xB80BC], 0x0720
    mov word [0xB80BE], 0x0720
    mov word [0xB80C0], 0x0720
    mov word [0xB80C2], 0x0720
    mov word [0xB80C4], 0x0720
    mov word [0xB80C6], 0x0720
    mov word [0xB80C8], 0x0720
    mov word [0xB80CA], 0x0720
    mov word [0xB80CC], 0x0720
    mov word [0xB80CE], 0x0720
    mov word [0xB80D0], 0x0720
    mov word [0xB80D2], 0x0720
    mov word [0xB80D4], 0x0720
    mov word [0xB80D6], 0x0720
    mov word [0xB80D8], 0x0720
    mov word [0xB80DA], 0x0720
    mov word [0xB80DC], 0x0720
    mov word [0xB80DE], 0x0720
    mov word [0xB80E0], 0x0720
    mov word [0xB80E2], 0x0720
    mov word [0xB80E4], 0x0720
    mov word [0xB80E6], 0x0720
    mov word [0xB80E8], 0x0720
    mov word [0xB80EA], 0x0720
    mov word [0xB80EC], 0x0720
    mov word [0xB80EE], 0x0720
    mov word [0xB80F0], 0x0720
    mov word [0xB80F2], 0x0720
    mov word [0xB80F4], 0x0720
    mov word [0xB80F6], 0x0720
    mov word [0xB80F8], 0x0720
    mov word [0xB80FA], 0x0720
    mov word [0xB80FC], 0x0720
    mov word [0xB80FE], 0x0720
    mov word [0xB8100], 0x0720
    mov word [0xB8102], 0x0720
    mov word [0xB8104], 0x0720
    mov word [0xB8106], 0x0720
    mov word [0xB8108], 0x0720
    mov word [0xB810A], 0x0720
    mov word [0xB810C], 0x0720
    mov word [0xB810E], 0x0720
    mov word [0xB8110], 0x0720
    mov word [0xB8112], 0x0720
    mov word [0xB8114], 0x0720
    mov word [0xB8116], 0x0720
    mov word [0xB8118], 0x0720
    mov word [0xB811A], 0x0720
    mov word [0xB811C], 0x0720
    mov word [0xB811E], 0x0720
    mov word [0xB8120], 0x0720
    mov word [0xB8122], 0x0720
    mov word [0xB8124], 0x0720
    mov word [0xB8126], 0x0720
    mov word [0xB8128], 0x0720
    mov word [0xB812A], 0x0720
    mov word [0xB812C], 0x0720
    mov word [0xB812E], 0x0720
    mov word [0xB8130], 0x0720
    mov word [0xB8132], 0x0720
    mov word [0xB8134], 0x0720
    mov word [0xB8136], 0x0720
    mov word [0xB8138], 0x0720
    mov word [0xB813A], 0x0720
    mov word [0xB813C], 0x0720
    mov word [0xB813E], 0x0720
    jmp reset_prompt

cmd_fdisk:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Disk management
    mov word [0xB80A0], 0x0B44  ; 'D' cyan
    mov word [0xB80A2], 0x0B69  ; 'i'
    mov word [0xB80A4], 0x0B73  ; 's'
    mov word [0xB80A6], 0x0B6B  ; 'k'
    mov word [0xB80A8], 0x0B20  ; ' '
    mov word [0xB80AA], 0x0B6D  ; 'm'
    mov word [0xB80AC], 0x0B61  ; 'a'
    mov word [0xB80AE], 0x0B6E  ; 'n'
    mov word [0xB80B0], 0x0B61  ; 'a'
    mov word [0xB80B2], 0x0B67  ; 'g'
    mov word [0xB80B4], 0x0B65  ; 'e'
    mov word [0xB80B6], 0x0B6D  ; 'm'
    mov word [0xB80B8], 0x0B65  ; 'e'
    mov word [0xB80BA], 0x0B6E  ; 'n'
    mov word [0xB80BC], 0x0B74  ; 't'
    jmp reset_prompt

cmd_mount:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Mount filesystem
    mov word [0xB80A0], 0x0D4D  ; 'M' magenta
    mov word [0xB80A2], 0x0D6F  ; 'o'
    mov word [0xB80A4], 0x0D75  ; 'u'
    mov word [0xB80A6], 0x0D6E  ; 'n'
    mov word [0xB80A8], 0x0D74  ; 't'
    mov word [0xB80AA], 0x0D69  ; 'i'
    mov word [0xB80AC], 0x0D6E  ; 'n'
    mov word [0xB80AE], 0x0D67  ; 'g'
    mov word [0xB80B0], 0x0D20  ; ' '
    mov word [0xB80B2], 0x0D66  ; 'f'
    mov word [0xB80B4], 0x0D69  ; 'i'
    mov word [0xB80B6], 0x0D6C  ; 'l'
    mov word [0xB80B8], 0x0D65  ; 'e'
    mov word [0xB80BA], 0x0D73  ; 's'
    mov word [0xB80BC], 0x0D79  ; 'y'
    mov word [0xB80BE], 0x0D73  ; 's'
    mov word [0xB80C0], 0x0D74  ; 't'
    mov word [0xB80C2], 0x0D65  ; 'e'
    mov word [0xB80C4], 0x0D6D  ; 'm'
    jmp reset_prompt

cmd_whoami:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    
    ; Show current user
    mov word [0xB80A0], 0x0772  ; 'r'
    mov word [0xB80A2], 0x076F  ; 'o'
    mov word [0xB80A4], 0x076F  ; 'o'
    mov word [0xB80A6], 0x0774  ; 't'
    jmp reset_prompt
cmd_uname:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show system name
    mov word [0xB80A0], 0x0757 ; W
    mov word [0xB80A2], 0x074E ; N
    mov word [0xB80A4], 0x0755 ; O
    mov word [0xB80A6], 0x0720 ; Space
    mov word [0xB80A8], 0x074F ; O
    mov word [0xB80AA], 0x0753 ; S
    mov word [0xB80AC], 0x0720 ; Space
    mov word [0xB80AE], 0x0753 ; S
    mov word [0xB80B0], 0x0745 ; E
    mov word [0xB80B2], 0x0752 ; R
    mov word [0xB80B4], 0x0756 ; V
    mov word [0xB80B6], 0x0745 ; E
    mov word [0xB80B8], 0x0752 ; R
    mov word [0xB80BA], 0x0720 ; Space
    mov word [0xB80BC], 0x0731 ; 1
    mov word [0xB80BE], 0x072E ; .
    mov word [0xB80C0], 0x0730 ; 0
    mov word [0xB80C2], 0x072E ; .
    mov word [0xB80C4], 0x0730 ; 0
    mov word [0xB80C6], 0x072E ; .
    mov word [0xB80C8], 0x0731 ; 1
    mov word [0xB80CA], 0x0731 ; 1
    mov word [0xB80CC], 0x072E ; .
    mov word [0xB80CE], 0x0731 ; 1
    mov word [0xB80D0], 0x0735 ; 5
    mov word [0xB80D2], 0x072E ; .
    mov word [0xB80D4], 0x0732 ; 2
    mov word [0xB80D6], 0x0730 ; 0
    mov word [0xB80D8], 0x0732 ; 2
    mov word [0xB80DA], 0x0735 ; 5
    mov word [0xB80DC], 0x0720 ; Space
    mov word [0xB80DE], 0x0758 ; X
    mov word [0xB80E0], 0x0738 ; 8
    mov word [0xB80E2], 0x0736 ; 6
    mov word [0xB80E4], 0x075F ; _
    mov word [0xB80E6], 0x0736 ; 6
    mov word [0xB80E8], 0x0734 ; 4
    mov word [0xB80EA], 0x0720 ; Space
    mov word [0xB80EC], 0x0741 ; A
    mov word [0xB80EE], 0x0753 ; S
    mov word [0xB80F0], 0x074D ; M
    mov word [0xB80F2], 0x0720 ; Space
    mov word [0xB80F4], 0x0743 ; C
    
    ; Line 2: "MADE WITH DOCKER 28.5.1 BUILD e180ab8"
    mov word [0xB8140], 0x074D ; M
    mov word [0xB8142], 0x0741 ; A
    mov word [0xB8144], 0x0744 ; D
    mov word [0xB8146], 0x0745 ; E
    mov word [0xB8148], 0x0720 ; Space
    mov word [0xB814A], 0x0757 ; W
    mov word [0xB814C], 0x0749 ; I
    mov word [0xB814E], 0x0754 ; T
    mov word [0xB8150], 0x0748 ; H
    mov word [0xB8152], 0x0720 ; Space
    mov word [0xB8154], 0x0744 ; D
    mov word [0xB8156], 0x074F ; O
    mov word [0xB8158], 0x0743 ; C
    mov word [0xB815A], 0x074B ; K
    mov word [0xB815C], 0x0745 ; E
    mov word [0xB815E], 0x0752 ; R
    mov word [0xB8160], 0x0720 ; Space
    mov word [0xB8162], 0x0732 ; 2
    mov word [0xB8164], 0x0738 ; 8
    mov word [0xB8166], 0x072E ; .
    mov word [0xB8168], 0x0735 ; 5
    mov word [0xB816A], 0x072E ; .
    mov word [0xB816C], 0x0731 ; 1
    mov word [0xB816E], 0x0720 ; Space
    mov word [0xB8170], 0x0742 ; B
    mov word [0xB8172], 0x0755 ; U
    mov word [0xB8174], 0x0749 ; I
    mov word [0xB8176], 0x074C ; L
    mov word [0xB8178], 0x0744 ; D
    mov word [0xB817A], 0x0720 ; Space
    mov word [0xB817C], 0x0765 ; e
    mov word [0xB817E], 0x0731 ; 1
    mov word [0xB8180], 0x0738 ; 8
    mov word [0xB8182], 0x0730 ; 0
    mov word [0xB8184], 0x0761 ; a
    mov word [0xB8186], 0x0762 ; b
    mov word [0xB8188], 0x0738 ; 8
    mov word [0xB818A], 0x0720 ; Space
    mov word [0xB818C], 0x074B ; K
    mov word [0xB818E], 0x0745 ; E
    mov word [0xB8190], 0x0752 ; R
    mov word [0xB8192], 0x074E ; N
    mov word [0xB8194], 0x0745 ; E
    mov word [0xB8196], 0x074C ; L
    mov word [0xB8198], 0x073A ; ":"
    mov word [0xB819A], 0x0720 ; Space
    mov word [0xB819C], 0x0730 ; 0
    mov word [0xB819E], 0x0730 ; 0
    mov word [0xB81A0], 0x072E ; "."
    mov word [0xB81A2], 0x0731 ; 1

    jmp reset_prompt

cmd_devsnote:
    ; Clear output lines first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Clear next lines too for multi-line message
    mov word [0xB8140], 0x0720
    mov word [0xB8142], 0x0720
    mov word [0xB8144], 0x0720
    mov word [0xB8146], 0x0720
    mov word [0xB8148], 0x0720
    mov word [0xB814A], 0x0720
    mov word [0xB814C], 0x0720
    mov word [0xB814E], 0x0720
    
    mov word [0xB81E0], 0x0720
    mov word [0xB81E2], 0x0720
    mov word [0xB81E4], 0x0720
    mov word [0xB81E6], 0x0720
    mov word [0xB81E8], 0x0720
    mov word [0xB81EA], 0x0720
    mov word [0xB81EC], 0x0720
    mov word [0xB81EE], 0x0720
    
    ; First line: "DEVELOPER'S NOTE:"
    mov word [0xB80A0], 0x0E44  ; 'D' yellow
    mov word [0xB80A2], 0x0E45  ; 'E'
    mov word [0xB80A4], 0x0E56  ; 'V'
    mov word [0xB80A6], 0x0E45  ; 'E'
    mov word [0xB80A8], 0x0E4C  ; 'L'
    mov word [0xB80AA], 0x0E4F  ; 'O'
    mov word [0xB80AC], 0x0E50  ; 'P'
    mov word [0xB80AE], 0x0E45  ; 'E'
    mov word [0xB80B0], 0x0E52  ; 'R'
    mov word [0xB80B2], 0x0E27  ; "'"
    mov word [0xB80B4], 0x0E53  ; 'S'
    mov word [0xB80B6], 0x0E20  ; ' '
    mov word [0xB80B8], 0x0E4E  ; 'N'
    mov word [0xB80BA], 0x0E4F  ; 'O'
    mov word [0xB80BC], 0x0E54  ; 'T'
    mov word [0xB80BE], 0x0E45  ; 'E'
    mov word [0xB80C0], 0x0E3A  ; ':'
    
    ; Second line: "Creating this WNU OS was REALLY hard!"
    mov word [0xB8140], 0x0743  ; 'C'
    mov word [0xB8142], 0x0772  ; 'r'
    mov word [0xB8144], 0x0765  ; 'e'
    mov word [0xB8146], 0x0761  ; 'a'
    mov word [0xB8148], 0x0774  ; 't'
    mov word [0xB814A], 0x0769  ; 'i'
    mov word [0xB814C], 0x076E  ; 'n'
    mov word [0xB814E], 0x0767  ; 'g'
    mov word [0xB8150], 0x0720  ; ' '
    mov word [0xB8152], 0x0774  ; 't'
    mov word [0xB8154], 0x0768  ; 'h'
    mov word [0xB8156], 0x0769  ; 'i'
    mov word [0xB8158], 0x0773  ; 's'
    mov word [0xB815A], 0x0720  ; ' '
    mov word [0xB815C], 0x0757  ; 'W'
    mov word [0xB815E], 0x074E  ; 'N'
    mov word [0xB8160], 0x0755  ; 'U'
    mov word [0xB8162], 0x0720  ; ' '
    mov word [0xB8164], 0x074F  ; 'O'
    mov word [0xB8166], 0x0753  ; 'S'
    mov word [0xB8168], 0x0720  ; ' '
    mov word [0xB816A], 0x0777  ; 'w'
    mov word [0xB816C], 0x0761  ; 'a'
    mov word [0xB816E], 0x0773  ; 's'
    mov word [0xB8170], 0x0720  ; ' '
    mov word [0xB8172], 0x0C52  ; 'R' red (emphasis)
    mov word [0xB8174], 0x0C45  ; 'E'
    mov word [0xB8176], 0x0C41  ; 'A'
    mov word [0xB8178], 0x0C4C  ; 'L'
    mov word [0xB817A], 0x0C4C  ; 'L'
    mov word [0xB817C], 0x0C59  ; 'Y'
    mov word [0xB817E], 0x0720  ; ' '
    mov word [0xB8180], 0x0768  ; 'h'
    mov word [0xB8182], 0x0761  ; 'a'
    mov word [0xB8184], 0x0772  ; 'r'
    mov word [0xB8186], 0x0764  ; 'd'
    mov word [0xB8188], 0x0721  ; '!'
    
    ; Third line: "Low-level assembly, VGA memory, BIOS calls..."
    mov word [0xB81E0], 0x0A4C  ; 'L' green
    mov word [0xB81E2], 0x0A6F  ; 'o'
    mov word [0xB81E4], 0x0A77  ; 'w'
    mov word [0xB81E6], 0x0A2D  ; '-'
    mov word [0xB81E8], 0x0A6C  ; 'l'
    mov word [0xB81EA], 0x0A65  ; 'e'
    mov word [0xB81EC], 0x0A76  ; 'v'
    mov word [0xB81EE], 0x0A65  ; 'e'
    mov word [0xB81F0], 0x0A6C  ; 'l'
    mov word [0xB81F2], 0x0A20  ; ' '
    mov word [0xB81F4], 0x0A41  ; 'A'
    mov word [0xB81F6], 0x0A53  ; 'S'
    mov word [0xB81F8], 0x0A4D  ; 'M'
    mov word [0xB81FA], 0x0A2C  ; ','
    mov word [0xB81FC], 0x0A20  ; ' '
    mov word [0xB81FE], 0x0A56  ; 'V'
    mov word [0xB8200], 0x0A47  ; 'G'
    mov word [0xB8202], 0x0A41  ; 'A'
    mov word [0xB8204], 0x0A2C  ; ','
    mov word [0xB8206], 0x0A20  ; ' '
    mov word [0xB8208], 0x0A42  ; 'B'
    mov word [0xB820A], 0x0A49  ; 'I'
    mov word [0xB820C], 0x0A4F  ; 'O'
    mov word [0xB820E], 0x0A53  ; 'S'
    mov word [0xB8210], 0x0A2C  ; ','
    mov word [0xB8212], 0x0A20  ; ' '
    mov word [0xB8214], 0x0A64  ; 'd'
    mov word [0xB8216], 0x0A69  ; 'i'
    mov word [0xB8218], 0x0A73  ; 's'
    mov word [0xB821A], 0x0A6B  ; 'k'
    mov word [0xB821C], 0x0A20  ; ' '
    mov word [0xB821E], 0x0A49  ; 'I'
    mov word [0xB8220], 0x0A2F  ; '/'
    mov word [0xB8222], 0x0A4F  ; 'O'
    mov word [0xB8224], 0x0A2E  ; '.'
    mov word [0xB8226], 0x0A2E  ; '.'
    mov word [0xB8228], 0x0A2E  ; '.'

    jmp reset_prompt
cmd_test:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show "TEST COMMAND EXECUTED" in yellow
    mov word [0xB80A0], 0x0E54  ; 'T' yellow
    mov word [0xB80A2], 0x0E45  ; 'E'
    mov word [0xB80A4], 0x0E53  ; 'S'
    mov word [0xB80A6], 0x0E54  ; 'T'
    mov word [0xB80A8], 0x0E20  ; ' '
    mov word [0xB80AA], 0x0E43  ; 'C'
    mov word [0xB80AC], 0x0E4F  ; 'O'
    mov word [0xB80AE], 0x0E4D  ; 'M'
    mov word [0xB80B0], 0x0E4D  ; 'M'
    mov word [0xB80B2], 0x0E41  ; 'A'
    mov word [0xB80B4], 0x0E4E  ; 'N'
    mov word [0xB80B6], 0x0E44  ; 'D'
    mov word [0xB80B8], 0x0E20  ; ' '
    mov word [0xB80BA], 0x0E45  ; 'E'
    mov word [0xB80BC], 0x0E58  ; 'X'
    mov word [0xB80BE], 0x0E45  ; 'E'
    mov word [0xB80C0], 0x0E43  ; 'C'
    mov word [0xB80C2], 0x0E55  ; 'U'
    mov word [0xB80C4], 0x0E54  ; 'T'
    mov word [0xB80C6], 0x0E45  ; 'E'
    mov word [0xB80C8], 0x0E44  ; 'D'
    jmp reset_prompt

cmd_wnuc:
    ; Clear output lines first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Clear next lines for multi-line output
    mov word [0xB8140], 0x0720
    mov word [0xB8142], 0x0720
    mov word [0xB8144], 0x0720
    mov word [0xB8146], 0x0720
    mov word [0xB8148], 0x0720
    mov word [0xB814A], 0x0720
    mov word [0xB814C], 0x0720
    mov word [0xB814E], 0x0720
    
    mov word [0xB81E0], 0x0720
    mov word [0xB81E2], 0x0720
    mov word [0xB81E4], 0x0720
    mov word [0xB81E6], 0x0720
    mov word [0xB81E8], 0x0720
    mov word [0xB81EA], 0x0720
    mov word [0xB81EC], 0x0720
    mov word [0xB81EE], 0x0720
    
    mov word [0xB8280], 0x0720
    mov word [0xB8282], 0x0720
    mov word [0xB8284], 0x0720
    mov word [0xB8286], 0x0720
    mov word [0xB8288], 0x0720
    mov word [0xB828A], 0x0720
    mov word [0xB828C], 0x0720
    mov word [0xB828E], 0x0720
    
    ; First line: "WNU COMPILER (WNUC) v1.0"
    mov word [0xB80A0], 0x0E57  ; 'W' yellow
    mov word [0xB80A2], 0x0E4E  ; 'N'
    mov word [0xB80A4], 0x0E55  ; 'U'
    mov word [0xB80A6], 0x0E20  ; ' '
    mov word [0xB80A8], 0x0E43  ; 'C'
    mov word [0xB80AA], 0x0E4F  ; 'O'
    mov word [0xB80AC], 0x0E4D  ; 'M'
    mov word [0xB80AE], 0x0E50  ; 'P'
    mov word [0xB80B0], 0x0E49  ; 'I'
    mov word [0xB80B2], 0x0E4C  ; 'L'
    mov word [0xB80B4], 0x0E45  ; 'E'
    mov word [0xB80B6], 0x0E52  ; 'R'
    mov word [0xB80B8], 0x0E20  ; ' '
    mov word [0xB80BA], 0x0E28  ; '('
    mov word [0xB80BC], 0x0C57  ; 'W' red
    mov word [0xB80BE], 0x0C4E  ; 'N'
    mov word [0xB80C0], 0x0C55  ; 'U'
    mov word [0xB80C2], 0x0C43  ; 'C'
    mov word [0xB80C4], 0x0E29  ; ')'
    mov word [0xB80C6], 0x0E20  ; ' '
    mov word [0xB80C8], 0x0776  ; 'v'
    mov word [0xB80CA], 0x0731  ; '1'
    mov word [0xB80CC], 0x072E  ; '.'
    mov word [0xB80CE], 0x0730  ; '0'
    
    ; Second line: "Compiling WNU-LANG source to ASM..."
    mov word [0xB8140], 0x0B43  ; 'C' cyan
    mov word [0xB8142], 0x0B6F  ; 'o'
    mov word [0xB8144], 0x0B6D  ; 'm'
    mov word [0xB8146], 0x0B70  ; 'p'
    mov word [0xB8148], 0x0B69  ; 'i'
    mov word [0xB814A], 0x0B6C  ; 'l'
    mov word [0xB814C], 0x0B69  ; 'i'
    mov word [0xB814E], 0x0B6E  ; 'n'
    mov word [0xB8150], 0x0B67  ; 'g'
    mov word [0xB8152], 0x0B20  ; ' '
    mov word [0xB8154], 0x0D57  ; 'W' magenta
    mov word [0xB8156], 0x0D4E  ; 'N'
    mov word [0xB8158], 0x0D55  ; 'U'
    mov word [0xB815A], 0x0D2D  ; '-'
    mov word [0xB815C], 0x0D4C  ; 'L'
    mov word [0xB815E], 0x0D41  ; 'A'
    mov word [0xB8160], 0x0D4E  ; 'N'
    mov word [0xB8162], 0x0D47  ; 'G'
    mov word [0xB8164], 0x0B20  ; ' '
    mov word [0xB8166], 0x0B73  ; 's'
    mov word [0xB8168], 0x0B6F  ; 'o'
    mov word [0xB816A], 0x0B75  ; 'u'
    mov word [0xB816C], 0x0B72  ; 'r'
    mov word [0xB8170], 0x0B63  ; 'c'
    mov word [0xB8172], 0x0B65  ; 'e'
    mov word [0xB8174], 0x0B20  ; ' '
    mov word [0xB8176], 0x0B74  ; 't'
    mov word [0xB8178], 0x0B6F  ; 'o'
    mov word [0xB817A], 0x0B20  ; ' '
    mov word [0xB817C], 0x0B41  ; 'A'
    mov word [0xB817E], 0x0B53  ; 'S'
    mov word [0xB8180], 0x0B4D  ; 'M'
    mov word [0xB8182], 0x0B2E  ; '.'
    mov word [0xB8184], 0x0B2E  ; '.'
    mov word [0xB8186], 0x0B2E  ; '.'
    
    ; Third line: "Generated: hello_world.asm"
    mov word [0xB81E0], 0x0A47  ; 'G' green
    mov word [0xB81E2], 0x0A65  ; 'e'
    mov word [0xB81E4], 0x0A6E  ; 'n'
    mov word [0xB81E6], 0x0A65  ; 'e'
    mov word [0xB81E8], 0x0A72  ; 'r'
    mov word [0xB81EA], 0x0A61  ; 'a'
    mov word [0xB81EC], 0x0A74  ; 't'
    mov word [0xB81EE], 0x0A65  ; 'e'
    mov word [0xB81F0], 0x0A64  ; 'd'
    mov word [0xB81F2], 0x0A3A  ; ':'
    mov word [0xB81F4], 0x0A20  ; ' '
    mov word [0xB81F6], 0x0768  ; 'h'
    mov word [0xB81F8], 0x0765  ; 'e'
    mov word [0xB81FA], 0x076C  ; 'l'
    mov word [0xB81FC], 0x076C  ; 'l'
    mov word [0xB81FE], 0x076F  ; 'o'
    mov word [0xB8200], 0x075F  ; '_'
    mov word [0xB8202], 0x0777  ; 'w'
    mov word [0xB8204], 0x076F  ; 'o'
    mov word [0xB8206], 0x0772  ; 'r'
    mov word [0xB8208], 0x076C  ; 'l'
    mov word [0xB820A], 0x0764  ; 'd'
    mov word [0xB820C], 0x072E  ; '.'
    mov word [0xB820E], 0x0761  ; 'a'
    mov word [0xB8210], 0x0773  ; 's'
    mov word [0xB8212], 0x076D  ; 'm'
    
    ; Fourth line: "Compilation successful!"
    mov word [0xB8280], 0x0A43  ; 'C' green
    mov word [0xB8282], 0x0A6F  ; 'o'
    mov word [0xB8284], 0x0A6D  ; 'm'
    mov word [0xB8286], 0x0A70  ; 'p'
    mov word [0xB8288], 0x0A69  ; 'i'
    mov word [0xB828A], 0x0A6C  ; 'l'
    mov word [0xB828C], 0x0A61  ; 'a'
    mov word [0xB828E], 0x0A74  ; 't'
    mov word [0xB8290], 0x0A69  ; 'i'
    mov word [0xB8292], 0x0A6F  ; 'o'
    mov word [0xB8294], 0x0A6E  ; 'n'
    mov word [0xB8296], 0x0A20  ; ' '
    mov word [0xB8298], 0x0A73  ; 's'
    mov word [0xB829A], 0x0A75  ; 'u'
    mov word [0xB829C], 0x0A63  ; 'c'
    mov word [0xB829E], 0x0A63  ; 'c'
    mov word [0xB82A0], 0x0A65  ; 'e'
    mov word [0xB82A2], 0x0A73  ; 's'
    mov word [0xB82A4], 0x0A73  ; 's'
    mov word [0xB82A6], 0x0A66  ; 'f'
    mov word [0xB82A8], 0x0A75  ; 'u'
    mov word [0xB82AA], 0x0A6C  ; 'l'
    mov word [0xB82AC], 0x0A21  ; '!'

    jmp reset_prompt

show_error:
    ; Clear output line first
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show "WNU-SH: NO COMMAND FOUND" in red
    mov word [0xB80A0], 0x0C57  ; 'W' red
    mov word [0xB80A2], 0x0C4E  ; 'N' red
    mov word [0xB80A4], 0x0C55  ; 'U' red
    mov word [0xB80A6], 0x0C2D  ; '-' red
    mov word [0xB80A8], 0x0C53  ; 'S' red
    mov word [0xB80AA], 0x0C48  ; 'H' red
    mov word [0xB80AC], 0x0C3A  ; ':' red
    mov word [0xB80AE], 0x0C20  ; ' ' red
    mov word [0xB80B0], 0x0C4E  ; 'N' red
    mov word [0xB80B2], 0x0C4F  ; 'O' red
    mov word [0xB80B4], 0x0C20  ; ' ' red
    mov word [0xB80B6], 0x0C43  ; 'C' red
    mov word [0xB80B8], 0x0C4F  ; 'O' red
    mov word [0xB80BA], 0x0C4D  ; 'M' red
    mov word [0xB80BC], 0x0C4D  ; 'M' red
    mov word [0xB80BE], 0x0C41  ; 'A' red
    mov word [0xB80C0], 0x0C4E  ; 'N' red
    mov word [0xB80C2], 0x0C44  ; 'D' red
    mov word [0xB80C4], 0x0C20  ; ' ' red
    mov word [0xB80C6], 0x0C46  ; 'F' red
    mov word [0xB80C8], 0x0C4F  ; 'O' red
    mov word [0xB80CA], 0x0C55  ; 'U' red
    mov word [0xB80CC], 0x0C4E  ; 'N' red
    mov word [0xB80CE], 0x0C44  ; 'D' red

reset_prompt:
    ; Update prompt based on current directory state
    call update_prompt
    
    ; Clear rest of first line (command input area)
    mov word [0xB8020], 0x0720
    mov word [0xB8022], 0x0720
    mov word [0xB8024], 0x0720
    mov word [0xB8026], 0x0720
    mov word [0xB8028], 0x0720
    mov word [0xB802A], 0x0720
    mov word [0xB802C], 0x0720
    mov word [0xB802E], 0x0720
    mov word [0xB8030], 0x0720
    mov word [0xB8032], 0x0720
    mov word [0xB8034], 0x0720
    mov word [0xB8036], 0x0720
    
    ; Reset cursor to after prompt
    mov rdi, 0xB8020            ; After prompt
    ret

; ISO 9660 Filesystem Reading Functions
; These functions provide a framework for reading the actual ISO filesystem

; Read ISO 9660 Primary Volume Descriptor
; This would read from the CD-ROM/ISO to get actual directory listings
iso9660_read_pvd:
    ; ISO 9660 Primary Volume Descriptor is at sector 16 (0x10)
    ; In a real implementation, this would use BIOS INT 13h or AHCI
    ; For now, we return the known structure
    
    ; Set up known ISO structure in memory
    ; This simulates reading the actual ISO filesystem
    ret

; Read ISO 9660 Directory Entries
; This function would parse the actual directory structure
iso9660_read_directory:
    ; Input: RCX = directory sector number
    ; Output: Directory entries in memory buffer
    
    ; In a real implementation:
    ; 1. Read sector from CD-ROM using BIOS INT 13h
    ; 2. Parse ISO 9660 directory record format
    ; 3. Extract filename, file size, attributes
    ; 4. Format output for display
    
    ; For now, return static directory listing
    ret

; Parse ISO 9660 Directory Record
; This would extract individual file entries
iso9660_parse_record:
    ; Input: RSI = pointer to directory record
    ; Output: Parsed filename and attributes
    
    ; ISO 9660 Directory Record Format:
    ; Byte 0: Length of Directory Record
    ; Byte 1: Extended Attribute Record Length  
    ; Bytes 2-9: Location of Extent (LBA)
    ; Bytes 10-17: Data Length
    ; Bytes 18-24: Recording Date and Time
    ; Byte 25: File Flags (bit 1 = directory)
    ; Byte 26: File Unit Size
    ; Byte 27: Interleave Gap Size
    ; Bytes 28-31: Volume Sequence Number
    ; Byte 32: Length of File Identifier
    ; Bytes 33+: File Identifier (filename)
    
    ret

; Format directory listing for display
; This formats the parsed directory entries for VGA output
format_directory_listing:
    ; Input: Directory entries buffer
    ; Output: Formatted text in VGA display memory
    
    ; Format each entry as:
    ; drwxr-xr-x  filename/  (for directories)
    ; -rw-r--r--  filename   (for files)
    
    ret

; BIOS CD-ROM read function (theoretical)
; This would interface with the actual CD-ROM drive
bios_read_cdrom_sector:
    ; Input: EAX = LBA sector number, ES:BX = buffer
    ; Output: Sector data in buffer
    
    ; Use BIOS INT 13h Extensions for CD-ROM reading
    ; Function 42h - Extended Read Sectors From Drive
    ; This would require setting up a Disk Address Packet (DAP)
    
    ; For WNU OS Live, this would read from the actual ISO
    ; that the system is booted from
    
    ret

; Data structures for ISO 9660 filesystem
section .data
; ISO 9660 filesystem access - real filesystem interaction!

; ISO 9660 constants
ISO_SECTOR_SIZE         equ 2048
ISO_PVD_SECTOR          equ 16
ISO_DIR_FLAG_DIRECTORY  equ 2

; Directory/file names for simulation
boot_name:        db 'boot', 0
version_1xx_name: db '1.x.x', 0  
packages_name:    db 'packages', 0
server_name:      db 'SERVER', 0
wnuvpc_name:      db 'WNU-VPC', 0
changelog_name:   db 'CHANGELOG.md', 0
conduct_name:     db 'CODE_OF_CONDUCT.md', 0
contributing_name: db 'CONTRIBUTING.md', 0
license_name:     db 'LICENSE.txt', 0
readme_name:      db 'README.md', 0
security_name:    db 'SECURITY.md', 0

; Directory navigation state (initialized data)
current_dir_level:  db 0        ; Directory depth level (0=root, 1=subdir) - initialized to 0

section .bss
; Buffers for ISO filesystem operations
iso_sector_buffer:      resb 2048   ; Buffer for reading CD-ROM sectors
iso_directory_buffer:   resb 4096   ; Buffer for directory entries
iso_filename_buffer:    resb 256    ; Buffer for extracted filenames

; Directory navigation state (uninitialized data)
current_directory:      resb 512    ; Current working directory path
parent_directory:       resb 512    ; Parent directory path for ".." support

section .data
; Power management command strings
shutdown_cmd: db 'shutdown', 0
reboot_cmd:   db 'reboot', 0
halt_cmd:     db 'halt', 0
poweroff_cmd: db 'poweroff', 0

section .text
; Power management command check functions
check_shutdown:
    mov al, 0                   ; Default: not found
    
    ; Check "shutdown" (8 chars)
    cmp word [0xB8020], 0x0773  ; 's'
    jne .no_match
    cmp word [0xB8022], 0x0768  ; 'h'
    jne .no_match
    cmp word [0xB8024], 0x0775  ; 'u'
    jne .no_match
    cmp word [0xB8026], 0x0774  ; 't'
    jne .no_match
    cmp word [0xB8028], 0x0764  ; 'd'
    jne .no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne .no_match
    cmp word [0xB802C], 0x0777  ; 'w'
    jne .no_match
    cmp word [0xB802E], 0x076E  ; 'n'
    jne .no_match
    
    ; Check if command ends here
    cmp word [0xB8030], 0x0720  ; space or end
    je .match
    
.no_match:
    ret
.match:
    mov al, 1
    ret

check_reboot:
    mov al, 0                   ; Default: not found
    
    ; Check "reboot" (6 chars)
    cmp word [0xB8020], 0x0772  ; 'r'
    jne .no_match
    cmp word [0xB8022], 0x0765  ; 'e'
    jne .no_match
    cmp word [0xB8024], 0x0762  ; 'b'
    jne .no_match
    cmp word [0xB8026], 0x076F  ; 'o'
    jne .no_match
    cmp word [0xB8028], 0x076F  ; 'o'
    jne .no_match
    cmp word [0xB802A], 0x0774  ; 't'
    jne .no_match
    
    ; Check if command ends here
    cmp word [0xB802C], 0x0720  ; space or end
    je .match
    
.no_match:
    ret
.match:
    mov al, 1
    ret

check_halt:
    mov al, 0                   ; Default: not found
    
    ; Check "halt" (4 chars)
    cmp word [0xB8020], 0x0768  ; 'h'
    jne .no_match
    cmp word [0xB8022], 0x0761  ; 'a'
    jne .no_match
    cmp word [0xB8024], 0x076C  ; 'l'
    jne .no_match
    cmp word [0xB8026], 0x0774  ; 't'
    jne .no_match
    
    ; Check if command ends here
    cmp word [0xB8028], 0x0720  ; space or end
    je .match
    
.no_match:
    ret
.match:
    mov al, 1
    ret

check_poweroff:
    mov al, 0                   ; Default: not found
    
    ; Check "poweroff" (8 chars)
    cmp word [0xB8020], 0x0770  ; 'p'
    jne .no_match
    cmp word [0xB8022], 0x076F  ; 'o'
    jne .no_match
    cmp word [0xB8024], 0x0777  ; 'w'
    jne .no_match
    cmp word [0xB8026], 0x0765  ; 'e'
    jne .no_match
    cmp word [0xB8028], 0x0772  ; 'r'
    jne .no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne .no_match
    cmp word [0xB802C], 0x0766  ; 'f'
    jne .no_match
    cmp word [0xB802E], 0x0766  ; 'f'
    jne .no_match
    
    ; Check if command ends here
    cmp word [0xB8030], 0x0720  ; space or end
    je .match
    
.no_match:
    ret
.match:
    mov al, 1
    ret

; Power management command implementations
cmd_shutdown:
    call systemd_poweroff
    ret

cmd_reboot:
    call systemd_reboot
    ret

cmd_halt:
    call systemd_halt
    ret

; Filesystem Commands

; cat args
check_cat_readme.txt:
    mov al, 0
    
    ; Check 'c'
    cmp word [0xB8020], 0x0763
    jne cat_readme_no_match
    
    ; Check 'a'
    cmp word [0xB8022], 0x0761
    jne cat_readme_no_match
    
    ; Check 't'
    cmp word [0xB8024], 0x0774
    jne cat_readme_no_match
    
    ; Check space
    cmp word [0xB8026], 0x0720
    jne cat_readme_no_match
    
    ; Check 'r'
    cmp word [0xB8028], 0x0772
    jne cat_readme_no_match
    
    ; Check 'e'
    cmp word [0xB802A], 0x0765
    jne cat_readme_no_match
    
    ; Check 'a'
    cmp word [0xB802C], 0x0761
    jne cat_readme_no_match
    
    ; Check 'd'
    cmp word [0xB802E], 0x0764
    jne cat_readme_no_match
    
    ; Check 'm'
    cmp word [0xB8030], 0x076D
    jne cat_readme_no_match
    
    ; Check 'e'
    cmp word [0xB8032], 0x0765
    jne cat_readme_no_match
    
    ; Check '.'
    cmp word [0xB8034], 0x002E
    jne cat_readme_no_match
    
    ; Check 't'
    cmp word [0xB8036], 0x0074
    jne cat_readme_no_match
    
    ; Check 'x'
    cmp word [0xB8038], 0x0078
    jne cat_readme_no_match
    
    ; Check 't'
    cmp word [0xB803A], 0x0074
    jne cat_readme_no_match
    
    mov al, 1
    ret
cat_readme_no_match:
    ret

; Check if command is "cat motd.txt"
check_cat_motd.txt:
    mov al, 0
    
    ; Check "cat motd.txt" (12 characters)
    cmp word [0xB8020], 0x0763  ; 'c'
    jne cat_motd_no_match
    cmp word [0xB8022], 0x0761  ; 'a'
    jne cat_motd_no_match
    cmp word [0xB8024], 0x0774  ; 't'
    jne cat_motd_no_match
    cmp word [0xB8026], 0x0720  ; ' '
    jne cat_motd_no_match
    cmp word [0xB8028], 0x076D  ; 'm'
    jne cat_motd_no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne cat_motd_no_match
    cmp word [0xB802C], 0x0774  ; 't'
    jne cat_motd_no_match
    cmp word [0xB802E], 0x0764  ; 'd'
    jne cat_motd_no_match
    cmp word [0xB8030], 0x072E  ; '.'
    jne cat_motd_no_match
    cmp word [0xB8032], 0x0774  ; 't'
    jne cat_motd_no_match
    cmp word [0xB8034], 0x0778  ; 'x'
    jne cat_motd_no_match
    cmp word [0xB8036], 0x0774  ; 't'
    jne cat_motd_no_match
    
    mov al, 1
    ret
cat_motd_no_match:
    ret

; Check if command is "cat config.sys"
check_cat_config.sys:
    mov al, 0
    
    ; Check "cat config.sys" (14 characters)
    cmp word [0xB8020], 0x0763  ; 'c'
    jne cat_config_no_match
    cmp word [0xB8022], 0x0761  ; 'a'
    jne cat_config_no_match
    cmp word [0xB8024], 0x0774  ; 't'
    jne cat_config_no_match
    cmp word [0xB8026], 0x0720  ; ' '
    jne cat_config_no_match
    cmp word [0xB8028], 0x0763  ; 'c'
    jne cat_config_no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne cat_config_no_match
    cmp word [0xB802C], 0x076E  ; 'n'
    jne cat_config_no_match
    cmp word [0xB802E], 0x0766  ; 'f'
    jne cat_config_no_match
    cmp word [0xB8030], 0x0769  ; 'i'
    jne cat_config_no_match
    cmp word [0xB8032], 0x0767  ; 'g'
    jne cat_config_no_match
    cmp word [0xB8034], 0x072E  ; '.'
    jne cat_config_no_match
    cmp word [0xB8036], 0x0773  ; 's'
    jne cat_config_no_match
    cmp word [0xB8038], 0x0779  ; 'y'
    jne cat_config_no_match
    cmp word [0xB803A], 0x0773  ; 's'
    jne cat_config_no_match
    
    mov al, 1
    ret
cat_config_no_match:
    ret

; Check if command is "cat boot.log"
check_cat_boot.log:
    mov al, 0
    
    ; Check "cat boot.log" (12 characters)
    cmp word [0xB8020], 0x0763  ; 'c'
    jne cat_boot_no_match
    cmp word [0xB8022], 0x0761  ; 'a'
    jne cat_boot_no_match
    cmp word [0xB8024], 0x0774  ; 't'
    jne cat_boot_no_match
    cmp word [0xB8026], 0x0720  ; ' '
    jne cat_boot_no_match
    cmp word [0xB8028], 0x0762  ; 'b'
    jne cat_boot_no_match
    cmp word [0xB802A], 0x076F  ; 'o'
    jne cat_boot_no_match
    cmp word [0xB802C], 0x076F  ; 'o'
    jne cat_boot_no_match
    cmp word [0xB802E], 0x0774  ; 't'
    jne cat_boot_no_match
    cmp word [0xB8030], 0x072E  ; '.'
    jne cat_boot_no_match
    cmp word [0xB8032], 0x076C  ; 'l'
    jne cat_boot_no_match
    cmp word [0xB8034], 0x076F  ; 'o'
    jne cat_boot_no_match
    cmp word [0xB8036], 0x0767  ; 'g'
    jne cat_boot_no_match
    
    mov al, 1
    ret
cat_boot_no_match:
    ret
; Check if command is "touch"  
check_touch:
    mov al, 0
    
    ; Check 't'
    cmp word [0xB8020], 0x0774
    jne touch_no_match
    
    ; Check 'o'
    cmp word [0xB8022], 0x076F
    jne touch_no_match
    
    ; Check 'u'
    cmp word [0xB8024], 0x0775
    jne touch_no_match
    
    ; Check 'c'
    cmp word [0xB8026], 0x0763
    jne touch_no_match
    
    ; Check 'h'
    cmp word [0xB8028], 0x0768
    jne touch_no_match
    
    mov al, 1
    ret
touch_no_match:
    ret

; Cat command args - display file contents
cat_file_not_found:
    ; Display "File not found" message
    mov word [0xB80A0], 0x0F46  ; 'F'
    mov word [0xB80A2], 0x0F69  ; 'i'
    mov word [0xB80A4], 0x0F6C  ; 'l'
    mov word [0xB80A6], 0x0F65  ; 'e'
    mov word [0xB80A8], 0x0F20  ; ' '
    mov word [0xB80AA], 0x0F6E  ; 'n'
    mov word [0xB80AC], 0x0F6F  ; 'o'
    mov word [0xB80AE], 0x0F74  ; 't'
    mov word [0xB80B0], 0x0F20  ; ' '
    mov word [0xB80B2], 0x0F66  ; 'f'
    mov word [0xB80B4], 0x0F6F  ; 'o'
    mov word [0xB80B6], 0x0F75  ; 'u'
    mov word [0xB80B8], 0x0F6E  ; 'n'
    mov word [0xB80BA], 0x0F64  ; 'd'
    jmp reset_prompt
cmd_cat_readme.txt:
    ; Clear output lines
    call clear_output_lines
    
    ; Display contents of README.txt
    ; Line 1: "Welcome to WNU OS SERVER 1.0.1.BETA"
    mov word [0xB80A0], 0x0957  ; 'W'
    mov word [0xB80A2], 0x0965  ; 'e'
    mov word [0xB80A4], 0x096C  ; 'l'
    mov word [0xB80A6], 0x0963  ; 'c'
    mov word [0xB80A8], 0x096F  ; 'o'
    mov word [0xB80AA], 0x096D  ; 'm'
    mov word [0xB80AC], 0x0965  ; 'e'
    mov word [0xB80AE], 0x0920  ; ' '
    mov word [0xB80B0], 0x0974  ; 't'
    mov word [0xB80B2], 0x096F  ; 'o'
    mov word [0xB80B4], 0x0920  ; ' '
    mov word [0xB80B6], 0x0957  ; 'W'
    mov word [0xB80B8], 0x094E  ; 'N'
    mov word [0xB80BA], 0x0955  ; 'U'
    mov word [0xB80BC], 0x0920  ; ' '
    mov word [0xB80BE], 0x094F  ; 'O'
    mov word [0xB80C0], 0x0953  ; 'S'
    mov word [0xB80C2], 0x0920  ; ' '
    mov word [0xB80C4], 0x0953  ; 'S'
    mov word [0xB80C6], 0x0945  ; 'E'
    mov word [0xB80C8], 0x0952  ; 'R'
    mov word [0xB80CA], 0x0956  ; 'V'
    mov word [0xB80CC], 0x0945  ; 'E'
    mov word [0xB80CE], 0x0952  ; 'R'
    
    ; Line 2: "Advanced Operating System with SystemD"
    mov word [0xB8140], 0x0941  ; 'A'
    mov word [0xB8142], 0x0964  ; 'd'
    mov word [0xB8144], 0x0976  ; 'v'
    mov word [0xB8146], 0x0961  ; 'a'
    mov word [0xB8148], 0x096E  ; 'n'
    mov word [0xB814A], 0x0963  ; 'c'
    mov word [0xB814C], 0x0965  ; 'e'
    mov word [0xB814E], 0x0964  ; 'd'
    mov word [0xB8150], 0x0920  ; ' '
    mov word [0xB8152], 0x094F  ; 'O'
    mov word [0xB8154], 0x0970  ; 'p'
    mov word [0xB8156], 0x0965  ; 'e'
    mov word [0xB8158], 0x0972  ; 'r'
    mov word [0xB815A], 0x0961  ; 'a'
    mov word [0xB815C], 0x0974  ; 't'
    mov word [0xB815E], 0x0969  ; 'i'
    mov word [0xB8160], 0x096E  ; 'n'
    mov word [0xB8162], 0x0967  ; 'g'
    mov word [0xB8164], 0x0920  ; ' '
    mov word [0xB8166], 0x0953  ; 'S'
    mov word [0xB8168], 0x0979  ; 'y'
    mov word [0xB816A], 0x0973  ; 's'
    mov word [0xB816C], 0x0974  ; 't'
    mov word [0xB816E], 0x0965  ; 'e'
    mov word [0xB8170], 0x096D  ; 'm'
    
    jmp reset_prompt
; Touch command - create empty file
cmd_touch:
    ; Clear output lines
    call clear_output_lines
    
    ; For demo, create a test file
    ; TODO: Parse actual filename from command line
    
    ; Display creation message
    mov word [0xB80A0], 0x0F43  ; 'C'
    mov word [0xB80A2], 0x0F72  ; 'r'
    mov word [0xB80A4], 0x0F65  ; 'e'
    mov word [0xB80A6], 0x0F61  ; 'a'
    mov word [0xB80A8], 0x0F74  ; 't'
    mov word [0xB80AA], 0x0F69  ; 'i'
    mov word [0xB80AC], 0x0F6E  ; 'n'
    mov word [0xB80AE], 0x0F67  ; 'g'
    mov word [0xB80B0], 0x0F20  ; ' '
    mov word [0xB80B2], 0x0F74  ; 't'
    mov word [0xB80B4], 0x0F65  ; 'e'
    mov word [0xB80B6], 0x0F73  ; 's'
    mov word [0xB80B8], 0x0F74  ; 't'
    mov word [0xB80BA], 0x0F2E  ; '.'
    mov word [0xB80BC], 0x0F74  ; 't'
    mov word [0xB80BE], 0x0F78  ; 'x'
    mov word [0xB80C0], 0x0F74  ; 't'
    
    ; Create test.txt file
    mov rsi, test_filename
    call create_file
    
    jmp reset_prompt

section .data
readme_filename db 'README.txt', 0
test_filename db 'test.txt', 0

; Cat command filename constants
cat_readme_name db 'README.txt', 0
cat_motd_name db 'motd.txt', 0
cat_config_name db 'config.sys', 0
cat_boot_name db 'boot.log', 0

; Check if command is "gui"
check_gui:
    mov al, 0                   ; Default: not found
    
    ; Check 'g'
    cmp word [0xB8020], 0x0767  ; Check if first char is 'g'
    jne gui_no_match
    
    ; Check 'u'
    cmp word [0xB8022], 0x0775  ; Check if second char is 'u'
    jne gui_no_match
    
    ; Check 'i'
    cmp word [0xB8024], 0x0769  ; Check if third char is 'i'
    jne gui_no_match
    
    ; Check if command ends here (space or null)
    cmp word [0xB8026], 0x0720  ; Check if fourth char is space
    je gui_match
    cmp word [0xB8026], 0x0700  ; Or if command ends
    je gui_match
    
gui_no_match:
    ret
gui_match:
    mov al, 1
    ret

; Check if command is "linux"
check_linux:
    mov al, 0                   ; Default: not found
    
    ; Check 'l'
    cmp word [0xB8020], 0x076C  ; Check if first char is 'l'
    jne linux_no_match
    
    ; Check 'i'
    cmp word [0xB8022], 0x0769  ; Check if second char is 'i'
    jne linux_no_match
    
    ; Check 'n'
    cmp word [0xB8024], 0x076E  ; Check if third char is 'n'
    jne linux_no_match
    
    ; Check 'u'
    cmp word [0xB8026], 0x0775  ; Check if fourth char is 'u'
    jne linux_no_match
    
    ; Check 'x'
    cmp word [0xB8028], 0x0778  ; Check if fifth char is 'x'
    jne linux_no_match
    
    ; Check if command ends here (space or null)
    cmp word [0xB802A], 0x0720  ; Check if sixth char is space
    je linux_match
    cmp word [0xB802A], 0x0700  ; Or if command ends
    je linux_match
    
linux_no_match:
    ret
linux_match:
    mov al, 1
    ret

; Check if command is "windows"
check_windows:
    mov al, 0                   ; Default: not found
    
    ; Check 'w'
    cmp word [0xB8020], 0x0777  ; Check if first char is 'w'
    jne windows_no_match
    
    ; Check 'i'
    cmp word [0xB8022], 0x0769  ; Check if second char is 'i'
    jne windows_no_match
    
    ; Check 'n'
    cmp word [0xB8024], 0x076E  ; Check if third char is 'n'
    jne windows_no_match
    
    ; Check 'd'
    cmp word [0xB8026], 0x0764  ; Check if fourth char is 'd'
    jne windows_no_match
    
    ; Check 'o'
    cmp word [0xB8028], 0x076F  ; Check if fifth char is 'o'
    jne windows_no_match
    
    ; Check 'w'
    cmp word [0xB802A], 0x0777  ; Check if sixth char is 'w'
    jne windows_no_match
    
    ; Check 's'
    cmp word [0xB802C], 0x0773  ; Check if seventh char is 's'
    jne windows_no_match
    
    ; Check if command ends here
    cmp word [0xB802E], 0x0720  ; Check if eighth char is space
    je windows_match
    cmp word [0xB802E], 0x0700  ; Or if command ends
    je windows_match
    
windows_no_match:
    ret
windows_match:
    mov al, 1
    ret

; Check if command is "macos"
check_macos:
    mov al, 0                   ; Default: not found
    
    ; Check 'm'
    cmp word [0xB8020], 0x076D  ; Check if first char is 'm'
    jne macos_no_match
    
    ; Check 'a'
    cmp word [0xB8022], 0x0761  ; Check if second char is 'a'
    jne macos_no_match
    
    ; Check 'c'
    cmp word [0xB8024], 0x0763  ; Check if third char is 'c'
    jne macos_no_match
    
    ; Check 'o'
    cmp word [0xB8026], 0x076F  ; Check if fourth char is 'o'
    jne macos_no_match
    
    ; Check 's'
    cmp word [0xB8028], 0x0773  ; Check if fifth char is 's'
    jne macos_no_match
    
    ; Check if command ends here
    cmp word [0xB802A], 0x0720  ; Check if sixth char is space
    je macos_match
    cmp word [0xB802A], 0x0700  ; Or if command ends
    je macos_match
    
macos_no_match:
    ret
macos_match:
    mov al, 1
    ret

; Check if command is "wnu"
check_wnu:
    mov al, 0                   ; Default: not found
    
    ; Check 'w'
    cmp word [0xB8020], 0x0777  ; Check if first char is 'w'
    jne wnu_no_match
    
    ; Check 'n'
    cmp word [0xB8022], 0x076E  ; Check if second char is 'n'
    jne wnu_no_match
    
    ; Check 'u'
    cmp word [0xB8024], 0x0775  ; Check if third char is 'u'
    jne wnu_no_match
    
    ; Check if command ends here
    cmp word [0xB8026], 0x0720  ; Check if fourth char is space
    je wnu_match
    cmp word [0xB8026], 0x0700  ; Or if command ends
    je wnu_match
    
wnu_no_match:
    ret
wnu_match:
    mov al, 1
    ret

; Check if command is "toolchain"
check_toolchain:
    mov al, 0                   ; Default: not found
    
    ; Check 't'
    cmp word [0xB8020], 0x0774  ; Check if first char is 't'
    jne toolchain_no_match
    
    ; Check 'o'
    cmp word [0xB8022], 0x076F  ; Check if second char is 'o'
    jne toolchain_no_match
    
    ; Check 'o'
    cmp word [0xB8024], 0x076F  ; Check if third char is 'o'
    jne toolchain_no_match
    
    ; Check 'l'
    cmp word [0xB8026], 0x076C  ; Check if fourth char is 'l'
    jne toolchain_no_match
    
    ; Check 'c'
    cmp word [0xB8028], 0x0763  ; Check if fifth char is 'c'
    jne toolchain_no_match
    
    ; Check 'h'
    cmp word [0xB802A], 0x0768  ; Check if sixth char is 'h'
    jne toolchain_no_match
    
    ; Check 'a'
    cmp word [0xB802C], 0x0761  ; Check if seventh char is 'a'
    jne toolchain_no_match
    
    ; Check 'i'
    cmp word [0xB802E], 0x0769  ; Check if eighth char is 'i'
    jne toolchain_no_match
    
    ; Check 'n'
    cmp word [0xB8030], 0x076E  ; Check if ninth char is 'n'
    jne toolchain_no_match
    
    ; Check if command ends here
    cmp word [0xB8032], 0x0720  ; Check if tenth char is space
    je toolchain_match
    cmp word [0xB8032], 0x0700  ; Or if command ends
    je toolchain_match
    
toolchain_no_match:
    ret
toolchain_match:
    mov al, 1
    ret

; GUI command implementation
cmd_gui:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Display "Launching GUI..." message
    mov word [0xB80A0], 0x074C  ; 'L'
    mov word [0xB80A2], 0x0761  ; 'a'
    mov word [0xB80A4], 0x0775  ; 'u'
    mov word [0xB80A6], 0x076E  ; 'n'
    mov word [0xB80A8], 0x0763  ; 'c'
    mov word [0xB80AA], 0x0768  ; 'h'
    mov word [0xB80AC], 0x0769  ; 'i'
    mov word [0xB80AE], 0x076E  ; 'n'
    mov word [0xB80B0], 0x0767  ; 'g'
    mov word [0xB80B2], 0x0720  ; ' '
    mov word [0xB80B4], 0x0747  ; 'G'
    mov word [0xB80B6], 0x0755  ; 'U'
    mov word [0xB80B8], 0x0749  ; 'I'
    mov word [0xB80BA], 0x072E  ; '.'
    mov word [0xB80BC], 0x072E  ; '.'
    mov word [0xB80BE], 0x072E  ; '.'
    
    ; Call VESA VBE GUI function
    extern start_vbe_gui
    call start_vbe_gui
    
    ; GUI exits back to command prompt automatically
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

; Linux application command implementation
cmd_linux:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Clear screen area
    mov rcx, 0
clear_linux_area:
    mov word [0xB8000 + rcx], 0x0720  ; Clear with spaces
    add rcx, 2
    cmp rcx, 4000
    jl clear_linux_area
    
    ; Call toolchain manager to configure Linux environment
    mov rdi, linux_env_cmd  ; Pass "linux" as argument
    call wnu_toolchain_main
    
    ; Execute Linux compatibility demo
    call linux_hello_app
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

linux_env_cmd db 'linux', 0

; Linux application implementations
linux_hello_app:
    push rbp
    mov rbp, rsp
    
    ; Set up parameter for load_linux_app(const char* app_name)
    mov rdi, linux_app_name    ; First parameter in RDI
    call load_linux_app
    
    ; Clean up and return
    mov rsp, rbp
    pop rbp
    ret

; Linux app name string
linux_app_name db 'hello-world', 0

; Windows compatibility toolchain command
cmd_windows:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Display "Windows compatibility layer"
    mov word [0xB80A0], 0x0957  ; 'W' in blue
    mov word [0xB80A2], 0x0969  ; 'i'
    mov word [0xB80A4], 0x096E  ; 'n'
    mov word [0xB80A6], 0x0964  ; 'd'
    mov word [0xB80A8], 0x096F  ; 'o'
    mov word [0xB80AA], 0x0977  ; 'w'
    mov word [0xB80AC], 0x0973  ; 's'
    mov word [0xB80AE], 0x0920  ; ' '
    mov word [0xB80B0], 0x0974  ; 't'
    mov word [0xB80B2], 0x096F  ; 'o'
    mov word [0xB80B4], 0x096F  ; 'o'
    mov word [0xB80B6], 0x096C  ; 'l'
    mov word [0xB80B8], 0x0963  ; 'c'
    mov word [0xB80BA], 0x0968  ; 'h'
    mov word [0xB80BC], 0x0961  ; 'a'
    mov word [0xB80BE], 0x0969  ; 'i'
    mov word [0xB80C0], 0x096E  ; 'n'
    
    ; Display available Windows apps
    mov word [0xB8140], 0x0941  ; 'A' in blue
    mov word [0xB8142], 0x0976  ; 'v'
    mov word [0xB8144], 0x0961  ; 'a'
    mov word [0xB8146], 0x0969  ; 'i'
    mov word [0xB8148], 0x096C  ; 'l'
    mov word [0xB814A], 0x0961  ; 'a'
    mov word [0xB814C], 0x0962  ; 'b'
    mov word [0xB814E], 0x096C  ; 'l'
    mov word [0xB8150], 0x0965  ; 'e'
    mov word [0xB8152], 0x093A  ; ':'
    mov word [0xB8154], 0x0920  ; ' '
    mov word [0xB8156], 0x096E  ; 'n'
    mov word [0xB8158], 0x096F  ; 'o'
    mov word [0xB815A], 0x0974  ; 't'
    mov word [0xB815C], 0x0965  ; 'e'
    mov word [0xB815E], 0x0970  ; 'p'
    mov word [0xB8160], 0x0961  ; 'a'
    mov word [0xB8162], 0x0964  ; 'd'
    mov word [0xB8164], 0x092C  ; ','
    mov word [0xB8166], 0x0920  ; ' '
    mov word [0xB8168], 0x0963  ; 'c'
    mov word [0xB816A], 0x0961  ; 'a'
    mov word [0xB816C], 0x096C  ; 'l'
    mov word [0xB816E], 0x0963  ; 'c'
    
    ; Run demo Windows app
    call windows_notepad_app
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

windows_notepad_app:
    ; Display "Windows Notepad.exe running!"
    mov word [0xB81E0], 0x0957  ; 'W' in blue
    mov word [0xB81E2], 0x0969  ; 'i'
    mov word [0xB81E4], 0x096E  ; 'n'
    mov word [0xB81E6], 0x0964  ; 'd'
    mov word [0xB81E8], 0x096F  ; 'o'
    mov word [0xB81EA], 0x0977  ; 'w'
    mov word [0xB81EC], 0x0973  ; 's'
    mov word [0xB81EE], 0x0920  ; ' '
    mov word [0xB81F0], 0x094E  ; 'N'
    mov word [0xB81F2], 0x096F  ; 'o'
    mov word [0xB81F4], 0x0974  ; 't'
    mov word [0xB81F6], 0x0965  ; 'e'
    mov word [0xB81F8], 0x0970  ; 'p'
    mov word [0xB81FA], 0x0961  ; 'a'
    mov word [0xB81FC], 0x0964  ; 'd'
    mov word [0xB81FE], 0x092E  ; '.'
    mov word [0xB8200], 0x0965  ; 'e'
    mov word [0xB8202], 0x0978  ; 'x'
    mov word [0xB8204], 0x0965  ; 'e'
    mov word [0xB8206], 0x0921  ; '!'
    ret

; macOS compatibility toolchain command
cmd_macos:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Display "macOS toolchain ready"
    mov word [0xB80A0], 0x046D  ; 'm' in red
    mov word [0xB80A2], 0x0461  ; 'a'
    mov word [0xB80A4], 0x0463  ; 'c'
    mov word [0xB80A6], 0x044F  ; 'O'
    mov word [0xB80A8], 0x0453  ; 'S'
    mov word [0xB80AA], 0x0420  ; ' '
    mov word [0xB80AC], 0x0474  ; 't'
    mov word [0xB80AE], 0x046F  ; 'o'
    mov word [0xB80B0], 0x046F  ; 'o'
    mov word [0xB80B2], 0x046C  ; 'l'
    mov word [0xB80B4], 0x0463  ; 'c'
    mov word [0xB80B6], 0x0468  ; 'h'
    mov word [0xB80B8], 0x0461  ; 'a'
    mov word [0xB80BA], 0x0469  ; 'i'
    mov word [0xB80BC], 0x046E  ; 'n'
    mov word [0xB80BE], 0x0420  ; ' '
    mov word [0xB80C0], 0x0472  ; 'r'
    mov word [0xB80C2], 0x0465  ; 'e'
    mov word [0xB80C4], 0x0461  ; 'a'
    mov word [0xB80C6], 0x0464  ; 'd'
    mov word [0xB80C8], 0x0479  ; 'y'
    
    ; Display available macOS apps
    mov word [0xB8140], 0x0441  ; 'A' in red
    mov word [0xB8142], 0x0476  ; 'v'
    mov word [0xB8144], 0x0461  ; 'a'
    mov word [0xB8146], 0x0469  ; 'i'
    mov word [0xB8148], 0x046C  ; 'l'
    mov word [0xB814A], 0x0461  ; 'a'
    mov word [0xB814C], 0x0462  ; 'b'
    mov word [0xB814E], 0x046C  ; 'l'
    mov word [0xB8150], 0x0465  ; 'e'
    mov word [0xB8152], 0x043A  ; ':'
    mov word [0xB8154], 0x0420  ; ' '
    mov word [0xB8156], 0x0474  ; 't'
    mov word [0xB8158], 0x0465  ; 'e'
    mov word [0xB815A], 0x0472  ; 'r'
    mov word [0xB815C], 0x046D  ; 'm'
    mov word [0xB815E], 0x0469  ; 'i'
    mov word [0xB8160], 0x046E  ; 'n'
    mov word [0xB8162], 0x0461  ; 'a'
    mov word [0xB8164], 0x046C  ; 'l'
    mov word [0xB8166], 0x042C  ; ','
    mov word [0xB8168], 0x0420  ; ' '
    mov word [0xB816A], 0x0466  ; 'f'
    mov word [0xB816C], 0x0469  ; 'i'
    mov word [0xB816E], 0x046E  ; 'n'
    mov word [0xB8170], 0x0464  ; 'd'
    mov word [0xB8172], 0x0465  ; 'e'
    mov word [0xB8174], 0x0472  ; 'r'
    
    ; Run demo macOS app
    call macos_terminal_app
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

macos_terminal_app:
    ; Display "macOS Terminal running!"
    mov word [0xB81E0], 0x046D  ; 'm' in red
    mov word [0xB81E2], 0x0461  ; 'a'
    mov word [0xB81E4], 0x0463  ; 'c'
    mov word [0xB81E6], 0x044F  ; 'O'
    mov word [0xB81E8], 0x0453  ; 'S'
    mov word [0xB81EA], 0x0420  ; ' '
    mov word [0xB81EC], 0x0454  ; 'T'
    mov word [0xB81EE], 0x0465  ; 'e'
    mov word [0xB81F0], 0x0472  ; 'r'
    mov word [0xB81F2], 0x046D  ; 'm'
    mov word [0xB81F4], 0x0469  ; 'i'
    mov word [0xB81F6], 0x046E  ; 'n'
    mov word [0xB81F8], 0x0461  ; 'a'
    mov word [0xB81FA], 0x046C  ; 'l'
    mov word [0xB81FC], 0x0420  ; ' '
    mov word [0xB81FE], 0x0472  ; 'r'
    mov word [0xB8200], 0x0475  ; 'u'
    mov word [0xB8202], 0x046E  ; 'n'
    mov word [0xB8204], 0x046E  ; 'n'
    mov word [0xB8206], 0x0469  ; 'i'
    mov word [0xB8208], 0x046E  ; 'n'
    mov word [0xB820A], 0x0467  ; 'g'
    mov word [0xB820C], 0x0421  ; '!'
    ret

; WNU Native toolchain command
cmd_wnu:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Display "WNU Native OS Kernel"
    mov word [0xB80A0], 0x0D57  ; 'W' in magenta
    mov word [0xB80A2], 0x0D4E  ; 'N'
    mov word [0xB80A4], 0x0D55  ; 'U'
    mov word [0xB80A6], 0x0D20  ; ' '
    mov word [0xB80A8], 0x0D4E  ; 'N'
    mov word [0xB80AA], 0x0D61  ; 'a'
    mov word [0xB80AC], 0x0D74  ; 't'
    mov word [0xB80AE], 0x0D69  ; 'i'
    mov word [0xB80B0], 0x0D76  ; 'v'
    mov word [0xB80B2], 0x0D65  ; 'e'
    mov word [0xB80B4], 0x0D20  ; ' '
    mov word [0xB80B6], 0x0D4F  ; 'O'
    mov word [0xB80B8], 0x0D53  ; 'S'
    mov word [0xB80BA], 0x0D20  ; ' '
    mov word [0xB80BC], 0x0D4B  ; 'K'
    mov word [0xB80BE], 0x0D65  ; 'e'
    mov word [0xB80C0], 0x0D72  ; 'r'
    mov word [0xB80C2], 0x0D6E  ; 'n'
    mov word [0xB80C4], 0x0D65  ; 'e'
    mov word [0xB80C6], 0x0D6C  ; 'l'
    
    ; Display WNU capabilities
    mov word [0xB8140], 0x0D4E  ; 'N' in magenta
    mov word [0xB8142], 0x0D61  ; 'a'
    mov word [0xB8144], 0x0D74  ; 't'
    mov word [0xB8146], 0x0D69  ; 'i'
    mov word [0xB8148], 0x0D76  ; 'v'
    mov word [0xB814A], 0x0D65  ; 'e'
    mov word [0xB814C], 0x0D20  ; ' '
    mov word [0xB814E], 0x0D73  ; 's'
    mov word [0xB8150], 0x0D79  ; 'y'
    mov word [0xB8152], 0x0D73  ; 's'
    mov word [0xB8154], 0x0D74  ; 't'
    mov word [0xB8156], 0x0D65  ; 'e'
    mov word [0xB8158], 0x0D6D  ; 'm'
    mov word [0xB815A], 0x0D20  ; ' '
    mov word [0xB815C], 0x0D63  ; 'c'
    mov word [0xB815E], 0x0D61  ; 'a'
    mov word [0xB8160], 0x0D6C  ; 'l'
    mov word [0xB8162], 0x0D6C  ; 'l'
    mov word [0xB8164], 0x0D73  ; 's'
    
    ; Run demo WNU app
    call wnu_native_app
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

wnu_native_app:
    ; Display "WNU OS Server 1.0.0 Active!"
    mov word [0xB81E0], 0x0D57  ; 'W' in magenta
    mov word [0xB81E2], 0x0D4E  ; 'N'
    mov word [0xB81E4], 0x0D55  ; 'U'
    mov word [0xB81E6], 0x0D20  ; ' '
    mov word [0xB81E8], 0x0D4F  ; 'O'
    mov word [0xB81EA], 0x0D53  ; 'S'
    mov word [0xB81EC], 0x0D20  ; ' '
    mov word [0xB81EE], 0x0D53  ; 'S'
    mov word [0xB81F0], 0x0D65  ; 'e'
    mov word [0xB81F2], 0x0D72  ; 'r'
    mov word [0xB81F4], 0x0D76  ; 'v'
    mov word [0xB81F6], 0x0D65  ; 'e'
    mov word [0xB81F8], 0x0D72  ; 'r'
    mov word [0xB81FA], 0x0D20  ; ' '
    mov word [0xB81FC], 0x0D31  ; '1'
    mov word [0xB81FE], 0x0D2E  ; '.'
    mov word [0xB8200], 0x0D30  ; '0'
    mov word [0xB8202], 0x0D2E  ; '.'
    mov word [0xB8204], 0x0D30  ; '0'
    mov word [0xB8206], 0x0D20  ; ' '
    mov word [0xB8208], 0x0D41  ; 'A'
    mov word [0xB820A], 0x0D63  ; 'c'
    mov word [0xB820C], 0x0D74  ; 't'
    mov word [0xB820E], 0x0D69  ; 'i'
    mov word [0xB8210], 0x0D76  ; 'v'
    mov word [0xB8212], 0x0D65  ; 'e'
    mov word [0xB8214], 0x0D21  ; '!'
    ret

; Toolchain overview command
cmd_toolchain:
    push rbp
    mov rbp, rsp
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    
    ; Clear screen area for toolchain display
    mov rcx, 0
clear_toolchain_area:
    mov word [0xB8000 + rcx], 0x0720  ; Clear with spaces
    add rcx, 2
    cmp rcx, 4000  ; 80x25 screen
    jl clear_toolchain_area
    
    ; Call the C toolchain manager with "select" command
    mov rdi, toolchain_select_cmd  ; Pass "select" as argument
    call wnu_toolchain_main
    
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp
    jmp reset_prompt

toolchain_select_cmd db 'select', 0

cmd_poweroff:
    call systemd_poweroff
    ret