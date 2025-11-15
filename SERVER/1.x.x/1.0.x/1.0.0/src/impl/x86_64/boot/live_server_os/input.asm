; WNU OS SERVER Input Handler - Ultra Safe Version
section .text
global get_input
global handle_keypress
extern process_command
bits 64

; Safe input checking function
get_input:
    ; Check if keyboard has data (port 0x64)
    in al, 0x64
    test al, 1
    jz no_input
    
    ; Read the scancode (port 0x60)
    in al, 0x60
    ret
    
no_input:
    mov al, 0                   ; Return 0 if no input
    ret

; Handle a single keypress safely - FULL KEYBOARD SUPPORT!
handle_keypress:
    ; AL contains the scancode
    ; RDI contains cursor position
    
    ; Check for shift key press/release first
    cmp al, 0x2A                ; Left shift pressed
    je shift_pressed
    cmp al, 0xAA                ; Left shift released
    je shift_released
    cmp al, 0x36                ; Right shift pressed
    je shift_pressed
    cmp al, 0xB6                ; Right shift released
    je shift_released
    
    cmp al, 0x1C                ; Enter key?
    je handle_enter
    
    cmp al, 0x0E                ; Backspace key?
    je handle_backspace
    
    ; Numbers row
    cmp al, 0x02                ; 1 key
    je type_1
    cmp al, 0x03                ; 2 key
    je type_2
    cmp al, 0x04                ; 3 key
    je type_3
    cmp al, 0x05                ; 4 key
    je type_4
    cmp al, 0x06                ; 5 key
    je type_5
    cmp al, 0x07                ; 6 key
    je type_6
    cmp al, 0x08                ; 7 key
    je type_7
    cmp al, 0x09                ; 8 key
    je type_8
    cmp al, 0x0A                ; 9 key
    je type_9
    cmp al, 0x0B                ; 0 key
    je type_0
    
    ; Top row letters
    cmp al, 0x10                ; Q key
    je type_q
    cmp al, 0x11                ; W key
    je type_w
    cmp al, 0x12                ; E key
    je type_e
    cmp al, 0x13                ; R key
    je type_r
    cmp al, 0x14                ; T key
    je type_t
    cmp al, 0x15                ; Y key
    je type_y
    cmp al, 0x16                ; U key
    je type_u
    cmp al, 0x17                ; I key
    je type_i
    cmp al, 0x18                ; O key
    je type_o
    cmp al, 0x19                ; P key
    je type_p
    
    ; Middle row letters
    cmp al, 0x1E                ; A key
    je type_a
    cmp al, 0x1F                ; S key
    je type_s
    cmp al, 0x20                ; D key
    je type_d
    cmp al, 0x21                ; F key
    je type_f
    cmp al, 0x22                ; G key
    je type_g
    cmp al, 0x23                ; H key
    je type_h
    cmp al, 0x24                ; J key
    je type_j
    cmp al, 0x25                ; K key
    je type_k
    cmp al, 0x26                ; L key
    je type_l
    
    ; Bottom row letters
    cmp al, 0x2C                ; Z key
    je type_z
    cmp al, 0x2D                ; X key
    je type_x
    cmp al, 0x2E                ; C key
    je type_c
    cmp al, 0x2F                ; V key
    je type_v
    cmp al, 0x30                ; B key
    je type_b
    cmp al, 0x31                ; N key
    je type_n
    cmp al, 0x32                ; M key
    je type_m
    
    ; Special keys
    cmp al, 0x39                ; Space key
    je type_space
    cmp al, 0x0C                ; - key
    je type_dash
    cmp al, 0x0D                ; = key
    je type_equals
    cmp al, 0x33                ; , key
    je type_comma
    cmp al, 0x34                ; . key
    je type_period
    cmp al, 0x35                ; / key
    je type_slash
    
    ; Unknown key - ignore
    ret

; Shift key handlers
shift_pressed:
    mov byte [shift_state], 1   ; Set shift flag
    ret
    
shift_released:
    mov byte [shift_state], 0   ; Clear shift flag
    ret

section .data
shift_state db 0               ; 0 = no shift, 1 = shift pressed

; Numbers
type_1:
    mov word [rdi], 0x0731
    add rdi, 2
    ret
type_2:
    mov word [rdi], 0x0732
    add rdi, 2
    ret
type_3:
    mov word [rdi], 0x0733
    add rdi, 2
    ret
type_4:
    mov word [rdi], 0x0734
    add rdi, 2
    ret
type_5:
    mov word [rdi], 0x0735
    add rdi, 2
    ret
type_6:
    mov word [rdi], 0x0736
    add rdi, 2
    ret
type_7:
    mov word [rdi], 0x0737
    add rdi, 2
    ret
type_8:
    mov word [rdi], 0x0738
    add rdi, 2
    ret
type_9:
    mov word [rdi], 0x0739
    add rdi, 2
    ret
type_0:
    mov word [rdi], 0x0730
    add rdi, 2
    ret

; Top row
type_q:
    cmp byte [shift_state], 1
    je type_q_upper
    mov word [rdi], 0x0771      ; 'q' lowercase
    add rdi, 2
    ret
