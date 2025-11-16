/*
 * WNU OS Graphics GUI - Clean Version WITH BEEPS AND WHITE TEXT!
 * Simple graphics interface for WNU OS SERVER 1.0.0
 * Copyright (c) 2025 WNU Project
 */

#include <stdint.h>
#include <stddef.h>
#include "vesa.h"
#include "vga.h"

// Static variables for text mode GUI tracking
static int gui_active = 0;
static int text_mode_active = 0;
static unsigned char last_key = 0;

// External beep function from vesa.c
extern void pc_beep(uint32_t frequency);
extern void simple_beep(void);

// Helper function to print strings to VGA using vga_print() correctly - ALL WHITE!
void print_message(int start_pos, uint8_t color, const char* message) {
    // FORCE ALL TEXT TO BE WHITE!
    color = 0x0F; // White on black
    
    int i = 0;
    while (message[i] != '\0' && i < 80) {  // Limit to screen width
        // Use the format you showed: vga_print(1, 7, "\x41"); for 'A'
        // Create hex string for current character
        char hex_string[5];
        unsigned char ascii = (unsigned char)message[i];
        hex_string[0] = '\\';
        hex_string[1] = 'x';
        // Convert to hex digits
        hex_string[2] = (ascii >> 4) < 10 ? '0' + (ascii >> 4) : 'A' + (ascii >> 4) - 10;
        hex_string[3] = (ascii & 0xF) < 10 ? '0' + (ascii & 0xF) : 'A' + (ascii & 0xF) - 10;
        hex_string[4] = '\0';
        
        vga_print(start_pos + i, color, hex_string);
        i++;
    }
}

// Simple text mode GUI fallback
void start_text_mode_gui(void) {
    print_message(80, 0x0F, "TEXT MODE GUI ACTIVE");
    print_message(160, 0x07, "Graphics not available, using text mode.");
    text_mode_active = 1;
}

// Main GUI entry point - BEEPS AND WHITE TEXT VERSION!
void start_vbe_gui(void) {
    gui_active = 1;
    
    // BEEP at start using simple beep!
    simple_beep();
    
    // Show starting message "GUI STARTING..." using direct vga_print calls - ALL WHITE!
    vga_print(160, 15, "\x47");  // G - WHITE (15)
    vga_print(161, 15, "\x55");  // U - WHITE
    vga_print(162, 15, "\x49");  // I - WHITE
    vga_print(163, 15, "\x20");  // space - WHITE
    vga_print(164, 15, "\x53");  // S - WHITE
    vga_print(165, 15, "\x54");  // T - WHITE
    vga_print(166, 15, "\x41");  // A - WHITE
    vga_print(167, 15, "\x52");  // R - WHITE
    vga_print(168, 15, "\x54");  // T - WHITE
    vga_print(169, 15, "\x49");  // I - WHITE
    vga_print(170, 15, "\x4E");  // N - WHITE
    vga_print(171, 15, "\x47");  // G - WHITE
    
    // Delay for visibility
    for (volatile int i = 0; i < 50000000; i++);
    
    // Another beep before graphics!
    simple_beep();
    
    // Initialize VESA graphics
    int vesa_result = vesa_init();
    
    if (vesa_result != 0) {
        // VESA failed - show "VESA FAILED!" in WHITE with beep
        simple_beep(); // Error beep
        vga_print(240, 15, "\x56");  // V - WHITE
        vga_print(241, 15, "\x45");  // E - WHITE
        vga_print(242, 15, "\x53");  // S - WHITE
        vga_print(243, 15, "\x41");  // A - WHITE
        vga_print(244, 15, "\x20");  // space - WHITE
        vga_print(245, 15, "\x46");  // F - WHITE
        vga_print(246, 15, "\x41");  // A - WHITE
        vga_print(247, 15, "\x49");  // I - WHITE
        vga_print(248, 15, "\x4C");  // L - WHITE
        vga_print(249, 15, "\x45");  // E - WHITE
        vga_print(250, 15, "\x44");  // D - WHITE
        vga_print(251, 15, "\x21");  // ! - WHITE
        
        start_text_mode_gui();
        return;
    }
    
    // VESA succeeded - show "VESA SUCCESS!" in WHITE with success beep!
    simple_beep(); // Success beep
    vga_print(320, 15, "\x56");  // V - WHITE
    vga_print(321, 15, "\x45");  // E - WHITE
    vga_print(322, 15, "\x53");  // S - WHITE
    vga_print(323, 15, "\x41");  // A - WHITE
    vga_print(324, 15, "\x20");  // space - WHITE
    vga_print(325, 15, "\x53");  // S - WHITE
    vga_print(326, 15, "\x55");  // U - WHITE
    vga_print(327, 15, "\x43");  // C - WHITE
    vga_print(328, 15, "\x43");  // C - WHITE
    vga_print(329, 15, "\x45");  // E - WHITE
    vga_print(330, 15, "\x53");  // S - WHITE
    vga_print(331, 15, "\x53");  // S - WHITE
    vga_print(332, 15, "\x21");  // ! - WHITE
    
    // Draw test pattern with real graphics
    vesa_clear_screen(COLOR_CYAN);
    vesa_fill_rect(100, 100, 200, 100, COLOR_RED);
    vesa_fill_rect(350, 100, 200, 100, COLOR_GREEN);
    vesa_fill_rect(600, 100, 200, 100, COLOR_BLUE);
    
    // Show "COMPLETE!" in WHITE at bottom with final beep celebration!
    simple_beep(); // Celebration beep
    vga_print(480, 15, "\x43");  // C - WHITE
    vga_print(481, 15, "\x4F");  // O - WHITE
    vga_print(482, 15, "\x4D");  // M - WHITE
    vga_print(483, 15, "\x50");  // P - WHITE
    vga_print(484, 15, "\x4C");  // L - WHITE
    vga_print(485, 15, "\x45");  // E - WHITE
    vga_print(486, 15, "\x54");  // T - WHITE
    vga_print(487, 15, "\x45");  // E - WHITE
    vga_print(488, 11, "\x21");  // !
    
    // Final delay
    for (volatile int i = 0; i < 50000000; i++);
    
    gui_active = 0;
    return;
}

// Check if GUI is active
int is_gui_active(void) {
    return gui_active;
}

// Check if text mode GUI is active  
int is_text_mode_active(void) {
    return text_mode_active;
}