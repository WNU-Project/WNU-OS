// WNU OS BIOS BOOTLOADER
#include <stdio.h>
#include <windows.h>
#include <conio.h>

#define RESET     "\033[0m"
#define HIGHLIGHT "\033[7m"   // reverse video

int bios() {
    int selection = 0;
    char key;

    const char* entriesbiosboot[] = {
        "WNU WUB 1.0.0",
        "Exit"
    };

    while (1) {
        system("cls");

        // Header
        printf("\n");
        printf("                           WNU BIOS Bootloader\n\n");
        printf(" +----------------------------------------------------------------------------+\n");
        printf(" | BIOS BOOT:                                                                 |\n");
        // Print menu entries with highlight
        for (int i = 0; i < 2; i++) {
            if (i == selection)
                printf(" |%s%-76s%s|\n", HIGHLIGHT, entriesbiosboot[i], RESET);
            else
                printf(" |%-76s|\n", entriesbiosboot[i]);
        }

        // Fill remaining lines
        for (int i = 2; i < 10; i++)
            printf(" |                                                                            |\n");

        printf(" +----------------------------------------------------------------------------+\n");
        printf(" |*%-75s|\n", entriesbiosboot[selection]);
        printf("\n");
        printf("      Press enter to continue booting.\n");

        // Input
        key = _getch();

        if (key == 72) { // Up arrow
            selection = (selection - 1 + 2) % 2;
        } else if (key == 80) { // Down arrow  
            selection = (selection + 1) % 2;
        } else if (key == 13) { // Enter
            if (selection == 0) { // WNU WUB 1.0.0
                system("cls");
                printf("Continuing boot sequence...\n");
                Sleep(2000);
                return 0;
            } else if (selection == 1) { // Exit
                printf("Exiting BIOS Bootloader...\n");
                Sleep(1000);
                return 1;
            }
        } else if (key == 27) { // ESC key
            printf("Exiting BIOS Bootloader...\n");
            Sleep(1000);
            return 1;
        }
    }
}