section .data
    message db "Hello, World!", 10  ; The string to print, followed by a newline character (ASCII 10)
    msglen equ $ - message         ; Calculate the length of the message
    prompt db "Press Y to start install", 0
    prompt_len equ $ - prompt - 1
    install_done_msg db "Install complete!", 0
    install_done_len equ $ - install_done_msg - 1
    fb_flag: db 0

section .text
    global _start
    bits 64

_start:
    ; Try to find a framebuffer provided by the multiboot2 info (RDI)
    ; If found, write directly to the linear framebuffer (non-text mode).
    mov rbx, rdi            ; rbx = multiboot2 info pointer
    test rbx, rbx
    jz .no_fb_found

    add rbx, 8              ; skip total_size and reserved
.tag_scan:
    mov eax, dword [rbx]    ; tag type
    mov edx, dword [rbx+4]  ; tag size
    cmp eax, 0
    je .no_fb_found
    cmp eax, 8              ; framebuffer tag (multiboot2)
    jne .next_tag

    lea rcx, [rbx+8]        ; payload address
    mov rax, [rcx]          ; framebuffer physical/virtual address (u64)
    mov rdi, rax            ; rdi = framebuffer base
    mov ebx, dword [rcx+8]  ; pitch (bytes per scanline)
    mov esi, dword [rcx+12] ; width
    mov edx, dword [rcx+16] ; height
    mov cl, byte [rcx+20]   ; bpp
    cmp cl, 32
    jb .no_fb_found         ; only support 32bpp here
    mov bl, byte [rcx+21]   ; framebuffer type

    ; Determine color masks if framebuffer is RGB (type 1)
    xor ebp, ebp            ; ebp will hold color value (32-bit)
    cmp bl, 1
    jne .fb_render_setup
    ; read RGB field positions and sizes (offsets 24..29)
    mov dl, byte [rcx+24]   ; red_field_position
    mov dh, byte [rcx+25]   ; red_mask_size
    mov sil, byte [rcx+26]  ; green_field_position
    mov sil, sil            ; keep in sil
    mov dil, byte [rcx+27]  ; green_mask_size (temporarily in dil)
    mov r8b, byte [rcx+28]  ; blue_field_position
    mov r9b, byte [rcx+29]  ; blue_mask_size

    ; Build color max per channel then shift into place
    ; red_max = ((1 << red_mask_size) - 1) << red_field_position
    mov eax, 1
    mov cl, dh
    shl eax, cl
    dec eax
    mov ecx, eax
    mov cl, dl
    shl ecx, cl
    mov ebp, ecx
    ; green
    mov eax, 1
    mov cl, dil
    shl eax, cl
    dec eax
    mov ecx, eax
    mov cl, sil
    shl ecx, cl
    or ebp, ecx
    ; blue
    mov eax, 1
    mov cl, r9b
    shl eax, cl
    dec eax
    mov ecx, eax
    mov cl, r8b
    shl ecx, cl
    or ebp, ecx

.fb_render_setup:
    ; Mark framebuffer as found
    mov byte [fb_flag], 1

    ; Render message into 32bpp linear framebuffer using a tiny 8x8 glyph set
    mov r15, rdi           ; r15 = framebuffer base
    mov r14d, ebx          ; r14d = pitch (bytes per scanline)
    mov r13d, esi          ; r13d = width
    mov r12d, edx          ; r12d = height
    mov r11d, 4            ; r11d = bytes per pixel (we only support 32bpp)

    xor r10d, r10d         ; x_pixels = 0
    xor r9d, r9d           ; y_base = 0

    lea rsi, [rel message]
    mov rdx, msglen        ; number of characters to render (message length)
.fb_text_loop:
    test rdx, rdx
    jz .fb_done
    mov al, [rsi]
    cmp al, 10             ; newline in the data -> end
    je .fb_done

    ; Select glyph pointer for the small set of characters used in the message
    movzx eax, al
    cmp al, 'H'
    je .set_H
    cmp al, 'e'
    je .set_e
    cmp al, 'l'
    je .set_l
    cmp al, 'o'
    je .set_o
    cmp al, ','
    je .set_comma
    cmp al, ' '
    je .set_space
    cmp al, 'W'
    je .set_W
    cmp al, 'r'
    je .set_r
    cmp al, 'd'
    je .set_d
    cmp al, '!'
    je .set_excl
    ; Unknown char: advance 8 pixels
    add r10d, 8
    add rsi, 1
    dec rdx
    jmp .fb_text_loop

.set_H: lea rbx, [rel glyph_H_bytes]
    jmp .render_glyph
.set_e: lea rbx, [rel glyph_e_bytes]
    jmp .render_glyph
