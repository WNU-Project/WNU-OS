#include <stdio.h>
#include <windows.h>
#include <conio.h>

int WNU_VPC() {
    system("cls");
    printf("Loading Virtual PC Environment...\n");
    Sleep(1000);
    printf("Please wait wile The WNU VPC Boots Itself........\n");
    Sleep(1000);
    printf(" /$$      /$$ /$$   /$$ /$$   /$$       /$$    /$$ /$$             /$$                         /$$       /$$$$$$$   /$$$$$$ \n");
    printf("| $$  /$ | $$| $$$ | $$| $$  | $$      | $$   | $$|__/            | $$                        | $$      | $$__  $$ /$$ __  $$\n");
    printf("| $$ /$$$| $$| $$$$| $$| $$  | $$      | $$   | $$ /$$  /$$$$$$  /$$$$$$   /$$   /$$  /$$$$$$ | $$      | $$  \\ $$| $$  \\__/\n");
    printf("| $$/$$ $$ $$| $$ $$ $$| $$  | $$      |  $$ / $$/| $$ /$$__  $$|_  $$_/  | $$  | $$ |____  $$| $$      | $$$$$$$/| $$      \n");
    printf("| $$$$_  $$$$| $$  $$$$| $$  | $$       \\  $$ $$/ | $$| $$  \\__/  | $$    | $$  | $$  /$$$$$$$| $$      | $$____/ | $$      \n");
    printf("| $$$/ \\  $$$| $$\\  $$$| $$  | $$        \\  $$$/  | $$| $$        | $$ /$$| $$  | $$ /$$__  $$| $$      | $$      | $$    $$\n");
    printf("| $$/   \\  $$| $$ \\  $$|  $$$$$$/         \\  $/   | $$| $$        |  $$$$/|  $$$$$$/|  $$$$$$$| $$      | $$      |  $$$$$$/\n");
    printf("|__/     \\__/|__/  \\__/ \\______/           \\_/    |__/|__/         \\___/   \\______/  \\_______/|__/      |__/       \\______/  \n");
    printf("                                                                                                                            \n");
    printf("                                                                                                                            \n");
    printf("                                                                                                                            \n");
    printf("\n");
    printf("\n");
    for (int i = 0; i <= 100; i += 1) {
        printf("\r                                                      Progress: %d%%                                                 ", i);
        fflush(stdout);
        Sleep(100);
        
        // Check if ESC key is pressed
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) { // ESC key
                printf("\n\n");
                printf("THIS WILL BE THE BIOS SETTINGS\n");
                printf("\n");
                return 1;
            }
        }
    }
    
    return 0;
}