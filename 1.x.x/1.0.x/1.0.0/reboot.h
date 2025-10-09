#ifndef REBOOT_H
#define REBOOT_H

#include <stdint.h>

// Function declarations
uint64_t get_unix_epoch_seconds_reboot();
int check_reboot_interrupt();
void reboot_stop_service(const char* service_name);
void reboot_start_service(const char* service_name);
int reboot_sequence();

// Windows constant for ANSI color support (if not defined)
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#endif // REBOOT_H
