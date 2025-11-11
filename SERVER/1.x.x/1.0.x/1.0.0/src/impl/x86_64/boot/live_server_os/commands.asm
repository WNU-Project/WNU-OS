; WNU OS SERVER Command Recognition and Handling System
section .text
global process_command
global check_command
bits 64

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
    
    call check_cd
    cmp al, 1
    je cmd_cd
    
    call check_pwd
    cmp al, 1
    je cmd_pwd
    
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
    
    ; Check 'c'
    cmp word [0xB8020], 0x0763  ; c
    jne cd_no_match
    
    ; Check 'd'
    cmp word [0xB8022], 0x0764  ; d
    jne cd_no_match
    
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

cmd_ls:
    ; Clear output lines
    call clear_output_lines
    
    ; Display header
    call display_ls_header
    
    ; Display simulated ISO directory listing
    call display_iso_simulation
    
    jmp reset_prompt

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
    mov word [0xB80CA], 0x0F2F  ; '/' bright white
    mov word [0xB80CC], 0x0F3A  ; ':' bright white
    
    ret
    cmp al, 0
    je header_root
    
    ; For now, just show root "/"
header_root:
    mov word [0xB80CC], 0x0F3A  ; ':' bright white
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
    or al, 0x0F                 ; Bright white
    mov ah, al
    and al, 0x7F               ; Keep only char
    mov ah, 0x0F               ; Bright white attribute
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

cmd_cd:
    ; Parse the cd command argument
    ; cd command format: "cd directory" or "cd" (for root)
    
    ; Check if there's an argument after "cd "
    cmp word [0xB8024], 0x0720  ; Check for space
    jne cd_go_root              ; No space, go to root
    
    ; Extract directory name from command
    ; Check for specific directories
    
    ; Check for "cd boot"
    cmp word [0xB8026], 0x0762  ; 'b'
    jne cd_check_parent
    cmp word [0xB8028], 0x076F  ; 'o'
    jne cd_check_parent
    cmp word [0xB802A], 0x076F  ; 'o'
    jne cd_check_parent
    cmp word [0xB802C], 0x0774  ; 't'
    jne cd_check_parent
    
    ; Change to boot directory
    mov byte [current_dir_level], 1
    jmp cd_success
    
cd_check_parent:
    ; Check for "cd .."
    cmp word [0xB8026], 0x072E  ; '.'
    jne cd_check_filesystem
    cmp word [0xB8028], 0x072E  ; '.'
    jne cd_check_filesystem
    
    ; Go to parent directory (root)
    mov byte [current_dir_level], 0
    jmp cd_success

cd_check_filesystem:
    ; Check for "cd filesystem"
    cmp word [0xB8026], 0x0766  ; 'f'
    jne cd_check_installhelp
    cmp word [0xB8028], 0x0769  ; 'i'
    jne cd_check_installhelp
    cmp word [0xB802A], 0x076C  ; 'l'
    jne cd_check_installhelp
    cmp word [0xB802C], 0x0765  ; 'e'
    jne cd_check_installhelp
    cmp word [0xB802E], 0x0773  ; 's'
    jne cd_check_installhelp
    
    ; Change to filesystem directory
    mov byte [current_dir_level], 2
    jmp cd_success

cd_check_installhelp:
    ; Check for "cd installhelp"
    cmp word [0xB8026], 0x0769  ; 'i'
    jne cd_error_not_found
    cmp word [0xB8028], 0x076E  ; 'n'
    jne cd_error_not_found
    cmp word [0xB802A], 0x0773  ; 's'
    jne cd_error_not_found
    cmp word [0xB802C], 0x0774  ; 't'
    jne cd_error_not_found
    
    ; Change to installhelp directory  
    mov byte [current_dir_level], 3
    jmp cd_success

cd_go_root:
    ; Change to root directory
    mov byte [current_dir_level], 0
    jmp cd_success

cd_success:
    ; Clear output line
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show success message
    mov word [0xB80A0], 0x0A43  ; 'C' green
    mov word [0xB80A2], 0x0A68  ; 'h'
    mov word [0xB80A4], 0x0A61  ; 'a'
    mov word [0xB80A6], 0x0A6E  ; 'n'
    mov word [0xB80A8], 0x0A67  ; 'g'
    mov word [0xB80AA], 0x0A65  ; 'e'
    mov word [0xB80AC], 0x0A64  ; 'd'
    mov word [0xB80AE], 0x0A20  ; ' '
    mov word [0xB80B0], 0x0A64  ; 'd'
    mov word [0xB80B2], 0x0A69  ; 'i'
    mov word [0xB80B4], 0x0A72  ; 'r'
    mov word [0xB80B6], 0x0A65  ; 'e'
    mov word [0xB80B8], 0x0A63  ; 'c'
    mov word [0xB80BA], 0x0A74  ; 't'
    mov word [0xB80BC], 0x0A6F  ; 'o'
    mov word [0xB80BE], 0x0A72  ; 'r'
    mov word [0xB80C0], 0x0A79  ; 'y'
    
    ; Update prompt to show current directory
    call update_prompt
    jmp reset_prompt

