/*
 * Linux Compatibility Layer for WNU OS
 */

#include "vga.h"

// Linux application loader using safe printf
int load_linux_app(const char* app_name) {
    vga(80, 7, "\x4C"); // L
    vga(81, 7, "\x49"); // I
    vga(82, 7, "\x4E"); // N
    vga(83, 7, "\x55"); // U
    vga(84, 7, "\x58"); // X
    vga(85, 7, "\x20"); // Space
    vga(86, 7, "\x41"); // A
    vga(87, 7, "\x50"); // P
    vga(88, 7, "\x50"); // P
    vga(89, 7, "\x20"); // Space
    vga(90, 7, "\x4E"); // N
    vga(91, 7, "\x4F"); // O
    vga(92, 7, "\x54"); // T
    vga(93, 7, "\x20"); // Space
    vga(94, 7, "\x46"); // F
    vga(95, 7, "\x4F"); // O
    vga(96, 7, "\x55"); // U
    vga(97, 7, "\x4E"); // N
    vga(98, 7, "\x44"); // D
    return 0;
}
