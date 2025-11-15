/*
 * Simple Text-Mode GUI for WNU OS (QEMU Compatible)
 * Uses VGA text mode instead of graphics mode for better compatibility
 * No BIOS calls - direct VGA buffer access only
 */

#include "vbe_gui.h"

// VGA text mode buffer - directly access video memory
static unsigned char* vga_buffer = (unsigned char*)0xB8000;
static int cursor_x = 40;
static int cursor_y = 12;

// Keyboard state
static unsigned char last_key = 0;

// Dummy functions for compatibility (not used in text mode)
int vbe_get_controller_info(void) { return 1; }
int vbe_get_mode_info(unsigned short mode) { (void)mode; return 1; }
unsigned short vbe_find_mode(unsigned int width, unsigned int height, unsigned int depth) {
    (void)width; (void)height; (void)depth; return 1;
}
int vbe_set_mode(unsigned short mode) { (void)mode; return 1; }
void vbe_set_pixel(int x, int y, unsigned int color) { (void)x; (void)y; (void)color; }
unsigned int vbe_get_pixel(int x, int y) { (void)x; (void)y; return 0; }
void vbe_fill_rect(int x, int y, int width, int height, unsigned int color) { 
    (void)x; (void)y; (void)width; (void)height; (void)color; 
}
void vbe_draw_line(int x0, int y0, int x1, int y1, unsigned int color) {
    (void)x0; (void)y0; (void)x1; (void)y1; (void)color;
}
void vbe_draw_rect(int x, int y, int width, int height, unsigned int color) {
    (void)x; (void)y; (void)width; (void)height; (void)color;
}
void vbe_clear_screen(unsigned int color) { (void)color; }
void vbe_draw_char(int x, int y, char c, unsigned int fg_color, unsigned int bg_color) {
    (void)x; (void)y; (void)c; (void)fg_color; (void)bg_color;
}
void vbe_draw_string(int x, int y, const char* str, unsigned int fg_color, unsigned int bg_color) {
    (void)x; (void)y; (void)str; (void)fg_color; (void)bg_color;
}
void save_cursor_background(void) { }
void restore_cursor_background(void) { }
void draw_cursor(void) { }
void handle_keyboard(void) { }
void draw_window(int x, int y, int width, int height, const char* title) {
    (void)x; (void)y; (void)width; (void)height; (void)title;
}
void draw_desktop(void) { }

// Read keyboard scancode (safe method)
static unsigned char read_keyboard_safe(void) {
    unsigned char status, scancode;
    
    // Read keyboard status
    __asm__ volatile ("inb $0x64, %0" : "=a" (status));
    
    if (status & 0x01) {
        // Data available, read scancode
        __asm__ volatile ("inb $0x60, %0" : "=a" (scancode));
        return scancode;
    }
    
    return 0;
}

