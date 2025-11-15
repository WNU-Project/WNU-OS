/*
 * Linux Compatibility Layer for WNU OS
 */

#include "vga.h"

// Linux application loader using safe printf
int load_linux_app(const char* app_name) {
    vga(80, 7, "\x4C");
    vga(81, 7, "\x49");
    vga(82, 7, "\x4E");
    return 0;
}
