void kernel_main() {
    // VGA text buffer is at 0xB8000
    // Each character takes 2 bytes: [character][attribute]
    // Attribute: 0x0F = white text on black background
    
    char* vga_buffer = (char*)0xB8000;
    const char* message = "Hello, World!";
    
    // Clear screen first (fill with spaces)
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga_buffer[i] = ' ';      // Character
        vga_buffer[i + 1] = 0x0F; // Attribute (white on black)
    }
    
    // Write our message
    for (int i = 0; message[i] != '\0'; i++) {
        vga_buffer[i * 2] = message[i];     // Character
        vga_buffer[i * 2 + 1] = 0x0E;       // Attribute (yellow on black)
    }
    
    // Infinite loop to prevent the kernel from exiting
    while (1) {
        asm("hlt"); // Halt the CPU until next interrupt
    }
}