cd_error_not_found:
    ; Clear output line
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    
    ; Show error message
    mov word [0xB80A0], 0x0C63  ; 'c' red
    mov word [0xB80A2], 0x0C64  ; 'd'
    mov word [0xB80A4], 0x0C3A  ; ':'
    mov word [0xB80A6], 0x0C20  ; ' '
    mov word [0xB80A8], 0x0C4E  ; 'N'
    mov word [0xB80AA], 0x0C6F  ; 'o'
    mov word [0xB80AC], 0x0C20  ; ' '
    mov word [0xB80AE], 0x0C73  ; 's'
    mov word [0xB80B0], 0x0C75  ; 'u'
    mov word [0xB80B2], 0x0C63  ; 'c'
    mov word [0xB80B4], 0x0C68  ; 'h'
    mov word [0xB80B6], 0x0C20  ; ' '
    mov word [0xB80B8], 0x0C64  ; 'd'
    mov word [0xB80BA], 0x0C69  ; 'i'
    mov word [0xB80BC], 0x0C72  ; 'r'
    mov word [0xB80BE], 0x0C65  ; 'e'
    mov word [0xB80C0], 0x0C63  ; 'c'
    mov word [0xB80C2], 0x0C74  ; 't'
    mov word [0xB80C4], 0x0C6F  ; 'o'
    mov word [0xB80C6], 0x0C72  ; 'r'
    mov word [0xB80C8], 0x0C79  ; 'y'
    jmp reset_prompt

; Update the prompt to show current directory
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
    mov word [0xB80D8], 0x0C77  ; 'w' red (highlight WNUC)
    mov word [0xB80DA], 0x0C6E  ; 'n'
    mov word [0xB80DC], 0x0C75  ; 'u'
    mov word [0xB80DE], 0x0C63  ; 'c'
    jmp reset_prompt

cmd_clear:
    ; Clear the entire output line (80 characters)
    mov word [0xB80A0], 0x0720
    mov word [0xB80A2], 0x0720
    mov word [0xB80A4], 0x0720
    mov word [0xB80A6], 0x0720
    mov word [0xB80A8], 0x0720
    mov word [0xB80AA], 0x0720
    mov word [0xB80AC], 0x0720
    mov word [0xB80AE], 0x0720
    mov word [0xB80B0], 0x0720
    mov word [0xB80B2], 0x0720
    mov word [0xB80B4], 0x0720
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
    mov word [0xB80A0], 0x0F72  ; 'r' bright white
    mov word [0xB80A2], 0x0F6F  ; 'o'
    mov word [0xB80A4], 0x0F6F  ; 'o'
    mov word [0xB80A6], 0x0F74  ; 't'
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
    mov word [0xB80D0], 0x0731 ; 1
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
    mov word [0xB8140], 0x0F43  ; 'C' bright white
    mov word [0xB8142], 0x0F72  ; 'r'
    mov word [0xB8144], 0x0F65  ; 'e'
    mov word [0xB8146], 0x0F61  ; 'a'
    mov word [0xB8148], 0x0F74  ; 't'
    mov word [0xB814A], 0x0F69  ; 'i'
    mov word [0xB814C], 0x0F6E  ; 'n'
    mov word [0xB814E], 0x0F67  ; 'g'
    mov word [0xB8150], 0x0F20  ; ' '
    mov word [0xB8152], 0x0F74  ; 't'
    mov word [0xB8154], 0x0F68  ; 'h'
    mov word [0xB8156], 0x0F69  ; 'i'
    mov word [0xB8158], 0x0F73  ; 's'
    mov word [0xB815A], 0x0F20  ; ' '
    mov word [0xB815C], 0x0F57  ; 'W'
    mov word [0xB815E], 0x0F4E  ; 'N'
    mov word [0xB8160], 0x0F55  ; 'U'
    mov word [0xB8162], 0x0F20  ; ' '
    mov word [0xB8164], 0x0F4F  ; 'O'
    mov word [0xB8166], 0x0F53  ; 'S'
    mov word [0xB8168], 0x0F20  ; ' '
    mov word [0xB816A], 0x0F77  ; 'w'
    mov word [0xB816C], 0x0F61  ; 'a'
    mov word [0xB816E], 0x0F73  ; 's'
    mov word [0xB8170], 0x0F20  ; ' '
    mov word [0xB8172], 0x0C52  ; 'R' red (emphasis)
    mov word [0xB8174], 0x0C45  ; 'E'
    mov word [0xB8176], 0x0C41  ; 'A'
    mov word [0xB8178], 0x0C4C  ; 'L'
    mov word [0xB817A], 0x0C4C  ; 'L'
    mov word [0xB817C], 0x0C59  ; 'Y'
    mov word [0xB817E], 0x0F20  ; ' '
    mov word [0xB8180], 0x0F68  ; 'h'
    mov word [0xB8182], 0x0F61  ; 'a'
    mov word [0xB8184], 0x0F72  ; 'r'
    mov word [0xB8186], 0x0F64  ; 'd'
    mov word [0xB8188], 0x0F21  ; '!'
    
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
    mov word [0xB80C8], 0x0F76  ; 'v' bright white
    mov word [0xB80CA], 0x0F31  ; '1'
    mov word [0xB80CC], 0x0F2E  ; '.'
    mov word [0xB80CE], 0x0F30  ; '0'
    
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
    mov word [0xB81F6], 0x0F68  ; 'h' bright white
    mov word [0xB81F8], 0x0F65  ; 'e'
    mov word [0xB81FA], 0x0F6C  ; 'l'
    mov word [0xB81FC], 0x0F6C  ; 'l'
    mov word [0xB81FE], 0x0F6F  ; 'o'
    mov word [0xB8200], 0x0F5F  ; '_'
    mov word [0xB8202], 0x0F77  ; 'w'
    mov word [0xB8204], 0x0F6F  ; 'o'
    mov word [0xB8206], 0x0F72  ; 'r'
    mov word [0xB8208], 0x0F6C  ; 'l'
    mov word [0xB820A], 0x0F64  ; 'd'
    mov word [0xB820C], 0x0F2E  ; '.'
    mov word [0xB820E], 0x0F61  ; 'a'
    mov word [0xB8210], 0x0F73  ; 's'
    mov word [0xB8212], 0x0F6D  ; 'm'
    
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