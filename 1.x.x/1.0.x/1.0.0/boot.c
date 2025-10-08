// boot.c
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   // for rand, srand
#include <time.h>     // for seeding srand

double get_unix_epoch_seconds() {
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

void start_service(const char* service_name, int delay_ms, int success) {
    double t = get_unix_epoch_seconds();
    printf("[\033[2m%.6f\033[0m] Starting %s... [\t\033[33mPENDING\033[0m\t]\n", t, service_name);
    fflush(stdout);

    // Add randomized delay
    Sleep(delay_ms + (rand() % 100));

    if (success) {
        printf("[\033[2m%.6f\033[0m] Starting %s... [\t\033[32mOK\033[0m\t]\n",
               t + (delay_ms / 1000.0), service_name);
    } else {
        printf("[\033[2m%.6f\033[0m] Starting %s... [\t\033[31mFAILED\033[0m\t]\n",
               t + (delay_ms / 1000.0), service_name);
    }
}

void boot_sequence() {
    // Seed RNG once per boot
    srand((unsigned int)time(NULL));

    double t = get_unix_epoch_seconds();

    // Initial boot message
    printf("\n");
    printf("Welcome To WNU OS 1.0.0 (Windows Bash Shell)!\n");

    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.0 booting... [\t\033[33mPENDING\033[0m\t]\n", t);
    fflush(stdout);
    Sleep(300 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.0 booting... [\t\033[32mOK\033[0m\t]\n", t + 0.3);

    printf("[\033[2m%.6f\033[0m] Initializing kernel... [\t\033[33mPENDING\033[0m\t]\n", t + 0.4);
    fflush(stdout);
    Sleep(250 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Initializing kernel... [\t\033[32mOK\033[0m\t]\n", t + 0.65);

    printf("[\033[2m%.6f\033[0m] Loading hardware drivers... [\t\033[33mPENDING\033[0m\t]\n", t + 0.7);
    fflush(stdout);
    Sleep(400 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Loading hardware drivers... [\t\033[32mOK\033[0m\t]\n", t + 1.1);

    printf("[\033[2m%.6f\033[0m] Mounting root filesystem at / (C:\\)... [\t\033[33mPENDING\033[0m\t]\n", t + 1.2);
    fflush(stdout);
    Sleep(200 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Mounting root filesystem at / (C:\\)... [\t\033[32mOK\033[0m\t]\n", t + 1.4);

    // Start system services with randomized delays
    start_service("udev", 300, 1);
    Sleep(200 + (rand() % 100));
    start_service("systemd-journald", 150, 1);
    Sleep(200 + (rand() % 100));
    start_service("systemd-networkd", 200, 1);
    Sleep(200 + (rand() % 100));
    start_service("dbus", 180, 1);
    start_service("NetworkManager", 250, 1);
    Sleep(200 + (rand() % 100));
    start_service("ssh", 120, 1);
    Sleep(200 + (rand() % 100));
    start_service("cron", 100, 1);
    Sleep(200 + (rand() % 100));
    start_service("rsyslog", 140, 1);
    Sleep(200 + (rand() % 100));
    start_service("getty@tty1", 160, 1);
    Sleep(200 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Reaching target Multi-User System. [\t\033[33mPENDING\033[0m\t]\n", get_unix_epoch_seconds());
    Sleep(200 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Reached target Multi-User System. [\t\033[32mOK\033[0m\t]\n", get_unix_epoch_seconds());

    printf("\n");
    printf("Unix Epoch Time since Boot Success: %.6f seconds\n", get_unix_epoch_seconds() - t);
    printf("WNU OS 1.0.0 boot completed successfully.\n");
    printf("System ready for login.\n");
    printf("\n");
    Sleep(500 + (rand() % 100));
    // system("cls");
}
