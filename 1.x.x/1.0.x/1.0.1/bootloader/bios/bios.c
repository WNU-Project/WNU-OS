// WNU OS BIOS BOOTLOADER
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "../WNU-VPC/wnu-vpc.h"

#define RESET     "\033[0m"
#define HIGHLIGHT "\033[7m"   // reverse video

int bios() {
    int selection = 0;
    char key;

    const char* entriesbiosboot[] = {
        "WNU WUB 1.0.0",
        "Exit"
    };
    if (WNU_VPC() == 0) {
        /*Move On*/
    } else {
        exit(1);
    }
    while (1) {
        system("cls");

        // Header
        printf("\n");
        printf("WNU VPC BIOS\n\n");
        // Print menu entries with highlight
        for (int i = 0; i < 2; i++) {
            if (i == selection)
                printf("%s%-76s%s\n", HIGHLIGHT, entriesbiosboot[i], RESET);
            else
                printf("%-76s\n", entriesbiosboot[i]);
        }
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
                printf("\n");
                Sleep(2000);
                return 0;
            } else if (selection == 1) { // Exit
                printf("Exiting WNU-VPC BIOS...\n");
                Sleep(1000);
                return 1;
            }
        } else if (key == 27) { // ESC key
            printf("Exiting WNU-VPC BIOS...\n");
            Sleep(1000);
            return 1;
        }
    }
}