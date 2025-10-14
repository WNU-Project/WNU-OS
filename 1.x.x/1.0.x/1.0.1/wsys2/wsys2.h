#ifndef WSYS2_H
#define WSYS2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <wininet.h>

// WSYS2 Version Information
#define WSYS2_VERSION "1.0.0"
#define WSYS2_PACKAGE_EXT ".wnupkg"

// Directory paths
#define WSYS2_SYSTEM_PACKAGES "C:\\WNU\\packages"
#define WSYS2_USER_PACKAGES "%USERPROFILE%\\.local\\packages"
#define WSYS2_DATABASE_FILE "installed.db"
#define WSYS2_TEMP_DIR "%TEMP%\\wsys2"

// Package structure
typedef struct {
    char name[256];
    char version[64];
    char description[512];
    char author[128];
    char dependencies[1024];
    char install_path[512];
    int installed;
} Package;

// Core WSYS2 functions
int wsys2_init(void);
int wsys2_install(const char* package_file);
int wsys2_remove(const char* package_name);
int wsys2_update(void);
int wsys2_search(const char* search_term);
int wsys2_list(void);
int wsys2_info(const char* package_name);
int wsys2_run(const char* package_spec, const char* program_name, char** args, int arg_count);
int wsys2_upgrade(const char* package_name);

// Online package functions
int wsys2_online_update(void);
int wsys2_online_search(const char* search_term);
int wsys2_online_install(const char* package_name);
int wsys2_online_list(void);
int wsys2_online_info(const char* package_name);

// Online helpers
int check_url_exists(const char* url);
int download_file(const char* url, const char* local_path);

// Package management functions
int package_parse_wnupkg(const char* wnupkg_file, Package* pkg);
int package_install_files(const Package* pkg, const char* source_file, const char* dest_dir);
int package_extract_files(const char* wnupkg_file, const char* dest_dir, const Package* pkg);
int package_add_to_database(const Package* pkg);
int package_remove_from_database(const char* package_name);
Package* package_find_installed(const char* package_name);
int package_list_installed(Package** packages, int* count);

// Utility functions
int wsys2_create_directories(void);
char* wsys2_get_package_dir(int user_install);
int wsys2_is_admin(void);
void wsys2_print_package(const Package* pkg);

// Compare two semantic version strings (declared here to avoid implicit
// declaration warnings when used across compilation units)
// Version compare helper: -1 if a<b, 0 if equal, 1 if a>b
int version_compare(const char* a, const char* b);

// Find an executable recursively inside a package directory. Returns 1 on
// success and fills exe_name/exe_dir/exe_path, 0 if none found.
int wsys2_find_executable_recursive(const char* base_path, char* exe_name, char* exe_dir, char* exe_path);

#endif // WSYS2_H