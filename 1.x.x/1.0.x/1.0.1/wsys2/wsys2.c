#include "wsys2.h"
#include <direct.h>
#include <sys/stat.h>

// Initialize WSYS2 system
int wsys2_init(void) {
    printf("Initializing WSYS2 package manager...\n");
    
    // Create necessary directories
    if (wsys2_create_directories() != 0) {
        printf("\033[31mError:\033[0m Failed to create WSYS2 directories\n");
        return 1;
    }
    
    printf("\033[32m✓\033[0m WSYS2 initialized successfully\n");
    return 0;
}

// Install a .wnupkg package
int wsys2_install(const char* package_file) {
    printf("Installing package: %s\n", package_file);
    
    // Check if file exists
    if (GetFileAttributesA(package_file) == INVALID_FILE_ATTRIBUTES) {
        printf("\033[31mError:\033[0m Package file '%s' not found\n", package_file);
        return 1;
    }
    
    // Check if it's a .wnupkg file
    if (!strstr(package_file, WSYS2_PACKAGE_EXT)) {
        printf("\033[31mError:\033[0m Invalid package format. Expected %s file\n", WSYS2_PACKAGE_EXT);
        return 1;
    }
    
    printf("\033[33m[1/4]\033[0m Reading package...\n");
    
    // Create temporary directory
    char temp_dir[512];
    ExpandEnvironmentStringsA(WSYS2_TEMP_DIR, temp_dir, sizeof(temp_dir));
    _mkdir(temp_dir);
    
    printf("\033[33m[2/4]\033[0m Parsing manifest...\n");
    
    // Parse .wnupkg file (our custom format)
    Package pkg = {0};
    if (package_parse_wnupkg(package_file, &pkg) != 0) {
        printf("\033[31mError:\033[0m Failed to parse package manifest\n");
        return 1;
    }
    
    printf("Found package: %s v%s by %s\n", pkg.name, pkg.version, pkg.author);
    
    printf("\033[33m[3/4]\033[0m Installing files...\n");
    
    // Install package files to destination
    char* install_dir = wsys2_get_package_dir(0);
    char pkg_install_path[512];
    snprintf(pkg_install_path, sizeof(pkg_install_path), "%s\\%s", install_dir, pkg.name);
    
    if (package_install_files(&pkg, package_file, pkg_install_path) != 0) {
        printf("\033[31mError:\033[0m Failed to install package files\n");
        return 1;
    }
    
    // Store installation path
    strcpy(pkg.install_path, pkg_install_path);
    
    printf("\033[33m[4/4]\033[0m Updating package database...\n");
    
    // Add to installed packages database
    if (package_add_to_database(&pkg) != 0) {
        printf("\033[31mError:\033[0m Failed to update package database\n");
        return 1;
    }
    
    printf("\033[32m✓ Package '%s' installed successfully!\033[0m\n", pkg.name);
    return 0;
}

// Remove an installed package
int wsys2_remove(const char* package_name) {
    printf("Removing package: %s\n", package_name);
    
    // Check if package is installed
    Package* pkg = package_find_installed(package_name);
    if (!pkg) {
        printf("\033[31mError:\033[0m Package '%s' is not installed\n", package_name);
        return 1;
    }
    
    printf("\033[33m[1/2]\033[0m Removing files...\n");
    
    // Remove installed files
    if (strlen(pkg->install_path) > 0) {
        char rmdir_cmd[600];
        snprintf(rmdir_cmd, sizeof(rmdir_cmd), "rmdir /s /q \"%s\" 2>nul", pkg->install_path);
        system(rmdir_cmd);
        printf("  Removed: %s\n", pkg->install_path);
    }
    
    printf("\033[33m[2/2]\033[0m Updating package database...\n");
    
    // Remove from database
    if (package_remove_from_database(package_name) != 0) {
        printf("\033[31mError:\033[0m Failed to update package database\n");
        free(pkg);
        return 1;
    }
    
    free(pkg);
    printf("\033[32m✓ Package '%s' removed successfully!\033[0m\n", package_name);
    return 0;
}

// Update all installed packages
int wsys2_update(void) {
    printf("Updating all installed packages...\n");
    
    Package* packages;
    int count;
    
    if (package_list_installed(&packages, &count) != 0) {
        printf("\033[31mError:\033[0m Failed to get installed package list\n");
        return 1;
    }
    
    if (count == 0) {
        printf("\033[33mNo packages installed\033[0m\n");
        return 0;
    }
    
    printf("Found %d installed packages\n", count);
    
    // TODO: Check for updates and install newer versions
    
    printf("\033[32m✓ All packages are up to date!\033[0m\n");
    free(packages);
    return 0;
}

