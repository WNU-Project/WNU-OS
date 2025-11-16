/*
 * WNU OS Graphics System - VESA Interface Header
 * Simplified graphics interface for WNU OS SERVER 1.0.0
 * Copyright (c) 2025 WNU Project
 */

#ifndef VESA_H
#define VESA_H

#include <stdint.h>

// Graphics mode structure
typedef struct {
    int width;
    int height;
    int bpp;
    uint32_t framebuffer;
} graphics_mode_t;

// Color constants
#define COLOR_BLACK     0x000000
#define COLOR_WHITE     0xFFFFFF
#define COLOR_RED       0xFF0000
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_YELLOW    0xFFFF00
#define COLOR_CYAN      0x00FFFF
#define COLOR_MAGENTA   0xFF00FF

// Function prototypes
int vesa_init(void);
int vesa_is_initialized(void);
void vesa_clear_screen(uint32_t color);
void vesa_set_pixel(int x, int y, uint32_t color);
uint32_t vesa_get_pixel(int x, int y);
void vesa_fill_rect(int x, int y, int width, int height, uint32_t color);
void vesa_draw_line(int x0, int y0, int x1, int y1, uint32_t color);
graphics_mode_t* vesa_get_mode(void);
int vesa_set_mode(int width, int height, int bpp);
void vesa_shutdown(void);
uint32_t vesa_get_framebuffer(void);
void vesa_test_pattern(void);

// Audio functions
void simple_beep(void);

#endif // VESA_H
