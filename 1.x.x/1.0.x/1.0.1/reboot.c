#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // for rand, srand
#include <time.h>     // for seeding srand
#include <string.h>   // for strlen
#include <conio.h>    // for _kbhit and _getch
#include "reboot.h"
#include "boot.h"
#include "poweroff.h"

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// Get current Unix epoch seconds
uint64_t get_unix_epoch_seconds_reboot() {
    FILETIME ft;
    uint64_t time_since_epoch;
    
    GetSystemTimeAsFileTime(&ft);
    
    // Convert FILETIME to 64-bit integer
    time_since_epoch = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    
    // Convert from 100-nanosecond intervals since Jan 1, 1601 to seconds since Jan 1, 1970
    time_since_epoch = (time_since_epoch - 116444736000000000ULL) / 10000000ULL;
    
    return time_since_epoch;
}

// Function to check for reboot interrupt (Ctrl+C or R key)
int check_reboot_interrupt() {
    if (_kbhit()) {
        int key = _getch();
        
        // Handle extended keys
        if (key == 0 || key == 224) {
            if (_kbhit()) {
                _getch(); // Consume the extended key code
            }
            return 0;
        }
        
        // Check for Ctrl+C (interrupt reboot)
        if (key == 3) { // Ctrl+C
            printf(RED "\n[CTRL+C DETECTED - REBOOT INTERRUPTED]\n" RESET);
            return 1;
        }
        
        // Check for 'i' key (interrupt)
        if (key == 'i' || key == 'I') {
            printf(CYAN "\n[I key pressed - REBOOT INTERRUPTED]\n" RESET);
            return 1;
        }
        
        // Check for ESC key
        if (key == 27) { // ESC
            printf(YELLOW "\n[ESC pressed - REBOOT INTERRUPTED]\n" RESET);
            return 1;
        }
    }
    return 0; // No interrupt
}

void reboot_stop_service(const char* service_name) {
    printf(YELLOW "[  " CYAN "%llu" YELLOW "  ] " RESET "Stopping %s...", 
           get_unix_epoch_seconds_reboot(), service_name);
    fflush(stdout);
    
    // Random delay with interrupt checking
    int total_delay = 50 + rand() % 200;
    int check_interval = 25; // Check every 25ms for faster response
    
    for (int elapsed = 0; elapsed < total_delay; elapsed += check_interval) {
        Sleep(check_interval);
        
        // Check for interrupt during service stop
        if (check_reboot_interrupt()) {
            printf(RED " [INTERRUPTED]\n" RESET);
            return; // Service stop was interrupted
        }
    }
    
    printf(GREEN " [ OK ]\n" RESET);
}

void reboot_start_service(const char* service_name) {
    printf(BLUE "[  " CYAN "%llu" BLUE "  ] " RESET "Starting %s...", 
           get_unix_epoch_seconds_reboot(), service_name);
    fflush(stdout);
    
    // Random delay with interrupt checking
    int total_delay = 100 + rand() % 300;
    int check_interval = 25; // Check every 25ms for faster response
    
    for (int elapsed = 0; elapsed < total_delay; elapsed += check_interval) {
        Sleep(check_interval);
        
        // Check for interrupt during service start
        if (check_reboot_interrupt()) {
            printf(RED " [INTERRUPTED]\n" RESET);
            return; // Service start was interrupted
        }
    }
    
    printf(GREEN " [ OK ]\n" RESET);
}

int reboot_sequence() {
    // Enable ANSI color support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    time_t start = time(NULL);
    int duration = 5; // seconds
    
    // Seed random number generator
    srand((unsigned int)time(NULL));

    char* loadingbar = "####################################################################################################";
    printf(CYAN "Starting systemd-reboot.service... 0%%" RESET);
    
    int total_length = strlen(loadingbar);
    
    for (int i = 0; loadingbar[i] != '\0'; i++) {
        // Calculate current percentage based on characters printed
        int percentage = ((i + 1) * 100) / total_length;
        
        // Move cursor back to overwrite the percentage
        printf("\b\b\b\b"); // Move back 4 characters to overwrite "XX%"
        
        // Print the current character
        printf("%c", loadingbar[i]);
        
        // Print updated percentage
        printf(" %d%%", percentage);
        
        fflush(stdout);
        Sleep(10); // Simulate loading delay
    }
    
    // Final newline after 100% completion
    
    printf(BOLD RED "\nWNU OS Reboot Sequence Initiated\n" RESET);
    printf(YELLOW "=================================\n" RESET);
    printf(CYAN "Press Ctrl+C, ESC, or I key to interrupt reboot\n\n" RESET);
    
    // === SHUTDOWN PHASE ===
    printf(RED "Phase 1: Shutdown Services\n" RESET);
    printf(RED "==========================\n" RESET);
    
    poweroff_sequence(); // Call poweroff sequence to simulate shutdown process
    system("cls");
    while (time(NULL) - start < duration) {
      printf("");
    }
    
    // Check for interrupt during restart pause
    if (check_reboot_interrupt()) {
        printf(RED "\n[REBOOT INTERRUPTED DURING RESTART]\n" RESET);
        printf(YELLOW "Reboot sequence cancelled by user!\n" RESET);
        return -1;
    }
    
    // === STARTUP PHASE ===
    printf(GREEN "\nPhase 2: Starting Services\n" RESET);
    printf(GREEN "==========================\n" RESET);
    
    boot_sequence(); // Call boot sequence to simulate boot process
    system("cls");
    return 0; // Return 0 for successful reboot completion
}