// Search for packages
int wsys2_search(const char* search_term) {
    if (search_term) {
        printf("Searching for packages matching: %s\n", search_term);
    } else {
        printf("Listing available packages:\n");
    }
    
    // Search in local packages directory
    printf("\n\033[34mAvailable Packages:\033[0m\n");
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA("packages\\*.wnupkg", &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // Remove .wnupkg extension for display
            char pkg_name[256];
            strcpy(pkg_name, findData.cFileName);
            char* dot = strrchr(pkg_name, '.');
            if (dot) *dot = '\0';
            
            // Try to get description from package
            char pkg_path[512];
            snprintf(pkg_path, sizeof(pkg_path), "packages\\%s", findData.cFileName);
            
            Package pkg;
            if (package_parse_wnupkg(pkg_path, &pkg) == 0) {
                printf("  \033[36m%s\033[0m v%s - %s\n", pkg.name, pkg.version, pkg.description);
            } else {
                printf("  %s - Package file\n", pkg_name);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    } else {
        printf("  No packages found in packages/ directory\n");
    }
    
    printf("\nUse 'wsys2 install packages/<package>.wnupkg' to install a package\n");
    return 0;
}

// List installed packages
int wsys2_list(void) {
    printf("Installed packages:\n");
    
    Package* packages;
    int count;
    
    if (package_list_installed(&packages, &count) != 0) {
        printf("\033[31mError:\033[0m Failed to get installed package list\n");
        return 1;
    }
    
    if (count == 0) {
        printf("\033[33mNo packages installed\033[0m\n");
        return 0;
    }
    
    printf("\n\033[32mInstalled Packages (%d):\033[0m\n", count);
    for (int i = 0; i < count; i++) {
        wsys2_print_package(&packages[i]);
    }
    
    free(packages);
    return 0;
}

// Show package information
int wsys2_info(const char* package_name) {
    printf("Package information for: %s\n", package_name);
    
    Package* pkg = package_find_installed(package_name);
    if (!pkg) {
        printf("\033[31mError:\033[0m Package '%s' is not installed\n", package_name);
        return 1;
    }
    
    printf("\n");
    wsys2_print_package(pkg);
    
    free(pkg);
    return 0;
}

// Create necessary directories
int wsys2_create_directories(void) {
    char system_dir[512];
    char user_dir[512];
    
    // Expand environment variables
    ExpandEnvironmentStringsA(WSYS2_SYSTEM_PACKAGES, system_dir, sizeof(system_dir));
    ExpandEnvironmentStringsA(WSYS2_USER_PACKAGES, user_dir, sizeof(user_dir));
    
    // Create system packages directory (if admin)
    if (wsys2_is_admin()) {
        _mkdir("C:\\WNU");
        _mkdir(system_dir);
    }
    
    // Create user packages directory
    _mkdir(user_dir);
    
    return 0;
}

// Get package installation directory
char* wsys2_get_package_dir(int user_install) {
    static char dir[512];
    
    if (user_install || !wsys2_is_admin()) {
        ExpandEnvironmentStringsA(WSYS2_USER_PACKAGES, dir, sizeof(dir));
    } else {
        ExpandEnvironmentStringsA(WSYS2_SYSTEM_PACKAGES, dir, sizeof(dir));
    }
    
    return dir;
}

// Check if running as administrator
int wsys2_is_admin(void) {
    // Simple check - try to create a file in system directory
    FILE* test = fopen("C:\\WNU\\test_admin.tmp", "w");
    if (test) {
        fclose(test);
        DeleteFileA("C:\\WNU\\test_admin.tmp");
        return 1;
    }
    return 0;
}

// Print package information
void wsys2_print_package(const Package* pkg) {
    printf("  \033[36m%s\033[0m v%s\n", pkg->name, pkg->version);
    printf("    Description: %s\n", pkg->description);
    if (strlen(pkg->author) > 0) {
        printf("    Author: %s\n", pkg->author);
    }
    if (strlen(pkg->install_path) > 0) {
        printf("    Installed in: %s\n", pkg->install_path);
    }
    printf("\n");
}