#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // for rand, srand
#include <time.h>     // for seeding srand
#include "poweroff.h"

double get_poweroff_unix_epoch_seconds(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    // Convert FILETIME to 64-bit value
    ULONGLONG time = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;

    // FILETIME is in 100-nanosecond intervals since Jan 1, 1601
    // UNIX epoch starts Jan 1, 1970 — offset is 11644473600 seconds
    const ULONGLONG EPOCH_DIFF = 116444736000000000ULL;

    double unix_time = (time - EPOCH_DIFF) / 10000000.0;
    return unix_time;
}

void stop_service(const char* service_name, int delay_ms, int success) {
    double t = get_poweroff_unix_epoch_seconds();
    printf("[\033[2m%.6f\033[0m] Stopping %s... [\t\033[33mPENDING\033[0m\t]\n", t, service_name);
    fflush(stdout);

    // Add randomized delay
    Sleep(delay_ms + (rand() % 50)); // Shorter random delays for shutdown

    if (success) {
        printf("[\033[2m%.6f\033[0m] Stopping %s... [\t\033[32mOK\033[0m\t]\n",
               t + (delay_ms / 1000.0), service_name);
    } else {
        printf("[\033[2m%.6f\033[0m] Stopping %s... [\t\033[31mFAILED\033[0m\t]\n",
               t + (delay_ms / 1000.0), service_name);
    }
}

int poweroff_sequence(void) {
    // Seed RNG for shutdown sequence
    srand((unsigned int)time(NULL));

    double t = get_poweroff_unix_epoch_seconds();

    char* loadingbar = "####################################################################################################";
    printf("\033[36mStarting systemd-poweroff.service... 0%%\033[0m");
    
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

    printf("\n");
    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.1 shutdown initiated... [\t\033[33mPENDING\033[0m\t]\n", t);
    fflush(stdout);
    Sleep(200 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.1 shutdown initiated... [\t\033[32mOK\033[0m\t]\n", t + 0.2);

    printf("[\033[2m%.6f\033[0m] Stopping user sessions... [\t\033[33mPENDING\033[0m\t]\n", get_poweroff_unix_epoch_seconds());
    Sleep(150 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] Stopping user sessions... [\t\033[32mOK\033[0m\t]\n", get_poweroff_unix_epoch_seconds());

    // Stop services in reverse order of boot
    stop_service("getty@tty1", 100, 1);
    Sleep(100 + (rand() % 50));
    stop_service("rsyslog", 120, 1);
    Sleep(100 + (rand() % 50));
    stop_service("cron", 80, 1);
    Sleep(100 + (rand() % 50));
    stop_service("ssh", 100, 1);
    Sleep(100 + (rand() % 50));
    stop_service("NetworkManager", 200, 1);
    Sleep(100 + (rand() % 50));
    stop_service("dbus", 150, 1);
    stop_service("systemd-networkd", 180, 1);
    Sleep(100 + (rand() % 50));
    stop_service("systemd-journald", 130, 1);
    Sleep(100 + (rand() % 50));
    stop_service("udev", 250, 1);
    Sleep(100 + (rand() % 50));
    stop_service("wsys2", 200, 1);

    printf("[\033[2m%.6f\033[0m] Reached target shutdown. [\t\033[33mPENDING\033[0m\t]\n", get_poweroff_unix_epoch_seconds());
    Sleep(150 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] Reached target shutdown. [\t\033[32mOK\033[0m\t]\n", get_poweroff_unix_epoch_seconds());

    printf("[\033[2m%.6f\033[0m] Unmounting filesystems... [\t\033[33mPENDING\033[0m\t]\n", get_poweroff_unix_epoch_seconds());
    Sleep(200 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] Unmounting filesystems... [\t\033[32mOK\033[0m\t]\n", get_poweroff_unix_epoch_seconds());

    printf("[\033[2m%.6f\033[0m] Unloading hardware drivers... [\t\033[33mPENDING\033[0m\t]\n", get_poweroff_unix_epoch_seconds());
    Sleep(300 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] Unloading hardware drivers... [\t\033[32mOK\033[0m\t]\n", get_poweroff_unix_epoch_seconds());

    printf("[\033[2m%.6f\033[0m] Powering off system... [\t\033[33mPENDING\033[0m\t]\n", get_poweroff_unix_epoch_seconds());
    Sleep(400 + (rand() % 50));
    printf("[\033[2m%.6f\033[0m] Powering off system... [\t\033[32mOK\033[0m\t]\n", get_poweroff_unix_epoch_seconds());

    printf("\n");
    printf("Unix Epoch Time for shutdown sequence: %.6f seconds\n", get_poweroff_unix_epoch_seconds() - t);
    printf("WNU OS 1.0.1 shutdown completed successfully.\n");
    printf("System halted. Thank you for using WNU OS!\n");
    printf("\n");

    // Final pause before exit
    Sleep(1000);
    
    // Clear screen and return 0 to indicate successful shutdown
    system("cls");
    return 0;  // Return 0 for successful shutdown (will end the shell)
}