.set_l: lea rbx, [rel glyph_l_bytes]
    jmp .render_glyph
.set_o: lea rbx, [rel glyph_o_bytes]
    jmp .render_glyph
.set_comma: lea rbx, [rel glyph_comma_bytes]
    jmp .render_glyph
.set_space: lea rbx, [rel glyph_space_bytes]
    jmp .render_glyph
.set_W: lea rbx, [rel glyph_W_bytes]
    jmp .render_glyph
.set_r: lea rbx, [rel glyph_r_bytes]
    jmp .render_glyph
.set_d: lea rbx, [rel glyph_d_bytes]
    jmp .render_glyph
.set_excl: lea rbx, [rel glyph_excl_bytes]
    jmp .render_glyph

; r15 = fb base, r14 = pitch, r11 = bpp bytes-per-pixel, r10 = x_pixels
.render_glyph:
    xor rcx, rcx           ; row = 0
.row_loop:
    ; compute base address for this row: fb + (y_base + row)*pitch + x_pixels*bytes_per_pixel
    mov rax, rcx
    imul rax, r14          ; rax = row * pitch
    mov rdx, r10
    imul rdx, r11          ; rdx = x_pixels * bytes_per_pixel
    add rax, rdx
    add rax, r15           ; rax = row_base_address

    mov bl, [rbx + rcx]    ; glyph row bits
    mov r8d, 0             ; col index
.col_loop:
    ; test highest bit
    test bl, 0x80
    jz .skip_pixel
    mov dword [rax], 0x00FFFFFF
.skip_pixel:
    add rax, r11           ; advance by bytes_per_pixel
    shl bl, 1
    inc r8d
    cmp r8d, 8
    jl .col_loop

    inc rcx
    cmp rcx, 8
    jl .row_loop

    ; advance to next character: move x by 8 pixels
    add r10d, 8
    add rsi, 1
    dec rdx
    jmp .fb_text_loop

.fb_done:
    ; After initial display, wait for user input to start install
    call .wait_for_key
    ; If key was 'y', perform simulated install
    cmp al, 'y'
    jne .fb_done
    call .simulate_install
    ; show completion message
    mov rsi, install_done_msg
    mov rdx, install_done_len
    call .render_message_generic
    cli
.fb_halt:
    hlt
    jmp .fb_halt

.next_tag:
    add rbx, rdx
    ; tags are 8-byte aligned
    and rbx, -8
    jmp .tag_scan

.no_fb_found:
    ; Fallback: write to VGA text-mode buffer at 0xB8000
    mov rdi, 0xb8000
    lea rsi, [rel message]
    mov rcx, msglen
.text_print_loop:
    mov al, [rsi]
    cmp al, 0
    je .done
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rsi, 1
    add rdi, 2
    dec rcx
    jnz .text_print_loop
.done:
    ; After text-mode display, present prompt and wait for key
    ; write prompt to VGA after the message
    lea rdi, [0xb8000 + 160*2]  ; start on second line (approx)
    lea rsi, [rel prompt]
    mov rcx, prompt_len
.write_prompt_vga:
    mov al, [rsi]
    cmp al, 0
    je .wait_key_vga
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rsi, 1
    add rdi, 2
    jmp .write_prompt_vga
.wait_key_vga:
    call .wait_for_key
    cmp al, 'y'
    jne .wait_key_vga
    call .simulate_install_vga
    ; show completion message in VGA
    lea rdi, [0xb8000 + 160*4]
    lea rsi, [rel install_done_msg]
    mov rcx, install_done_len
.write_done_vga:
    mov al, [rsi]
    cmp al, 0
    je .halt_vga
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rsi, 1
    add rdi, 2
    jmp .write_done_vga
.halt_vga:
    cli
.hlt_loop:
    hlt
    jmp .hlt_loop

; -- Input and install routines --
.wait_for_key:
    ; Poll keyboard controller for a key press and return ASCII in AL (lowercase if letter)
.wait_key_loop:
    in al, 0x64
    test al, 1
    jz .wait_key_loop
    in al, 0x60
    cmp al, 0x80
    jae .wait_key_loop
    ; map scancode to ASCII for basic keys (use common set)
    cmp al, 0x15
    je .key_y
    ; default: return 0
    mov al, 0
    ret
.key_y:
    mov al, 'y'
    ret

.simulate_install_vga:
    ; simple text-mode progress (0..100)
    lea rdi, [0xb8000 + 160*3]
    mov ecx, 0
