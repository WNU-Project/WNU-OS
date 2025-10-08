// boot.c
#include <windows.h>
#include <stdint.h>
#include <stdio.h>

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
    printf("[%.6f] Starting %s... [PENDING]\n", t, service_name);
    fflush(stdout);
    Sleep(delay_ms);
    
    if (success) {
        printf("[%.6f] Starting %s... [OK]\n", t + (delay_ms / 1000.0), service_name);
    } else {
        printf("[%.6f] Starting %s... [FAILED]\n", t + (delay_ms / 1000.0), service_name);
    }
}

void boot_sequence() {
    double t = get_unix_epoch_seconds();
    
    // Initial boot message
    printf("\n");
    printf("WNU OS 1.0.0 (GNU/Linux compatible)\n");
    printf("Copyright (C) 2025 WNU Project\n");
    printf("\n");
    
    printf("[%.6f] WNU OS 1.0.0 booting... [PENDING]\n", t);
    fflush(stdout);
    Sleep(300);
    printf("[%.6f] WNU OS 1.0.0 booting... [OK]\n", t + 0.3);
    
    printf("[%.6f] Initializing kernel... [PENDING]\n", t + 0.4);
    fflush(stdout);
    Sleep(250);
    printf("[%.6f] Initializing kernel... [OK]\n", t + 0.65);
    
    printf("[%.6f] Loading hardware drivers... [PENDING]\n", t + 0.7);
    fflush(stdout);
    Sleep(400);
    printf("[%.6f] Loading hardware drivers... [OK]\n", t + 1.1);
    
    printf("[%.6f] Mounting root filesystem at / (C:\\)... [PENDING]\n", t + 1.2);
    fflush(stdout);
    Sleep(200);
    printf("[%.6f] Mounting root filesystem at / (C:\\)... [OK]\n", t + 1.4);
    
    // Start system services
    start_service("udev", 300, 1);
    start_service("systemd-journald", 150, 1);
    start_service("systemd-networkd", 200, 1);
    start_service("dbus", 180, 1);
    start_service("NetworkManager", 250, 1);
    start_service("ssh", 120, 1);
    start_service("cron", 100, 1);
    start_service("rsyslog", 140, 1);
    start_service("getty@tty1", 160, 1);
    
    printf("\n");
    printf("WNU OS 1.0.0 boot completed successfully.\n");
    printf("System ready for login.\n");
    printf("\n");
    Sleep(500);
    system("cls");
}
