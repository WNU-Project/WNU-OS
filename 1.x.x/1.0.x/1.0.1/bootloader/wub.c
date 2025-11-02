// WUB - WNU Universal Bootloader
// INSPIRED BY GNU GRUB
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "bios/bios.h"

#define RESET     "\033[0m"
#define HIGHLIGHT "\033[7m"   // reverse video

int wub() {

    int selection = 0;
    char key;

    const char* entries[] = {
        "WNU OS 1.0.1 Update 2",
        "Advanced options for WNU OS 1.0.1 Update 2",
        "WNU OS 1.0.1 Update 2 (recovery mode)",
        "Exit"
    };
    int total = sizeof(entries) / sizeof(entries[0]);
    if (bios() == 0) {

    } else {
        exit(1);
    }

    while (1) {
        system("cls");

        // Header
        printf("\n");
        printf("                           WNU WUB  version 1.0\n\n");
        printf(" +----------------------------------------------------------------------------+\n");

        // Print menu entries with highlight
        for (int i = 0; i < total; i++) {
            if (i == selection)
                printf(" |%s%-76s%s|\n", HIGHLIGHT, entries[i], RESET);
            else
                printf(" |%-76s|\n", entries[i]);
        }

        // Fill remaining lines
        for (int i = total; i < 10; i++)
            printf(" |                                                                            |\n");

        printf(" +----------------------------------------------------------------------------+\n");
        printf(" |*%-75s|\n", entries[selection]);
        printf("\n");
        printf("      Use the ^ and v keys to select which entry is highlighted.\n");
        printf("      Press enter to boot the selected OS, `e' to edit the\n");
        printf("      commands before booting or `c' for a command-line.\n");

        // Input
        key = _getch();

        if (key == 72) { // Up arrow
            selection = (selection - 1 + total) % total;
        } else if (key == 80) { // Down arrow
            selection = (selection + 1) % total;
        } else if (key == 13) { // Enter
            system("cls");
            printf("Booting: %s...\n", entries[selection]);
            Sleep(2000);
            return 0;
            if (entries[selection] == "Exit") {
                printf("Exiting WUB...\n");
                Sleep(1000);
                exit(1);
            }
        } else if (key == 'c' || key == 'C') {
            printf("\nMinimal BASH-like line editing is supported...\n");
            printf("grub> ");
            char cmd[100];
            fgets(cmd, sizeof(cmd), stdin);
            printf("Unknown command '%s'\n", cmd);
            printf("Press any key to return to menu...\n");
            _getch();
        } else if (key == 27) { // ESC
            printf("\nExiting WUB...\n");
            Sleep(1000);
            return 1;
        }
    }
}