.install_loop_vga:
    ; print percentage
    ; write number tens/hundreds
    mov eax, ecx
    mov ebx, 10
    xor edx, edx
    div ebx
    ; EAX = quotient (tens), EDX = remainder (ones)
    add al, '0'
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rdi, 2
    mov al, dl
    add al, '0'
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rdi, 2
    mov byte [rdi], '%'
    mov byte [rdi+1], 0x07
    ; small delay
    mov rsi, 0x400000
.delay_vga:
    dec rsi
    jnz .delay_vga
    ; next
    inc ecx
    cmp ecx, 100
    jle .install_loop_vga
    ret

.simulate_install:
    ; framebuffer install simulation: draw growing filled rectangle
    mov rdi, r15       ; framebuffer base
    mov rsi, r14       ; pitch (use full 64-bit register)
    ; width in pixels = r13d
    mov ecx, 0
.install_loop_fb:
    ; compute pixel_count = (ecx * r13d) / 100
    mov eax, ecx
    imul eax, r13d
    mov ebx, 100
    xor edx, edx
    div ebx
    mov edi, eax        ; pixels to fill in first row
    ; fill first row pixels for demo
    mov r10, rdi
    xor r11, r11
.fb_fill_row:
    cmp r11, edi
    jge .fb_next_percent
    ; compute address = rdi + r11*4
    mov rax, r15
    mov rdx, r11
    shl rdx, 2
    add rax, rdx
    mov dword [rax], 0x00FF00FF
    inc r11
    jmp .fb_fill_row
.fb_next_percent:
    ; small delay
    mov rbx, 0x200000
.fb_delay:
    dec rbx
    jnz .fb_delay
    inc ecx
    cmp ecx, 100
    jle .install_loop_fb
    ret

; Generic message renderer used after framebuffer install to display
; the completion message. Caller must set:
;  rsi -> message (pointer)
;  rdx  = length
.render_message_generic:
    ; if we have a framebuffer, render simple 8x8 blocks per character
    cmp byte [fb_flag], 1
    jne .rmg_vga

    ; framebuffer path
    xor r10d, r10d        ; x_pixels = 0
    mov rcx, rdx          ; chars remaining
    test rcx, rcx
    jz .rmg_done
.rmg_fb_loop:
    mov al, [rsi]
    cmp al, 0
    je .rmg_done

    ; draw an 8x8 solid block for this character at (x_pixels, 0)
    xor r9d, r9d          ; row = 0
.rmg_row:
    ; compute row_base = fb_base + row * pitch + x_pixels * bytes_per_pixel
    mov rax, r9
    imul rax, r14         ; r14 = pitch
    mov rdx, r10
    imul rdx, r11         ; r11 = bytes per pixel
    add rax, rdx
    add rax, r15          ; rax = address for this row start

    xor r8d, r8d          ; col = 0
.rmg_col:
    mov dword [rax], 0x00FFFFFF
    add rax, r11
    inc r8d
    cmp r8d, 8
    jl .rmg_col

    inc r9d
    cmp r9d, 8
    jl .rmg_row

    ; advance to next character
    add r10d, 8
    inc rsi
    dec rcx
    jnz .rmg_fb_loop
.rmg_done:
    ret

.rmg_vga:
    ; VGA text-mode fallback: write message to 0xB8000 starting at line 6
    lea rdi, [0xb8000 + 160*6]
    mov rcx, rdx
.rmg_vga_loop:
    mov al, [rsi]
    cmp al, 0
    je .rmg_vga_done
    mov [rdi], al
    mov byte [rdi+1], 0x07
    add rsi, 1
    add rdi, 2
    dec rcx
    jnz .rmg_vga_loop
.rmg_vga_done:
    ret

section .rodata

; 8x8 glyphs for characters used in "Hello, World!"
glyph_H_bytes: db 0x81,0x81,0x81,0xFF,0x81,0x81,0x81,0x81
glyph_e_bytes: db 0x00,0x00,0x3C,0x42,0xFE,0x40,0x3C,0x00
glyph_l_bytes: db 0x10,0x10,0x10,0x10,0x10,0x10,0x0C,0x00
glyph_o_bytes: db 0x00,0x00,0x3C,0x42,0x42,0x42,0x3C,0x00
glyph_comma_bytes: db 0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x10
glyph_space_bytes: db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
glyph_W_bytes: db 0x81,0x81,0x81,0x81,0x81,0x5A,0x66,0x42
glyph_r_bytes: db 0x00,0x00,0x5C,0x62,0x40,0x40,0x40,0x00
glyph_d_bytes: db 0x02,0x02,0x3A,0x46,0x42,0x42,0x3E,0x00
glyph_excl_bytes: db 0x10,0x10,0x10,0x10,0x10,0x00,0x10,0x00
