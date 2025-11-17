/*
 * Linux Compatibility Layer for WNU OS
 */

#include "vga.h"

// Linux application loader using vga() function
int load_linux_app(const char* app_name) {
    vga_print(80, 7, VGA_LETTER_L); // L
    vga_print(81, 7, "\x49"); // I
    vga_print(82, 7, "\x4E"); // N
    vga_print(83, 7, "\x55"); // U
    vga_print(84, 7, "\x58"); // X
    vga_print(85, 7, "\x20"); // Space
    vga_print(86, 7, "\x41"); // A
    vga_print(87, 7, "\x50"); // P
    vga_print(88, 7, "\x50"); // P
    vga_print(89, 7, "\x20"); // Space
    vga_print(90, 7, "\x4E"); // N
    vga_print(91, 7, "\x4F"); // O
    vga_print(92, 7, "\x54"); // T
    vga_print(93, 7, "\x20"); // Space
    vga_print(94, 7, "\x46"); // F
    vga_print(95, 7, "\x4F"); // O
    vga_print(96, 7, "\x55"); // U
    vga_print(97, 7, "\x4E"); // N
    vga_print(98, 7, "\x44"); // D
    return 0;
}