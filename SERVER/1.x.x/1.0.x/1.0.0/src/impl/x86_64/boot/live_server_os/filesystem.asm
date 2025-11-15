; WNU OS SERVER File System Implementation
; Virtual File System (VFS) with in-memory storage
; Version: 1.0.1.12.11.2025.BETA

section .data
    fs_banner db 'WNU File System v1.0.1 Initializing...', 0x0A, 0
    fs_ready db 'File System mounted and ready', 0x0A, 0
    
    ; File system constants
    MAX_FILES equ 256           ; Maximum number of files
    MAX_FILENAME equ 32         ; Maximum filename length
    MAX_FILESIZE equ 4096       ; Maximum file size (4KB per file)
    
    ; Root directory structure
    root_dir_name db '/', 0
    current_dir db '/', 0
    
    ; File operation messages
    file_created db 'File created: ', 0
    file_deleted db 'File deleted: ', 0
    file_not_found db 'File not found: ', 0
    file_exists db 'File already exists: ', 0
    dir_created db 'Directory created: ', 0
    
    ; Default files
    readme_name db 'README.txt', 0
    readme_content db 'Welcome to WNU OS Server File System!', 0x0A
                  db 'This is a simple in-memory filesystem.', 0x0A
                  db 'Type "ls" to list files.', 0x0A
                  db 'Type "cat README.txt" to read this file.', 0x0A, 0
    
    motd_name db 'motd.txt', 0
    motd_content db 'Message of the Day:', 0x0A
                db '- WNU OS Server 1.0.1.BETA', 0x0A
                db '- File System: ACTIVE', 0x0A
                db '- SystemD: RUNNING', 0x0A
                db '- NetworkD: READY', 0x0A, 0
    
    space_char db ' ', 0
    newline db 0x0A, 0
    debug_creating db 'Creating default files... Count: ', 0

section .bss
    ; File table structure
    file_table resb (MAX_FILES * 64)  ; File metadata table
    file_data resb (MAX_FILES * MAX_FILESIZE)  ; File content storage
    file_count resb 4                  ; Number of files
    current_file_id resb 4             ; Current file ID for operations

section .text
global init_filesystem
global create_file
global delete_file
global read_file
global write_file
global list_files
global file_exists
global get_file_size
bits 64

; Initialize the file system
init_filesystem:
    ; Display banner
    mov rsi, fs_banner
    call print_fs_msg
    
    ; Initialize file table
    mov dword [file_count], 0
    call clear_file_table
    
    ; Create default files
    call create_default_files
    
    ; Mark filesystem as ready
    mov rsi, fs_ready
    call print_fs_msg
    
    ret

; Clear file table
clear_file_table:
    mov rdi, file_table
    mov rcx, (MAX_FILES * 64)
    xor rax, rax
    rep stosb
    ret

; Create default system files
create_default_files:
    ; Debug: Creating files
    mov rsi, debug_creating
    call print_fs_msg
    
    ; Create README.txt
    mov rsi, readme_name
    mov rdi, readme_content
    call create_file_with_content
    
    ; Debug: File count after README
    mov rax, [file_count]
    call print_number
    mov rsi, space_char
    call print_fs_msg
    
    ; Create motd.txt
    mov rsi, motd_name
    mov rdi, motd_content
    call create_file_with_content
    
    ; Debug: File count after MOTD
    mov rax, [file_count]
    call print_number
    mov rsi, newline
    call print_fs_msg
    
    ret

; Create a file with content
; RSI = filename, RDI = content
create_file_with_content:
    push rsi                    ; Save filename
    push rdi                    ; Save content
    push rbx                    ; Save rbx
    
    ; Check if we have space for more files
    mov eax, [file_count]
    cmp eax, MAX_FILES
    jae .no_space
    
    ; Calculate file table entry address
    mov rbx, rax
    imul rbx, 64                ; Each entry is 64 bytes
    add rbx, file_table
    
    ; Copy filename (first 32 bytes of entry)
    mov rdi, rbx               ; Destination = entry address
    mov rsi, [rsp + 16]        ; Source = original filename (rsp+16 because we pushed 3 times)
    mov rcx, MAX_FILENAME
    call copy_string_with_size
    
    ; Calculate data storage address
    mov eax, [file_count]
    imul rax, MAX_FILESIZE
    add rax, file_data
    mov [rbx + 32], rax         ; Store data pointer at offset 32
    
    ; Copy content to data storage
    mov rdi, rax               ; Destination = data storage
    mov rsi, [rsp + 8]         ; Source = original content (rsp+8)
    call copy_string_with_size
    mov [rbx + 40], rcx         ; Store file size at offset 40
    
    ; Mark file as active
    mov byte [rbx + 44], 1      ; Active flag at offset 44
    
    ; Increment file count
    inc dword [file_count]
    
    pop rbx
    pop rdi
    pop rsi
    ret

