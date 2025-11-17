/*
 * WNU OS Text Editor Implementation
 * Full-featured text editor with syntax highlighting and file operations
 * Copyright (c) 2025 WNU Project
 */

#include "wnu_editor.h"
#include "wnu_shell.h"
#include "vga.h"

// Global editor state
static editor_state_t editor;

// Utility functions
int editor_strlen(const char* str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

char* editor_strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

int editor_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Create new text line
text_line_t* editor_create_line(void) {
    static text_line_t line_pool[EDITOR_MAX_LINES];
    static int pool_index = 0;
    
    if (pool_index >= EDITOR_MAX_LINES) return NULL;
    
    text_line_t* line = &line_pool[pool_index++];
    line->data[0] = '\0';
    line->length = 0;
    line->next = NULL;
    line->prev = NULL;
    
    return line;
}

// Initialize editor
void editor_init(const char* filename) {
    // Initialize state
    editor.line_count = 0;
    editor.cursor_x = 0;
    editor.cursor_y = 0;
    editor.buffer_x = 0;
    editor.buffer_y = 0;
    editor.scroll_x = 0;
    editor.scroll_y = 0;
    editor.mode = MODE_NORMAL;
    editor.modified = 0;
    editor.running = 1;
    editor.status_timeout = 0;
    editor.search_direction = 1;
    
    // Initialize buffers
    editor.clipboard[0] = '\0';
    editor.command_buffer[0] = '\0';
    editor.search_query[0] = '\0';
    editor.status_message[0] = '\0';
    
    // Set filename
    if (filename) {
        editor_strcpy(editor.filename, filename);
    } else {
        editor_strcpy(editor.filename, "untitled.txt");
    }
    
    // Initialize with empty lines
    for (int i = 0; i < EDITOR_MAX_LINES; i++) {
        editor.lines[i] = NULL;
    }
    
    // Create first line
    editor.lines[0] = editor_create_line();
    editor.line_count = 1;
    
    // Try to load file if filename provided
    if (filename && editor_load_file(filename)) {
        editor_set_status_message("File loaded successfully");
    } else {
        editor_set_status_message("New file - WNU Editor v1.0");
    }
}

// Main editor loop
void editor_run(void) {
    editor_refresh_screen();
    
    while (editor.running) {
        char key = editor_get_key();
        if (key) {
            editor_process_key(key);
            editor_refresh_screen();
        }
    }
}

// Get key input (simplified for demo)
char editor_get_key(void) {
    // In a real implementation, this would read from keyboard
    // For demo, simulate user typing
    static const char demo_keys[] = {
        'i',           // Enter insert mode
        'H', 'e', 'l', 'l', 'o', ' ',
        'W', 'o', 'r', 'l', 'd', '!', '\n',
        'T', 'h', 'i', 's', ' ', 'i', 's', ' ',
        'a', ' ', 't', 'e', 'x', 't', ' ',
        'e', 'd', 'i', 't', 'o', 'r', '\n',
        'w', 'r', 'i', 't', 't', 'e', 'n', ' ',
        'i', 'n', ' ', 'C', '!', '\n',
        KEY_ESC,       // Exit insert mode
        ':', 'w', 'q', '\n' // Save and quit
    };
    static int demo_index = 0;
    static int demo_delay = 0;
    
    // Add delay between keystrokes for demo effect
    if (demo_delay++ < 50000) return 0;
    demo_delay = 0;
    
    if (demo_index < sizeof(demo_keys)) {
        return demo_keys[demo_index++];
    }
    
    // End demo
    editor.running = 0;
    return 0;
}

// Process key input
void editor_process_key(char key) {
    switch (editor.mode) {
        case MODE_NORMAL:
            editor_handle_normal_mode(key);
            break;
        case MODE_INSERT:
            editor_handle_insert_mode(key);
            break;
        case MODE_COMMAND:
            editor_handle_command_mode(key);
            break;
        case MODE_SEARCH:
            // Handle search mode
            break;
        default:
            break;
    }
}

// Handle normal mode keys (vi-like commands)
void editor_handle_normal_mode(char key) {
    switch (key) {
        case 'i': // Enter insert mode
            editor.mode = MODE_INSERT;
            editor_set_status_message("-- INSERT --");
            break;
        case 'a': // Enter insert mode after cursor
            editor_move_cursor_right();
            editor.mode = MODE_INSERT;
            editor_set_status_message("-- INSERT --");
            break;
        case 'h': case KEY_LEFT:
            editor_move_cursor_left();
            break;
        case 'j': case KEY_DOWN:
            editor_move_cursor_down();
            break;
        case 'k': case KEY_UP:
            editor_move_cursor_up();
            break;
        case 'l': case KEY_RIGHT:
            editor_move_cursor_right();
            break;
        case '0': case KEY_HOME:
            editor_move_cursor_home();
            break;
        case '$': case KEY_END:
            editor_move_cursor_end();
            break;
        case 'x': // Delete character
            editor_delete_char();
            editor.modified = 1;
            break;
        case 'o': // Open new line below
            editor_move_cursor_end();
            editor_insert_char('\n');
            editor.mode = MODE_INSERT;
            editor_set_status_message("-- INSERT --");
            break;
        case 'O': // Open new line above
            editor_move_cursor_home();
            editor_insert_char('\n');
            editor_move_cursor_up();
            editor.mode = MODE_INSERT;
            editor_set_status_message("-- INSERT --");
            break;
        case '/': // Start search
            editor.mode = MODE_SEARCH;
            editor_set_status_message("Search: ");
            break;
        case ':': // Enter command mode
            editor.mode = MODE_COMMAND;
            editor.command_buffer[0] = '\0';
            editor_set_status_message(":");
            break;
        case 'q': // Quick quit
            editor.running = 0;
            break;
        default:
            break;
    }
}

// Handle insert mode keys
void editor_handle_insert_mode(char key) {
    switch (key) {
        case KEY_ESC:
            editor.mode = MODE_NORMAL;
            editor_set_status_message("");
            break;
        case KEY_BACKSPACE:
            editor_delete_char_before();
            editor.modified = 1;
            break;
        case KEY_DELETE:
            editor_delete_char();
            editor.modified = 1;
            break;
        case KEY_ENTER:
        case '\n':
            editor_insert_char('\n');
            editor.modified = 1;
            break;
        case KEY_TAB:
            // Insert 4 spaces for tab
            for (int i = 0; i < 4; i++) {
                editor_insert_char(' ');
            }
            editor.modified = 1;
            break;
        case KEY_LEFT:
            editor_move_cursor_left();
            break;
        case KEY_RIGHT:
            editor_move_cursor_right();
            break;
        case KEY_UP:
            editor_move_cursor_up();
            break;
        case KEY_DOWN:
            editor_move_cursor_down();
            break;
        default:
            if (key >= 32 && key <= 126) { // Printable characters
                editor_insert_char(key);
                editor.modified = 1;
            }
            break;
    }
}

// Handle command mode
void editor_handle_command_mode(char key) {
    static int cmd_pos = 0;
    
    if (key == KEY_ESC) {
        editor.mode = MODE_NORMAL;
        editor_set_status_message("");
        cmd_pos = 0;
        return;
    }
    
    if (key == KEY_ENTER || key == '\n') {
        // Execute command
        if (editor_strcmp(editor.command_buffer, "w") == 0) {
            // Save file
            if (editor_save_file(editor.filename)) {
                editor_set_status_message("File saved");
                editor.modified = 0;
            } else {
                editor_set_status_message("Error saving file");
            }
        } else if (editor_strcmp(editor.command_buffer, "q") == 0) {
            // Quit
            if (!editor.modified) {
                editor.running = 0;
            } else {
                editor_set_status_message("File modified! Use :q! to force quit or :wq to save and quit");
            }
        } else if (editor_strcmp(editor.command_buffer, "q!") == 0) {
            // Force quit
            editor.running = 0;
        } else if (editor_strcmp(editor.command_buffer, "wq") == 0) {
            // Save and quit
            if (editor_save_file(editor.filename)) {
                editor.running = 0;
            } else {
                editor_set_status_message("Error saving file");
            }
        } else {
            editor_set_status_message("Unknown command");
        }
        
        editor.mode = MODE_NORMAL;
        cmd_pos = 0;
        return;
    }
    
    if (key == KEY_BACKSPACE && cmd_pos > 0) {
        cmd_pos--;
        editor.command_buffer[cmd_pos] = '\0';
        return;
    }
    
    if (key >= 32 && key <= 126 && cmd_pos < 255) {
        editor.command_buffer[cmd_pos++] = key;
        editor.command_buffer[cmd_pos] = '\0';
        
        // Update status message
        char status[300];
        status[0] = ':';
        editor_strcpy(status + 1, editor.command_buffer);
        editor_set_status_message(status);
    }
}

// Insert character at current position
void editor_insert_char(char c) {
    if (c == '\n') {
        // Create new line
        editor_insert_line(editor.buffer_y + 1);
        editor_move_cursor_down();
        editor_move_cursor_home();
        return;
    }
    
    if (editor.buffer_y >= editor.line_count) return;
    
    text_line_t* line = editor.lines[editor.buffer_y];
    if (!line || line->length >= EDITOR_LINE_LENGTH - 1) return;
    
    // Shift characters to the right
    for (int i = line->length; i > editor.buffer_x; i--) {
        line->data[i] = line->data[i - 1];
    }
    
    line->data[editor.buffer_x] = c;
    line->length++;
    line->data[line->length] = '\0';
    
    editor_move_cursor_right();
}

// Delete character at cursor
void editor_delete_char(void) {
    if (editor.buffer_y >= editor.line_count) return;
    
    text_line_t* line = editor.lines[editor.buffer_y];
    if (!line) return;
    
    if (editor.buffer_x >= line->length) {
        // Delete newline - join with next line
        if (editor.buffer_y + 1 < editor.line_count) {
            text_line_t* next_line = editor.lines[editor.buffer_y + 1];
            if (next_line && line->length + next_line->length < EDITOR_LINE_LENGTH - 1) {
                // Append next line content
                for (int i = 0; i < next_line->length; i++) {
                    line->data[line->length + i] = next_line->data[i];
                }
                line->length += next_line->length;
                line->data[line->length] = '\0';
                
                // Remove next line
                editor_delete_line(editor.buffer_y + 1);
            }
        }
        return;
    }
    
    // Shift characters to the left
    for (int i = editor.buffer_x; i < line->length - 1; i++) {
        line->data[i] = line->data[i + 1];
    }
    line->length--;
    line->data[line->length] = '\0';
}

// Delete character before cursor (backspace)
void editor_delete_char_before(void) {
    if (editor.buffer_x > 0) {
        editor_move_cursor_left();
        editor_delete_char();
    } else if (editor.buffer_y > 0) {
        // Join with previous line
        text_line_t* prev_line = editor.lines[editor.buffer_y - 1];
        text_line_t* curr_line = editor.lines[editor.buffer_y];
        
        if (prev_line && curr_line && 
            prev_line->length + curr_line->length < EDITOR_LINE_LENGTH - 1) {
            
            editor.buffer_x = prev_line->length;
            editor.cursor_x = editor.buffer_x;
            
            // Append current line to previous
            for (int i = 0; i < curr_line->length; i++) {
                prev_line->data[prev_line->length + i] = curr_line->data[i];
            }
            prev_line->length += curr_line->length;
            prev_line->data[prev_line->length] = '\0';
            
            // Remove current line
            editor_delete_line(editor.buffer_y);
            editor_move_cursor_up();
        }
    }
}

// Insert new line
void editor_insert_line(int y) {
    if (y > editor.line_count || editor.line_count >= EDITOR_MAX_LINES - 1) return;
    
    // Shift lines down
    for (int i = editor.line_count; i > y; i--) {
        editor.lines[i] = editor.lines[i - 1];
    }
    
    editor.lines[y] = editor_create_line();
    editor.line_count++;
}

// Delete line
void editor_delete_line(int y) {
    if (y >= editor.line_count) return;
    
    // Shift lines up
    for (int i = y; i < editor.line_count - 1; i++) {
        editor.lines[i] = editor.lines[i + 1];
    }
    
    editor.line_count--;
    editor.lines[editor.line_count] = NULL;
}

// Cursor movement functions
void editor_move_cursor_left(void) {
    if (editor.buffer_x > 0) {
        editor.buffer_x--;
        editor.cursor_x--;
    } else if (editor.buffer_y > 0) {
        editor_move_cursor_up();
        editor_move_cursor_end();
    }
}

void editor_move_cursor_right(void) {
    if (editor.buffer_y < editor.line_count) {
        text_line_t* line = editor.lines[editor.buffer_y];
        if (line && editor.buffer_x < line->length) {
            editor.buffer_x++;
            editor.cursor_x++;
        } else if (editor.buffer_y + 1 < editor.line_count) {
            editor_move_cursor_down();
            editor_move_cursor_home();
        }
    }
}

void editor_move_cursor_up(void) {
    if (editor.buffer_y > 0) {
        editor.buffer_y--;
        editor.cursor_y--;
        
        // Adjust x position if line is shorter
        text_line_t* line = editor.lines[editor.buffer_y];
        if (line && editor.buffer_x > line->length) {
            editor.buffer_x = line->length;
            editor.cursor_x = editor.buffer_x;
        }
    }
}

void editor_move_cursor_down(void) {
    if (editor.buffer_y + 1 < editor.line_count) {
        editor.buffer_y++;
        editor.cursor_y++;
        
        // Adjust x position if line is shorter
        text_line_t* line = editor.lines[editor.buffer_y];
        if (line && editor.buffer_x > line->length) {
            editor.buffer_x = line->length;
            editor.cursor_x = editor.buffer_x;
        }
    }
}

void editor_move_cursor_home(void) {
    editor.buffer_x = 0;
    editor.cursor_x = 0;
}

void editor_move_cursor_end(void) {
    if (editor.buffer_y < editor.line_count) {
        text_line_t* line = editor.lines[editor.buffer_y];
        if (line) {
            editor.buffer_x = line->length;
            editor.cursor_x = editor.buffer_x;
        }
    }
}

// Set status message
void editor_set_status_message(const char* message) {
    editor_strcpy(editor.status_message, message);
    editor.status_timeout = 100; // Display for 100 refresh cycles
}

// Refresh screen
void editor_refresh_screen(void) {
    // Clear screen
    clear_screen();
    
    // Draw editor components
    editor_draw_lines();
    editor_draw_status_bar();
    editor_draw_mode_indicator();
    editor_update_cursor();
    
    if (editor.status_timeout > 0) {
        editor.status_timeout--;
    }
}

// Draw text lines
void editor_draw_lines(void) {
    for (int screen_y = 0; screen_y < EDITOR_LINES && screen_y < editor.line_count; screen_y++) {
        int buffer_line = screen_y + editor.scroll_y;
        
        if (buffer_line < editor.line_count && editor.lines[buffer_line]) {
            text_line_t* line = editor.lines[buffer_line];
            
            // Draw line number
            char line_num[8];
            line_num[0] = '0' + ((buffer_line + 1) / 1000) % 10;
            line_num[1] = '0' + ((buffer_line + 1) / 100) % 10;
            line_num[2] = '0' + ((buffer_line + 1) / 10) % 10;
            line_num[3] = '0' + (buffer_line + 1) % 10;
            line_num[4] = ' ';
            line_num[5] = '\0';
            
            // Position: line_number * 160 (80 chars * 2 bytes per char)
            volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
            int pos = screen_y * 80;
            
            // Draw line number in gray
            for (int i = 0; i < 4; i++) {
                vga_buffer[pos + i] = (VGA_COLOR_GRAY << 8) | line_num[i];
            }
            
            // Draw line content
            for (int x = 0; x < line->length && x + 5 < 80; x++) {
                char c = line->data[x];
                uint8_t color = editor_get_syntax_color(c, x);
                vga_buffer[pos + x + 5] = (color << 8) | c;
            }
        }
    }
}

// Get syntax highlighting color
uint8_t editor_get_syntax_color(char c, int position) {
    // Simple syntax highlighting
    if (c >= '0' && c <= '9') return VGA_COLOR_CYAN;      // Numbers
    if (c == '"' || c == '\'') return VGA_COLOR_YELLOW;   // Strings
    if (c == '{' || c == '}' || c == '(' || c == ')' || 
        c == '[' || c == ']') return VGA_COLOR_MAGENTA;   // Brackets
    if (c == '+' || c == '-' || c == '*' || c == '/' || 
        c == '=') return VGA_COLOR_RED;                   // Operators
    
    return VGA_COLOR_WHITE; // Default
}

// Draw status bar
void editor_draw_status_bar(void) {
    volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
    int status_line = 22 * 80; // Line 22
    
    // Clear status line with different background
    for (int x = 0; x < 80; x++) {
        vga_buffer[status_line + x] = (VGA_COLOR_WHITE << 12) | (VGA_COLOR_BLACK << 8) | ' ';
    }
    
    // Show filename and modification status
    char status[80];
    status[0] = '\0';
    
    // Add filename
    for (int i = 0; editor.filename[i] && i < 30; i++) {
        status[i] = editor.filename[i];
        status[i + 1] = '\0';
    }
    
    // Add modification indicator
    if (editor.modified) {
        int len = editor_strlen(status);
        status[len] = ' ';
        status[len + 1] = '[';
        status[len + 2] = '+';
        status[len + 3] = ']';
        status[len + 4] = '\0';
    }
    
    // Draw status text
    for (int i = 0; status[i] && i < 60; i++) {
        vga_buffer[status_line + i] = (VGA_COLOR_WHITE << 12) | (VGA_COLOR_BLACK << 8) | status[i];
    }
    
    // Show cursor position on right side
    char pos_info[20];
    pos_info[0] = 'L';
    pos_info[1] = ':';
    pos_info[2] = '0' + ((editor.buffer_y + 1) / 100) % 10;
    pos_info[3] = '0' + ((editor.buffer_y + 1) / 10) % 10;
    pos_info[4] = '0' + (editor.buffer_y + 1) % 10;
    pos_info[5] = ' ';
    pos_info[6] = 'C';
    pos_info[7] = ':';
    pos_info[8] = '0' + ((editor.buffer_x + 1) / 10) % 10;
    pos_info[9] = '0' + (editor.buffer_x + 1) % 10;
    pos_info[10] = '\0';
    
    for (int i = 0; i < 10; i++) {
        vga_buffer[status_line + 70 + i] = (VGA_COLOR_WHITE << 12) | (VGA_COLOR_BLACK << 8) | pos_info[i];
    }
}

// Draw mode indicator
void editor_draw_mode_indicator(void) {
    volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
    int msg_line = 23 * 80; // Line 23
    
    // Clear message line
    for (int x = 0; x < 80; x++) {
        vga_buffer[msg_line + x] = (VGA_COLOR_BLACK << 8) | ' ';
    }
    
    // Show status message or mode
    const char* message = editor.status_message;
    if (editor.status_timeout <= 0) {
        switch (editor.mode) {
            case MODE_NORMAL: message = "-- NORMAL --"; break;
            case MODE_INSERT: message = "-- INSERT --"; break;
            case MODE_COMMAND: message = "-- COMMAND --"; break;
            case MODE_SEARCH: message = "-- SEARCH --"; break;
            default: message = ""; break;
        }
    }
    
    uint8_t color = (editor.mode == MODE_INSERT) ? VGA_COLOR_GREEN : VGA_COLOR_CYAN;
    
    for (int i = 0; message[i] && i < 79; i++) {
        vga_buffer[msg_line + i] = (color << 8) | message[i];
    }
}

// Update cursor position on screen
void editor_update_cursor(void) {
    // In a real implementation, this would update the hardware cursor
    // For now, we'll draw a cursor character
    volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
    
    if (editor.cursor_y < EDITOR_LINES && editor.cursor_x + 5 < 80) {
        int pos = editor.cursor_y * 80 + editor.cursor_x + 5; // +5 for line numbers
        
        // Draw cursor as highlighted character
        char cursor_char = ' ';
        if (editor.buffer_y < editor.line_count && editor.lines[editor.buffer_y]) {
            text_line_t* line = editor.lines[editor.buffer_y];
            if (editor.buffer_x < line->length) {
                cursor_char = line->data[editor.buffer_x];
            }
        }
        
        uint8_t cursor_color = (editor.mode == MODE_INSERT) ? 
                              (VGA_COLOR_BLACK << 4) | VGA_COLOR_GREEN :
                              (VGA_COLOR_BLACK << 4) | VGA_COLOR_WHITE;
        
        vga_buffer[pos] = (cursor_color << 8) | cursor_char;
    }
}

// File operations (simplified)
int editor_load_file(const char* filename) {
    // In a real implementation, this would read from filesystem
    // For demo, create some sample content
    
    editor_strcpy(editor.lines[0]->data, "Welcome to WNU Text Editor!");
    editor.lines[0]->length = editor_strlen(editor.lines[0]->data);
    
    editor_insert_line(1);
    editor_strcpy(editor.lines[1]->data, "This is a demonstration of the editor.");
    editor.lines[1]->length = editor_strlen(editor.lines[1]->data);
    
    editor_insert_line(2);
    editor_strcpy(editor.lines[2]->data, "Features include:");
    editor.lines[2]->length = editor_strlen(editor.lines[2]->data);
    
    editor_insert_line(3);
    editor_strcpy(editor.lines[3]->data, "- Syntax highlighting");
    editor.lines[3]->length = editor_strlen(editor.lines[3]->data);
    
    editor_insert_line(4);
    editor_strcpy(editor.lines[4]->data, "- Vi-like keybindings");
    editor.lines[4]->length = editor_strlen(editor.lines[4]->data);
    
    editor_insert_line(5);
    editor_strcpy(editor.lines[5]->data, "- File save/load");
    editor.lines[5]->length = editor_strlen(editor.lines[5]->data);
    
    editor_insert_line(6);
    editor_strcpy(editor.lines[6]->data, "");
    
    editor_insert_line(7);
    editor_strcpy(editor.lines[7]->data, "Press 'i' to enter insert mode.");
    editor.lines[7]->length = editor_strlen(editor.lines[7]->data);
    
    editor_insert_line(8);
    editor_strcpy(editor.lines[8]->data, "Press ESC to return to normal mode.");
    editor.lines[8]->length = editor_strlen(editor.lines[8]->data);
    
    editor_insert_line(9);
    editor_strcpy(editor.lines[9]->data, "Use :w to save, :q to quit, :wq to save and quit.");
    editor.lines[9]->length = editor_strlen(editor.lines[9]->data);
    
    return 1;
}

int editor_save_file(const char* filename) {
    // In a real implementation, this would write to filesystem
    editor_set_status_message("File saved (simulated)");
    return 1;
}

// Main editor application entry point
int app_edit(int argc, char* argv[]) {
    const char* filename = (argc > 1) ? argv[1] : NULL;
    
    // Print editor header
    shell_print_color("╔════════════════════════════════════════════════╗\n", VGA_COLOR_CYAN);
    shell_print_color("║              WNU Text Editor v1.0              ║\n", VGA_COLOR_CYAN);
    shell_print_color("║         Advanced Text Editor with Vi keys     ║\n", VGA_COLOR_CYAN);
    shell_print_color("╚════════════════════════════════════════════════╝\n", VGA_COLOR_CYAN);
    shell_print("\n");
    
    if (filename) {
        shell_print_info("Opening file: ");
        shell_print(filename);
        shell_print("\n");
    } else {
        shell_print_info("Creating new file...\n");
    }
    
    shell_print("\n");
    
    // Initialize and run editor
    editor_init(filename);
    editor_run();
    
    shell_print_success("\nText editor closed.\n");
    return 0;
}