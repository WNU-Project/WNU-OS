#ifndef BOOT_H
#define BOOT_H

void boot_sequence(void);
double get_unix_epoch_seconds(void);
extern char* target;

#endif // BOOT_H