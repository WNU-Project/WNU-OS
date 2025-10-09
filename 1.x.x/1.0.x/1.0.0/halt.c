#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // for rand, srand
#include <time.h>     // for seeding srand
#include <string.h>   // for strlen
#include <conio.h>    // for _kbhit and _getch
#include "halt.h"

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

// Get current Unix epoch seconds
uint64_t get_unix_epoch_seconds_halt() {
    FILETIME ft;
    uint64_t time_since_epoch;
    
    GetSystemTimeAsFileTime(&ft);
    
    // Convert FILETIME to 64-bit integer
    time_since_epoch = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    
    // Convert from 100-nanosecond intervals since Jan 1, 1601 to seconds since Jan 1, 1970
    time_since_epoch = (time_since_epoch - 116444736000000000ULL) / 10000000ULL;
    
    return time_since_epoch;
}

// Function to check for Ctrl+S+H key combination
int check_ctrl_s_h_interrupt() {
    static int waiting_for_s = 0;
    
    if (_kbhit()) {
        int key = _getch();
        
        // Handle different key states
        if (key == 0 || key == 224) {
            // Extended key, get the next character
            if (_kbhit()) {
                _getch(); // Consume the extended key code
            }
            return 0;
        }
        
        // Check for Ctrl+C first (as alternative interrupt)
        if (key == 3) { // Ctrl+C
            printf(RED "\n[CTRL+C DETECTED - INTERRUPT]\n" RESET);
            return 1;
        }
        
        // Method 1: Try to detect Ctrl key state using GetAsyncKeyState
        SHORT ctrl_state = GetAsyncKeyState(VK_CONTROL);
        SHORT s_state = GetAsyncKeyState('S');
        SHORT h_state = GetAsyncKeyState('H');
        
        // Check if Ctrl+S+H are all pressed simultaneously
        if ((ctrl_state & 0x8000) && (s_state & 0x8000) && (h_state & 0x8000)) {
            printf(GREEN "\n[CTRL+S+H DETECTED]\n" RESET);
            return 1;
        }
        
        // Method 2: Sequential detection - Ctrl+S then H
        if (key == 19) { // Ctrl+S detected
            waiting_for_s = 1;
            printf(YELLOW "[Ctrl+S detected, press H...]\n" RESET);
            return 0;
        }
        
        if (waiting_for_s && (key == 'H' || key == 'h')) {
            printf(GREEN "[H detected - RESTART TRIGGERED]\n" RESET);
            waiting_for_s = 0;
            return 1;
        }
        
        // Method 3: Simple 'r' key for restart (backup method)
        if (key == 'r' || key == 'R') {
            printf(CYAN "\n[R key pressed - RESTART]\n" RESET);
            return 1;
        }
        
        // Reset if any other key is pressed
        waiting_for_s = 0;
    }
    return 0; // No interrupt
}

void halt_stop_service(const char* service_name) {
    printf(YELLOW "[  " CYAN "%llu" YELLOW "  ] " RESET "Stopping %s...", 
           get_unix_epoch_seconds_halt(), service_name);
    fflush(stdout);
    
    // Random delay with interrupt checking
    int total_delay = 50 + rand() % 250;
    int check_interval = 50; // Check every 50ms
    
    for (int elapsed = 0; elapsed < total_delay; elapsed += check_interval) {
        Sleep(check_interval);
        
        // Check for Ctrl+S+H interrupt during service stop
        if (check_ctrl_s_h_interrupt()) {
            printf(RED " [INTERRUPTED]\n" RESET);
            return; // Service stop was interrupted
        }
    }
    
    printf(GREEN " [ OK ]\n" RESET);
}

int halt_sequence() {
    char* loadingbar = "####################################################################################################";
    // Enable ANSI color support
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    // Seed random number generator
    srand((unsigned int)time(NULL));
    printf(CYAN "Starting systemd-halt.service... 0%%" RESET);
    
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
    printf("\n" RESET);
    printf(RED "\nWNU OS Halt Sequence Initiated\n" RESET);
    printf(YELLOW "==============================\n" RESET);
    printf(CYAN "Press Ctrl+S+H, Ctrl+C, or R key to interrupt halt and restart shell\n\n" RESET);
    
    // Array of services to stop
    const char* services[] = {
        "NetworkManager",
        "SSH daemon (sshd)",
        "System logging (rsyslog)",
        "Cron daemon (crond)",
        "D-Bus system message bus",
        "User sessions",
        "Mount filesystems", 
        "Swap partitions",
        "Network interfaces",
        "Hardware abstraction layer",
        "Kernel modules"
    };
    
    int num_services = sizeof(services) / sizeof(services[0]);
    
    // Stop system services in halt order with interrupt checking
    for (int i = 0; i < num_services; i++) {
        // Check for interrupt before each service
        if (check_ctrl_s_h_interrupt()) {
            printf(RED "\n[HALT INTERRUPTED]\n" RESET);
            printf(YELLOW "Ctrl+S+H detected! Stopping halt sequence...\n" RESET);
            return -1; // Return -1 to indicate interruption
        }
        
        halt_stop_service(services[i]);
        
        // Additional interrupt check after each service
        if (check_ctrl_s_h_interrupt()) {
            printf(RED "\n[HALT INTERRUPTED]\n" RESET);
            printf(YELLOW "Ctrl+S+H detected! Stopping halt sequence...\n" RESET);
            return -1; // Return -1 to indicate interruption
        }
    }
    
    // Final interrupt check before completion
    if (check_ctrl_s_h_interrupt()) {
        printf(RED "\n[HALT INTERRUPTED]\n" RESET);
        printf(YELLOW "Ctrl+S+H detected! Stopping halt sequence...\n" RESET);
        return -1; // Return -1 to indicate interruption
    }
    
    printf(MAGENTA "\n[ " CYAN "%llu" MAGENTA " ] " RESET "All services stopped successfully\n", 
           get_unix_epoch_seconds_halt());
    
    printf(RED "[ " CYAN "%llu" RED " ] " RESET "System halted. Safe to power off.\n", 
           get_unix_epoch_seconds_halt());
    
    printf(YELLOW "\nSystem is now in halt state.\n" RESET);
    printf(WHITE "Hardware can be safely powered off.\n" RESET);
    printf(CYAN "Press Ctrl+S+H, Ctrl+C, or R key to restart shell...\n" RESET);
    
    // Enter halt state - do nothing until Ctrl+S+H is pressed
    while (1) {
        Sleep(100); // Sleep for 100ms to reduce CPU usage
        
        // Check for Ctrl+S+H to restart shell
        if (check_ctrl_s_h_interrupt()) {
            printf(GREEN "\n[SYSTEM RESTART]\n" RESET);
            printf(YELLOW "Ctrl+S+H detected! Restarting shell from halt state...\n" RESET);
            return -1; // Return -1 to indicate restart requested
        }
        
        // Optional: Show a periodic "still halted" indicator (every 30 seconds)
        static int halt_counter = 0;
        halt_counter++;
        if (halt_counter >= 300) { // 300 * 100ms = 30 seconds
            printf(BLUE "[ " CYAN "%llu" BLUE " ] " RESET "System remains halted. Press Ctrl+S+H, Ctrl+C, or R to restart.\n", 
                   get_unix_epoch_seconds_halt());
            halt_counter = 0;
        }
    }
    
    // This line should never be reached
    return 0;
}

