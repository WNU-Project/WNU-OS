#ifndef VGA_H
#define VGA_H

// function for VGA text output
int vga_print(int char_count, int color, char* textasciivaule);

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

#endif /* VGA_H */