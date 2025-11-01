// WUB - WNU Universal Bootloader
// INSPIRED BY GNU GRUB
#include <stdio.h>
#include <windows.h>
#include <conio.h>

int wub() {
    int selection = 0;
    char key;
    
    // Clear screen and set background color (simulating GRUB's blue background)
    system("cls");
    
    while (1) {
        // Clear screen for menu refresh
        system("cls");
        
        // GRUB-style header
        printf("\n");
        printf("                           WNU WUB  version 1.0\n");
        printf("\n");
        printf(" +----------------------------------------------------------------------------+\n");
        printf(" |WNU OS 1.0.1 Update 2                                                       |\n");
        printf(" |Advanced options for WNU OS 1.0.1 Update 2                                  |\n");
        printf(" |WNU OS 1.0.1 Update 2 (recovery mode)                                       |\n");
        printf(" |Exit                                                                        |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" |                                                                            |\n");
        printf(" +----------------------------------------------------------------------------+\n");
        
        // Show selection highlight
        if (selection == 0) {
            printf(" |*WNU OS 1.0.1 Update 2                                                     |\n");
        } else if (selection == 1) {
            printf(" |*Advanced options for WNU OS 1.0.1 Update 2                                |\n");
        } else if (selection == 2) {
            printf(" |*WNU OS 1.0.1 Update 2 (recovery mode)                                     |\n");
        } else if (selection == 3) {
            printf(" |*Exit                                                                      |\n");
        } else if (selection > 3) {
                selection = 0; // Safety check
        }
        
        printf("\n");
        printf("      Use the ^ and v keys to select which entry is highlighted.\n");
        printf("      Press enter to boot the selected OS, `e' to edit the\n");
        printf("      commands before booting or `c' for a command-line.\n");
        
        // Get key input
        key = _getch();
        
        if (key == 72) { // Up arrow
            selection = (selection - 1 + 4) % 4;
        } else if (key == 80) { // Down arrow  
            selection = (selection + 1) % 4;
        } else if (key == 13) { // Enter key
            if (selection == 0) {
                printf("\n");
                printf("Booting WNU OS 1.0.1 Update 2...\n");
                Sleep(2000);
                system("cls");
                return 0; // Normal boot
            } else if (selection == 1) {
                printf("\n");
                printf("Booting WNU OS 1.0.1 Update 2 (Advanced)...\n");
                Sleep(2000);
                system("cls");
                return 0; // Advanced boot (same as normal for now)
            } else if (selection == 2) {
                printf("\n");
                printf("Booting WNU OS 1.0.1 Update 2 (Recovery Mode)...\n");
                printf("Loading Linux 5.15.0-wnu-os...\n");
                printf("Loading initial ramdisk...\n");
                printf("Starting recovery mode...\n");
                Sleep(2000);
                system("cls");
                return 0; // Recovery boot
            } else if (selection == 3) {
                printf("\n");
                printf("Exiting WUB...\n");
                Sleep(1000);
                return 1; // Exit
            } else if (selection > 3) {
                selection = 0; // Safety check
            }
        } else if (key == 'c' || key == 'C') {
            printf("\n");
            printf("Minimal BASH-like line editing is supported. For the first word, TAB\n");
            printf("lists possible command completions. Anywhere else TAB lists possible\n");
            printf("device or file completions.\n");
            printf("\n");
            printf("grub> ");
            // Simple command line simulation
            char cmd[100];
            fgets(cmd, sizeof(cmd), stdin);
            printf("Unknown command '%s'\n", cmd);
            printf("Press any key to return to menu...\n");
            _getch();
        } else if (key == 27) { // ESC key
            printf("\n");
            printf("Exiting GRUB...\n");
            Sleep(1000);
            return 1; // Exit
        }
    }
}