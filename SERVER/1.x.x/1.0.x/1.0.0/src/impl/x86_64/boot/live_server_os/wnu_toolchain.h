/*
 * WNU OS Cross-Platform Toolchain Manager Header
 */

#ifndef WNU_TOOLCHAIN_H
#define WNU_TOOLCHAIN_H

#include "toolchain_config.h"

// Toolchain management functions
int wnu_detect_toolchain(const char* target_name);
void wnu_list_toolchains(void);
int wnu_cross_compile(int toolchain_id, const char* source_file);
void wnu_configure_target_env(int toolchain_id);
int wnu_interactive_toolchain_select(void);
void wnu_generate_makefile(int toolchain_id);
int wnu_toolchain_main(const char* command);

// Helper functions
int simple_strcmp(const char* s1, const char* s2);
void simple_sprintf(char* dest, const char* format, ...);

#endif /* WNU_TOOLCHAIN_H */