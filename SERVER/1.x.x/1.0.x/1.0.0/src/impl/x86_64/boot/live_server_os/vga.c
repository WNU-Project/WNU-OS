/* Used To Be printf from scratch
* But Now It's the Simple Way And C Language Compatable And Is best For OS Development!
*  Custom printf that converts string to VGA text values with color
* Usage: printf("Hello World", 0, 7); prints string in specified color In First line
*/

static volatile unsigned short* vga_memory = (volatile unsigned short*)0xB8000;
static int cursor_position = 0;

// Clear screen function
void clear_screen() {
    for (int i = 0; i < 80 * 25; i++) {
        vga_memory[i] = 0x0720; // Space character with light gray on black
    }
    cursor_position = 0;
}
int vga_print(int char_count, int color, char* textasciivaule) {
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