type_q_upper:
    mov word [rdi], 0x0751      ; 'Q' uppercase
    add rdi, 2
    ret

type_w:
    cmp byte [shift_state], 1
    je type_w_upper
    mov word [rdi], 0x0777      ; 'w' lowercase
    add rdi, 2
    ret
type_w_upper:
    mov word [rdi], 0x0757      ; 'W' uppercase
    add rdi, 2
    ret

type_e:
    cmp byte [shift_state], 1
    je type_e_upper
    mov word [rdi], 0x0765      ; 'e' lowercase
    add rdi, 2
    ret
type_e_upper:
    mov word [rdi], 0x0745      ; 'E' uppercase
    add rdi, 2
    ret

type_r:
    cmp byte [shift_state], 1
    je type_r_upper
    mov word [rdi], 0x0772      ; 'r' lowercase
    add rdi, 2
    ret
type_r_upper:
    mov word [rdi], 0x0752      ; 'R' uppercase
    add rdi, 2
    ret
type_t:
    cmp byte [shift_state], 1
    je type_t_upper
    mov word [rdi], 0x0774      ; 't' lowercase
    add rdi, 2
    ret
type_t_upper:
    mov word [rdi], 0x0754      ; 'T' uppercase
    add rdi, 2
    ret
type_y:
    mov word [rdi], 0x0779
    add rdi, 2
    ret
type_u:
    mov word [rdi], 0x0775
    add rdi, 2
    ret
type_i:
    mov word [rdi], 0x0769
    add rdi, 2
    ret
type_o:
    mov word [rdi], 0x076F
    add rdi, 2
    ret
type_p:
    mov word [rdi], 0x0770
    add rdi, 2
    ret

; Middle row
type_a:
    cmp byte [shift_state], 1
    je type_a_upper
    mov word [rdi], 0x0761      ; 'a' lowercase
    add rdi, 2
    ret
type_a_upper:
    mov word [rdi], 0x0741      ; 'A' uppercase
    add rdi, 2
    ret

type_s:
    cmp byte [shift_state], 1
    je type_s_upper
    mov word [rdi], 0x0773      ; 's' lowercase
    add rdi, 2
    ret
type_s_upper:
    mov word [rdi], 0x0753      ; 'S' uppercase
    add rdi, 2
    ret

type_d:
    cmp byte [shift_state], 1
    je type_d_upper
    mov word [rdi], 0x0764      ; 'd' lowercase
    add rdi, 2
    ret
type_d_upper:
    mov word [rdi], 0x0744      ; 'D' uppercase
    add rdi, 2
    ret

type_m:
    cmp byte [shift_state], 1
    je type_m_upper
    mov word [rdi], 0x076D      ; 'm' lowercase
    add rdi, 2
    ret
type_m_upper:
    mov word [rdi], 0x074D      ; 'M' uppercase
    add rdi, 2
    ret
type_f:
    mov word [rdi], 0x0766
    add rdi, 2
    ret
type_g:
    mov word [rdi], 0x0767
    add rdi, 2
    ret
type_h:
    mov word [rdi], 0x0768      ; 'h' white on black
    add rdi, 2                  ; Move cursor
    ret
type_j:
    mov word [rdi], 0x076A
    add rdi, 2
    ret
type_k:
    mov word [rdi], 0x076B
    add rdi, 2
    ret
type_l:
    mov word [rdi], 0x076C      ; 'l' white on black
    add rdi, 2
    ret

; Bottom row
type_z:
    mov word [rdi], 0x077A
    add rdi, 2
    ret
type_x:
    mov word [rdi], 0x0778
    add rdi, 2
    ret
type_c:
    cmp byte [shift_state], 1
    je type_c_upper
    mov word [rdi], 0x0763      ; 'c' lowercase
    add rdi, 2
    ret
type_c_upper:
    mov word [rdi], 0x0743      ; 'C' uppercase
    add rdi, 2
    ret
type_v:
    mov word [rdi], 0x0776
    add rdi, 2
    ret
type_b:
    mov word [rdi], 0x0762
    add rdi, 2
    ret
type_n:
    mov word [rdi], 0x076E
    add rdi, 2
    ret
; Special characters
type_space:
    mov word [rdi], 0x0720      ; ' ' space
    add rdi, 2
    ret
type_dash:
    mov word [rdi], 0x072D      ; '-'
    add rdi, 2
    ret
type_equals:
    mov word [rdi], 0x073D      ; '='
    add rdi, 2
    ret
type_comma:
    mov word [rdi], 0x072C      ; ','
    add rdi, 2
    ret
type_period:
    mov word [rdi], 0x072E      ; '.'
    add rdi, 2
    ret
type_slash:
    mov word [rdi], 0x072F      ; '/'
    add rdi, 2
    ret

handle_backspace:
    ; Check if we can backspace (not at the beginning of input line)
    cmp rdi, 0xB8020            ; Beginning of input area?
    je backspace_done           ; Can't backspace before start
    
    ; Move cursor back
    sub rdi, 2
    
    ; Clear the character (space with normal color)
    mov word [rdi], 0x0720      ; Space with normal color
    
backspace_done:
    ret

handle_enter:
    ; Call the command processor
    call process_command
    ret