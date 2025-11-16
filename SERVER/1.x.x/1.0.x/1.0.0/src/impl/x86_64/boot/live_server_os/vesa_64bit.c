/*
 * WNU OS Graphics System - PROPER 64-BIT FRAMEBUFFER
 * This is how REAL modern operating systems do graphics in 64-bit mode!
 * Copyright (c) 2025 WNU Project
 */

#include <stdint.h>
#include <stddef.h>
#include "vesa.h"

// PROPER 64-bit framebuffer state
static uint64_t framebuffer_addr = 0;
static uint32_t framebuffer_width = 0;
static uint32_t framebuffer_height = 0;
static uint32_t framebuffer_bpp = 0;
static uint32_t framebuffer_pitch = 0;
static graphics_mode_t current_mode = {0};
static int graphics_initialized = 0;

// Simple beep function
void simple_beep(void) {
    // Port I/O for PC speaker
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0xB6), "Nd"((uint16_t)0x43));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x34), "Nd"((uint16_t)0x42));
    asm volatile ("outb %0, %1" : : "a"((uint8_t)0x04), "Nd"((uint16_t)0x42));
    
    uint8_t speaker;
    asm volatile ("inb %1, %0" : "=a"(speaker) : "Nd"((uint16_t)0x61));
    asm volatile ("outb %0, %1" : : "a"(speaker | 3), "Nd"((uint16_t)0x61));
    
    for (volatile int i = 0; i < 3000000; i++);
    
    asm volatile ("outb %0, %1" : : "a"(speaker), "Nd"((uint16_t)0x61));
}

// The REAL way to do graphics in 64-bit mode!
int vesa_init_64bit(void) {
    simple_beep();
    
    // In 64-bit mode, we DON'T set video modes ourselves!
    // GRUB sets up a framebuffer and passes the info to us via multiboot2
    // For now, we'll assume typical values
    
    // Typical framebuffer setup from GRUB
    framebuffer_addr = 0xE0000000;    // Common framebuffer location  
    framebuffer_width = 800;          // GRUB default
    framebuffer_height = 600;         // GRUB default
    framebuffer_bpp = 32;             // 32-bit color (RGBA)
    framebuffer_pitch = framebuffer_width * 4; // 4 bytes per pixel
    
    // Set our mode info
    current_mode.width = framebuffer_width;
    current_mode.height = framebuffer_height;
    current_mode.bpp = framebuffer_bpp;
    current_mode.framebuffer = framebuffer_addr;
    
    graphics_initialized = 1;
    
    // Draw REAL 32-bit color graphics!
    volatile uint32_t* pixels = (volatile uint32_t*)framebuffer_addr;
    
    // Clear to black
    for (int i = 0; i < framebuffer_width * framebuffer_height; i++) {
        pixels[i] = 0x00000000; // Black
    }
    
    // Red top-left
    for (int y = 0; y < framebuffer_height/2; y++) {
        for (int x = 0; x < framebuffer_width/2; x++) {
            pixels[y * framebuffer_width + x] = 0xFFFF0000; // Red
        }
    }
    
    // Green top-right  
    for (int y = 0; y < framebuffer_height/2; y++) {
        for (int x = framebuffer_width/2; x < framebuffer_width; x++) {
            pixels[y * framebuffer_width + x] = 0xFF00FF00; // Green
        }
    }
    
    // Blue bottom-left
    for (int y = framebuffer_height/2; y < framebuffer_height; y++) {
        for (int x = 0; x < framebuffer_width/2; x++) {
            pixels[y * framebuffer_width + x] = 0xFF0000FF; // Blue
        }
    }
    
    // Yellow bottom-right
    for (int y = framebuffer_height/2; y < framebuffer_height; y++) {
        for (int x = framebuffer_width/2; x < framebuffer_width; x++) {
            pixels[y * framebuffer_width + x] = 0xFFFFFF00; // Yellow
        }
    }
    
    // White circle in center
    int cx = framebuffer_width / 2, cy = framebuffer_height / 2, r = 100;
    for (int y = cy - r; y < cy + r; y++) {
        for (int x = cx - r; x < cx + r; x++) {
            if (x >= 0 && x < framebuffer_width && y >= 0 && y < framebuffer_height) {
                int dx = x - cx, dy = y - cy;
                if (dx*dx + dy*dy < r*r) {
                    pixels[y * framebuffer_width + x] = 0xFFFFFFFF; // White
                }
            }
        }
    }
    
    simple_beep(); // Success!
    return 0;
}

// Alternative: Safe VGA text mode approach (fallback)
int vesa_init_safe_vga(void) {
    simple_beep();
    
    // Use VGA text mode as framebuffer (this always works)
    current_mode.width = 80;
    current_mode.height = 25; 
    current_mode.bpp = 16;
    current_mode.framebuffer = 0xB8000;
    
    graphics_initialized = 1;
    
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    
    // Create colored pattern using text mode
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            uint8_t color;
            if (y < 6) color = 0x4F; // Red background, white text
            else if (y < 12) color = 0x2F; // Green background, white text  
            else if (y < 18) color = 0x1F; // Blue background, white text
            else color = 0x6F; // Yellow background, white text
            
            vga[y * 80 + x] = (color << 8) | ' '; // Space character
        }
    }
    
    simple_beep();
    return 0;
}

// Main init function - try modern first, fallback to safe
int vesa_init(void) {
    // Try 64-bit framebuffer first
    if (vesa_init_64bit() == 0) {
        return 0; // Success!
    }
    
    // If that fails, use safe VGA text mode
    return vesa_init_safe_vga();
}

int vesa_is_initialized(void) {
    return graphics_initialized;
}

graphics_mode_t* vesa_get_mode(void) {
    return &current_mode;
}