// Main GUI entry point - Simple Text Mode GUI (QEMU Compatible)
void start_vbe_gui(void) {
    // Clear screen with spaces
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga_buffer[i] = ' ';     // Character
        vga_buffer[i + 1] = 0x07; // White on black
    }
    
    // Draw title bar
    for (int x = 0; x < 80; x++) {
        vga_buffer[x * 2] = ' ';
        vga_buffer[x * 2 + 1] = 0x17; // White on blue
    }
    
    // Title text
    char* title = "WNU OS - Text Mode GUI (QEMU Compatible)";
    for (int i = 0; title[i] && i < 40; i++) {
        vga_buffer[((5 + i) * 2)] = title[i];
        vga_buffer[((5 + i) * 2) + 1] = 0x17;
    }
    
    // Draw main window
    int win_x = 15, win_y = 3, win_w = 50, win_h = 18;
    
    // Window border using simple ASCII characters
    for (int x = win_x; x < win_x + win_w; x++) {
        // Top border
        vga_buffer[((win_y * 80) + x) * 2] = '-';
        vga_buffer[((win_y * 80) + x) * 2 + 1] = 0x0F;
        // Bottom border
        vga_buffer[(((win_y + win_h - 1) * 80) + x) * 2] = '-';
        vga_buffer[(((win_y + win_h - 1) * 80) + x) * 2 + 1] = 0x0F;
    }
    
    for (int y = win_y; y < win_y + win_h; y++) {
        // Left border
        vga_buffer[((y * 80) + win_x) * 2] = '|';
        vga_buffer[((y * 80) + win_x) * 2 + 1] = 0x0F;
        // Right border
        vga_buffer[((y * 80) + (win_x + win_w - 1)) * 2] = '|';
        vga_buffer[((y * 80) + (win_x + win_w - 1)) * 2 + 1] = 0x0F;
    }
    
    // Window corners
    vga_buffer[((win_y * 80) + win_x) * 2] = '+';
    vga_buffer[((win_y * 80) + (win_x + win_w - 1)) * 2] = '+';
    vga_buffer[(((win_y + win_h - 1) * 80) + win_x) * 2] = '+';
    vga_buffer[(((win_y + win_h - 1) * 80) + (win_x + win_w - 1)) * 2] = '+';
    
    // Window title
    char* win_title = " WNU OS GUI Demo ";
    for (int i = 0; win_title[i] && i < 18; i++) {
        vga_buffer[(((win_y) * 80) + (win_x + 2 + i)) * 2] = win_title[i];
        vga_buffer[(((win_y) * 80) + (win_x + 2 + i)) * 2 + 1] = 0x0F;
    }
    
    // Window content
    char* content1 = "Welcome to WNU OS!";
    for (int i = 0; content1[i] && i < 18; i++) {
        vga_buffer[(((win_y + 3) * 80) + (win_x + 15 + i)) * 2] = content1[i];
        vga_buffer[(((win_y + 3) * 80) + (win_x + 15 + i)) * 2 + 1] = 0x0E;
    }
    
    char* content2 = "Text Mode GUI is working!";
    for (int i = 0; content2[i] && i < 25; i++) {
        vga_buffer[(((win_y + 5) * 80) + (win_x + 12 + i)) * 2] = content2[i];
        vga_buffer[(((win_y + 5) * 80) + (win_x + 12 + i)) * 2 + 1] = 0x0A;
    }
    
    char* content3 = "No graphics mode required!";
    for (int i = 0; content3[i] && i < 27; i++) {
        vga_buffer[(((win_y + 7) * 80) + (win_x + 11 + i)) * 2] = content3[i];
        vga_buffer[(((win_y + 7) * 80) + (win_x + 11 + i)) * 2 + 1] = 0x0C;
    }
    
    char* content4 = "QEMU Compatible Mode";
    for (int i = 0; content4[i] && i < 20; i++) {
        vga_buffer[(((win_y + 9) * 80) + (win_x + 15 + i)) * 2] = content4[i];
        vga_buffer[(((win_y + 9) * 80) + (win_x + 15 + i)) * 2 + 1] = 0x0B;
    }
    
    // Draw cursor
    vga_buffer[((cursor_y * 80) + cursor_x) * 2] = '*';
    vga_buffer[((cursor_y * 80) + cursor_x) * 2 + 1] = 0x0C; // Red cursor
    
    // Status bar
    for (int x = 0; x < 80; x++) {
        vga_buffer[((24 * 80) + x) * 2] = ' ';
        vga_buffer[((24 * 80) + x) * 2 + 1] = 0x70; // Black on white
    }
    
    char* status = "Arrow keys to move cursor | Any other key to exit";
    for (int i = 0; status[i] && i < 48; i++) {
        vga_buffer[((24 * 80) + (2 + i)) * 2] = status[i];
        vga_buffer[((24 * 80) + (2 + i)) * 2 + 1] = 0x70;
    }
    
    // Simple input loop
    while (1) {
        unsigned char scancode = read_keyboard_safe();
        
        if (scancode > 0 && !(scancode & 0x80)) { // Key press (not release)
            // Clear old cursor
            vga_buffer[((cursor_y * 80) + cursor_x) * 2] = ' ';
            vga_buffer[((cursor_y * 80) + cursor_x) * 2 + 1] = 0x07;
            
            // Handle arrow keys
            switch (scancode) {
                case 0x48: // Up arrow
                    if (cursor_y > 1) cursor_y--;
                    break;
                case 0x50: // Down arrow
                    if (cursor_y < 23) cursor_y++;
                    break;
                case 0x4B: // Left arrow
                    if (cursor_x > 0) cursor_x--;
                    break;
                case 0x4D: // Right arrow
                    if (cursor_x < 79) cursor_x++;
                    break;
                default:
                    // Any other key exits
                    goto exit_gui;
            }
            
            // Draw new cursor
            vga_buffer[((cursor_y * 80) + cursor_x) * 2] = '*';
            vga_buffer[((cursor_y * 80) + cursor_x) * 2 + 1] = 0x0C;
        }
        
        // Small delay to prevent excessive CPU usage
        for (volatile int i = 0; i < 50000; i++);
    }
    
exit_gui:
    // Clear screen before returning
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga_buffer[i] = ' ';     // Character
        vga_buffer[i + 1] = 0x07; // White on black
    }
}
