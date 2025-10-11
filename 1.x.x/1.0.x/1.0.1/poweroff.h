#ifndef POWEROFF_H
#define POWEROFF_H

// Function declarations for poweroff sequence
double get_poweroff_unix_epoch_seconds(void);
void stop_service(const char* service_name, int delay_ms, int success);
int poweroff_sequence(void);  // Changed to return int (0 = success, 1 = failure)

#endif // POWEROFF_H