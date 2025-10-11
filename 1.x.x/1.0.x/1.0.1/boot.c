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
    
    // Binary representation of "START WNU OS 1.0.0 systemd BOOTUP (KERNEL: WNU Kernel 1.0.1)"
    char* binary = "01010011 01010100 01000001 01010010 01010100 00100000 01010111 01001110 01010101 00100000 01001111 01010011 00100000 00110001 00101110 00110000 00101110 00110000 00100000 01110011 01111001 01110011 01110100 01100101 01101101 01100100 00100000 01000010 01001111 01001111 01010100 01010101 01010000 00100000 00101000 01001011 01000101 01010010 01001110 01000101 01001100 00111010 00100000 01010111 01001110 01010101 00100000 01001011 01100101 01110010 01101110 01100101 01101100 00100000 00110001 00101110 00110000 00101110 00110001 00101001";
    // Binary representation of "STARTING WNU KERNEL 1.0.1's systemd BOOTUP SERVICE.."
    char* binary2 = "01010011 01010100 01000001 01010010 01010100 01001001 01001110 01000111 00100000 01010111 01001110 01010101 00100000 01001011 01000101 01010010 01001110 01000101 01001100 00100000 00110001 00101110 00110000 00101110 00110001 00100111 01110011 00100000 01110011 01111001 01110011 01110100 01100101 01101101 01100100 00100000 01000010 01001111 01001111 01010100 01010101 01010000 00100000 01010011 01000101 01010010 01010110 01001001 01000011 01000101 00101110 00101110";
    // Binary representation of "RUNNING BIOS COMMAND: sudo systemctl start default.target"
    char* binary3 = "01010010 01010101 01001110 01001110 01001001 01001110 01000111 00100000 01000010 01001001 01001111 01010011 00100000 01000011 01001111 01001101 01001101 01000001 01001110 01000100 00111010 00100000 01110011 01110101 01100100 01101111 00100000 01110011 01111001 01110011 01110100 01100101 01101101 01100011 01110100 01101100 00100000 01110011 01110100 01100001 01110010 01110100 00100000 01100100 01100101 01100110 01100001 01110101 01101100 01110100 00101110 01110100 01100001 01110010 01100111 01100101 01110100";
    // Binary representation of "COMMAND HAS STARTED TURNING FROM BINARY TO HUMAN READABLE TEXT"
    char* binary4 = "01000011 01001111 01001101 01001101 01000001 01001110 01000100 00100000 01001000 01000001 01010011 00100000 01010011 01010100 01000001 01010010 01010100 01000101 01000100 00100000 01010100 01010101 01010010 01001110 01001001 01001110 01000111 00100000 01000110 01010010 01001111 01001101 00100000 01000010 01001001 01001110 01000001 01010010 01011001 00100000 01010100 01001111 00100000 01001000 01010101 01001101 01000001 01001110 00100000 01010010 01000101 01000001 01000100 01000001 01000010 01001100 01000101 00100000 01010100 01000101 01011000 01010100";
    // Not Really Binary representation but close enough: "BINARY HAS BEEN SET TO HUMAN READABLE TEXT SHOWING LOG OF: sudo systemctl start default.target"
    char* binary5 = "BINARY HAS BEEN SET TO HUMAN READABLE TEXT SHOWING LOG OF: sudo systemctl start default.target";

    // Print each bit individually without newlines
    for (int i = 0; binary[i] != '\0'; i++) {
        printf("%c", binary[i]);
        fflush(stdout);
    }
    printf("\n\n\n");
    Sleep(10); // Pause before next binary output
    for (int i = 0; binary2[i] != '\0'; i++) {
        printf("%c", binary2[i]);
        fflush(stdout);
    }
    printf("\n\n\n");
    Sleep(10); // Pause before next binary output
    for (int i = 0; binary3[i] != '\0'; i++)
    {
        printf("%c", binary3[i]);
        fflush(stdout);
    }
    printf("\n\n\n");
    Sleep(10); // Pause before next binary output
    for (int i = 0; binary4[i] != '\0'; i++)
    {
        printf("%c", binary4[i]);
        fflush(stdout);
    }
    printf("\n\n\n");
    Sleep(10); // Pause before next binary output
    for (int i = 0; binary5[i] != '\0'; i++)
    {
        printf("%c", binary5[i]);
        fflush(stdout);
    }
    Sleep(500 + (rand() % 100));
    system("cls");
    
    // Initial boot message
    printf("\n");
    printf("Welcome To WNU OS 1.0.1 (Windows Bash Shell)!\n");

    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.1 booting... [\t\033[33mPENDING\033[0m\t]\n", t);
    fflush(stdout);
    Sleep(300 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] WNU OS 1.0.1 booting... [\t\033[32mOK\033[0m\t]\n", t + 0.3);

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
    start_service("udev", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("systemd-journald", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("systemd-networkd", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("dbus", 120, rand() % 10 != 0);
    start_service("NetworkManager", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("ssh", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("cron", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("rsyslog", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("getty@tty1", 120, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    start_service("wsys2", 150, rand() % 10 != 0);
    Sleep(200 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Reaching target Multi-User System. [\t\033[33mPENDING\033[0m\t]\n", get_unix_epoch_seconds());
    Sleep(200 + (rand() % 100));
    printf("[\033[2m%.6f\033[0m] Reached target Multi-User System. [\t\033[32mOK\033[0m\t]\n", get_unix_epoch_seconds());

    printf("\n");
    printf("Unix Epoch Time since Boot Success: %.6f seconds\n", get_unix_epoch_seconds() - t);
    printf("WNU OS 1.0.1 boot completed successfully.\n");
    printf("System ready for login.\n");
    printf("\n");
    Sleep(500 + (rand() % 100));
    system("cls");
}
