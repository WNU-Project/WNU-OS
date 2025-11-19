#ifndef VGA_H
#define VGA_H

// function for VGA text output
int vga_print(int char_count, int color, const char* textasciivaule);

// Simple string output function
void vga_puts(const char* str);

// Clear screen function
void clear_screen();

// Define VGA Colors
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_WHITE         7
#define VGA_COLOR_GRAY          8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_BRIGHT_WHITE  15

// Define Letters
#define VGA_LETTER_A "\x41"
#define VGA_LETTER_B "\x42"
#define VGA_LETTER_C "\x43"
#define VGA_LETTER_D "\x44"
#define VGA_LETTER_E "\x45"
#define VGA_LETTER_F "\x46"
#define VGA_LETTER_G "\x47"
#define VGA_LETTER_H "\x48"
#define VGA_LETTER_I "\x49"
#define VGA_LETTER_J "\x4A"
#define VGA_LETTER_K "\x4B"
#define VGA_LETTER_L "\x4C"
#define VGA_LETTER_M "\x4D"
#define VGA_LETTER_N "\x4E"
#define VGA_LETTER_O "\x4F"
#define VGA_LETTER_P "\x50"
#define VGA_LETTER_Q "\x51"
#define VGA_LETTER_R "\x52"
#define VGA_LETTER_S "\x53"
#define VGA_LETTER_T "\x54"
#define VGA_LETTER_U "\x55"
#define VGA_LETTER_V "\x56"
#define VGA_LETTER_W "\x57"
#define VGA_LETTER_X "\x58"
#define VGA_LETTER_Y "\x59"
#define VGA_LETTER_Z "\x5A"

#endif /* VGA_H */