.no_space:
    pop rbx
    pop rdi
    pop rsi
    ret

; Create an empty file
; RSI = filename
create_file:
    push rsi
    
    ; Check if file already exists
    call find_file
    cmp rax, -1
    jne .file_exists_error
    
    pop rsi
    push rsi
    
    ; Create file with empty content
    mov rdi, empty_content
    call create_file_with_content
    
    ; Display success message
    mov rsi, file_created
    call print_fs_msg
    pop rsi
    call print_fs_msg
    call print_newline
    
    ret

.file_exists_error:
    pop rsi
    push rsi
    mov rsi, file_exists
    call print_fs_msg
    pop rsi
    call print_fs_msg
    call print_newline
    ret

; Find file by name
; RSI = filename
; Returns: RAX = file table entry address, or -1 if not found
find_file:
    push rbx
    push rcx
    push rdi
    
    mov rbx, file_table
    mov ecx, [file_count]
    
.search_loop:
    cmp rcx, 0
    je .not_found
    
    ; Check if file is active
    cmp byte [rbx + 44], 1
    jne .next_file
    
    ; Compare filename
    push rsi
    push rcx
    mov rdi, rbx                ; File table entry
    mov rcx, MAX_FILENAME
    call compare_strings
    pop rcx
    pop rsi
    
    cmp rax, 1                  ; Strings match?
    je .found
    
.next_file:
    add rbx, 64                 ; Next file entry
    dec rcx
    jmp .search_loop
    
.not_found:
    mov rax, -1
    jmp .done
    
.found:
    mov rax, rbx
    
.done:
    pop rdi
    pop rcx
    pop rbx
    ret

; List all files
list_files:
    push rbx
    push rcx
    
    ; Print file count first for debugging
    mov rax, [file_count]
    call print_number
    call print_newline
    
    mov rbx, file_table
    mov ecx, [file_count]
    
.list_loop:
    cmp rcx, 0
    je .done
    
    ; Check if file is active
    cmp byte [rbx + 44], 1
    jne .next_file
    
    ; Print filename (filename is at the beginning of the entry)
    mov rsi, rbx                ; Point to filename
    call print_fs_msg
    
    ; Print newline
    call print_newline

.next_file:
    add rbx, 64
    dec rcx
    jmp .list_loop
    
.done:
    pop rcx
    pop rbx
    ret

; Read file content and display it
; RSI = filename
read_file:
    call find_file
    cmp rax, -1
    je .not_found
    
    ; Get file data pointer and size
    mov rbx, rax
    mov rsi, [rbx + 32]         ; Data pointer
    mov rcx, [rbx + 40]         ; File size
    
    ; Display content
    call print_fs_msg
    ret
    
.not_found:
    push rsi
    mov rsi, file_not_found
    call print_fs_msg
    pop rsi
    call print_fs_msg
    call print_newline
    ret

; Helper functions
copy_string_with_size:
    xor rcx, rcx
.copy_loop:
    lodsb
    test al, al
    jz .done
    stosb
    inc rcx
    cmp rcx, MAX_FILESIZE
    jae .done
    jmp .copy_loop
.done:
    ret

compare_strings:
    push rsi
    push rdi
.cmp_loop:
    lodsb
    scasb
    jne .not_equal
    test al, al
    jz .equal
    loop .cmp_loop
.not_equal:
    mov rax, 0
    jmp .done
.equal:
    mov rax, 1
.done:
    pop rdi
    pop rsi
    ret

print_number:
    ; Simple number printing (placeholder)
    ; In real implementation, convert RAX to string and print
    ret

print_newline:
    push rsi
    mov rsi, newline
    call print_fs_msg
    pop rsi
    ret

print_fs_msg:
    ; Use existing VGA printing infrastructure
    mov rdi, 0xB8000
    
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
    
    mov ah, 0x0F                ; White on black for filesystem messages
    stosw
    jmp .print_loop
    
.done:
    ret

section .data
empty_content db 0