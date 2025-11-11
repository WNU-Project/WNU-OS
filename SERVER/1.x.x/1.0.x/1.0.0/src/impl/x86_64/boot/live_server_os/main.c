/* WNU OS SERVER 1.0.0 Ultra-Simple Shell to prevent crashes */

void installer_main(void) {
    volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    int pos = 0;
    
    // Clear screen safely
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0x0720; // Space with black background
    }
    
    // Show simple prompt: "root@install:~# "
    vga[0] = 0x0A72;   // 'r' green
    vga[1] = 0x0A6F;   // 'o' green  
    vga[2] = 0x0A6F;   // 'o' green
    vga[3] = 0x0A74;   // 't' green
    vga[4] = 0x0A40;   // '@' green
    vga[5] = 0x0A69;   // 'i' green
    vga[6] = 0x0A6E;   // 'n' green
    vga[7] = 0x0A73;   // 's' green
    vga[8] = 0x0A74;   // 't' green
    vga[9] = 0x0A61;   // 'a' green
    vga[10] = 0x0A6C;  // 'l' green
    vga[11] = 0x0A6C;  // 'l' green
    vga[12] = 0x0A3A;  // ':' green
    vga[13] = 0x0A7E;  // '~' green
    vga[14] = 0x0A23;  // '#' green
    vga[15] = 0x0A20;  // ' ' green
    
    // Simple infinite loop - no keyboard input for now to avoid crashes
    while (1) {
        // Just halt safely
        __asm__ volatile ("hlt");
    }
}