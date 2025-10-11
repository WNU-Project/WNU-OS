#ifndef HALT_H
#define HALT_H

#include <stdint.h>

// Function declarations
uint64_t get_unix_epoch_seconds_halt();
int check_ctrl_s_h_interrupt();
void halt_stop_service(const char* service_name);
int halt_sequence();

// Windows constant for ANSI color support (if not defined)
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#endif // HALT_H
