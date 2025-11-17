/*
 * WNU OS Text Editor Application
 * Advanced text editor with cursor movement, file operations, and syntax highlighting
 * Copyright (c) 2025 WNU Project
 */

#ifndef WNU_EDITOR_H
#define WNU_EDITOR_H

#include <stdint.h>
#include <stddef.h>

// Editor configuration
#define EDITOR_LINES 20           // Visible lines on screen
#define EDITOR_COLS 75            // Visible columns per line
#define EDITOR_MAX_LINES 1000     // Maximum lines in document
#define EDITOR_LINE_LENGTH 256    // Maximum characters per line
#define EDITOR_FILENAME_LENGTH 64 // Maximum filename length

// Editor modes
typedef enum {
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND,
    MODE_SEARCH,
    MODE_SAVE
} editor_mode_t;

// Text buffer line structure
typedef struct text_line {
    char data[EDITOR_LINE_LENGTH];
    int length;
    struct text_line* next;
    struct text_line* prev;
} text_line_t;

// Editor state structure
typedef struct {
    // Text buffer
    text_line_t* lines[EDITOR_MAX_LINES];
    int line_count;
    
    // Cursor position
    int cursor_x, cursor_y;     // Screen position
    int buffer_x, buffer_y;     // Buffer position
    
    // View position
    int scroll_x, scroll_y;     // Scroll offsets
    
    // Editor state
    editor_mode_t mode;
    char filename[EDITOR_FILENAME_LENGTH];
    int modified;
    int running;
    
    // Status line
    char status_message[256];
    int status_timeout;
    
    // Search
    char search_query[128];
    int search_direction;
    
    // Copy/paste buffer
    char clipboard[EDITOR_LINE_LENGTH];
    
    // Command buffer
    char command_buffer[256];
    
} editor_state_t;

// Key codes
#define KEY_BACKSPACE   0x08
#define KEY_TAB         0x09
#define KEY_ENTER       0x0D
#define KEY_ESC         0x1B
#define KEY_SPACE       0x20
#define KEY_DELETE      0x7F

// Arrow keys (extended codes)
#define KEY_UP          0x80
#define KEY_DOWN        0x81
#define KEY_LEFT        0x82
#define KEY_RIGHT       0x83
#define KEY_HOME        0x84
#define KEY_END         0x85
#define KEY_PAGEUP      0x86
#define KEY_PAGEDOWN    0x87

// Control keys
#define KEY_CTRL_S      0x13
#define KEY_CTRL_Q      0x11
#define KEY_CTRL_X      0x18
#define KEY_CTRL_C      0x03
#define KEY_CTRL_V      0x16
#define KEY_CTRL_F      0x06
#define KEY_CTRL_G      0x07

// Function prototypes
int app_edit(int argc, char* argv[]);
void editor_init(const char* filename);
void editor_run(void);
void editor_shutdown(void);

// Text buffer operations
text_line_t* editor_create_line(void);
void editor_insert_line(int y);
void editor_delete_line(int y);
void editor_insert_char(char c);
void editor_delete_char(void);
void editor_delete_char_before(void);

// File operations
int editor_load_file(const char* filename);
int editor_save_file(const char* filename);
int editor_save_as(const char* filename);

// Cursor movement
void editor_move_cursor_up(void);
void editor_move_cursor_down(void);
void editor_move_cursor_left(void);
void editor_move_cursor_right(void);
void editor_move_cursor_home(void);
void editor_move_cursor_end(void);
void editor_move_cursor_page_up(void);
void editor_move_cursor_page_down(void);
void editor_goto_line(int line);

// Screen operations
void editor_refresh_screen(void);
void editor_draw_lines(void);
void editor_draw_status_bar(void);
void editor_draw_mode_indicator(void);
void editor_draw_line_numbers(void);
void editor_update_cursor(void);
void editor_scroll_if_needed(void);

// Input handling
char editor_get_key(void);
void editor_process_key(char key);
void editor_handle_normal_mode(char key);
void editor_handle_insert_mode(char key);
void editor_handle_command_mode(char key);

// Search functionality
void editor_start_search(void);
void editor_find_next(void);
void editor_find_previous(void);
int editor_search_in_line(text_line_t* line, const char* query, int start_pos);

// Copy/paste operations
void editor_copy_line(void);
void editor_cut_line(void);
void editor_paste_line(void);

// Status and messages
void editor_set_status_message(const char* message);
void editor_show_help(void);

// Syntax highlighting (basic)
uint8_t editor_get_syntax_color(char c, int position);

// Utility functions
int editor_strlen(const char* str);
char* editor_strcpy(char* dest, const char* src);
char* editor_strncpy(char* dest, const char* src, int n);
int editor_strcmp(const char* s1, const char* s2);
int editor_strncmp(const char* s1, const char* s2, int n);

#endif // WNU_EDITOR_H