/* Used To Be printf from scratch
* But Now It's the Simple Way And C Language Compatable And Is best For OS Development!
*  Custom printf that converts string to VGA text values with color
* Usage: printf("Hello World", 0, 7); prints string in specified color In First line
*/

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


static volatile unsigned short* vga_memory = (volatile unsigned short*)0xB8000;
static int cursor_position = 0;

// Clear screen function
void clear_screen() {
    for (int i = 0; i < 80 * 25; i++) {
        vga_memory[i] = 0x0720; // Space character with light gray on black
    }
    cursor_position = 0;
}
int vga_print(int char_count, int color, const char* textasciivaule) {
    // Try using 16-bit VGA writes instead of byte writes
    volatile unsigned short* vga_words = (volatile unsigned short*)0xB8000;
    
    // Use the actual parameter value
    unsigned char ascii_value = *textasciivaule + 2;
    
    // Create complete VGA word (color in high byte, character in low byte)
    unsigned short vga_word = (color << 8) | ascii_value;
    
    // Write the complete word
    vga_words[char_count] = vga_word;
    
    return 0;
}

// Simple string output function - outputs string to VGA text mode
void vga_puts(const char* str) {
    static int row = 0;
    static int col = 0;
    
    while (*str) {
        if (*str == '\n') {
            row++;
            col = 0;
            if (row >= 25) {
                // Simple scroll - move everything up one line
                for (int i = 0; i < 24 * 80; i++) {
                    vga_memory[i] = vga_memory[i + 80];
                }
                // Clear last line
                for (int i = 24 * 80; i < 25 * 80; i++) {
                    vga_memory[i] = 0x0720; // Space with light gray on black
                }
                row = 24;
            }
        } else {
            if (col < 80 && row < 25) {
                // Create VGA word: high byte = color (white on black), low byte = character
                vga_memory[row * 80 + col] = (VGA_COLOR_WHITE << 8) | *str;
                col++;
            }
        }
        str++;
    }
}