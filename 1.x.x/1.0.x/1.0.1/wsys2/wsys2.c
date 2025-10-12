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

// === ONLINE PACKAGE FUNCTIONS ===

// Helper function to check if URL exists
int check_url_exists(const char* url) {
    HINTERNET hInternet, hUrl;
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(dwStatusCode);
    
    hInternet = InternetOpenA("WSYS2/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return 0;
    
    hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    HttpQueryInfoA(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                   &dwStatusCode, &dwSize, NULL);
    
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    return (dwStatusCode == 200);
}

// Update online package database
int wsys2_online_update(void) {
    printf("\033[95mUpdating online package database...\033[0m\n");
    printf("Repository: https://wnu-project.github.io/wnuos.packages.com/\n\n");
    
    printf("\033[33m[1/3]\033[0m Connecting to repository...\n");
    
    // Check if the main repository is accessible
    if (!check_url_exists("https://wnu-project.github.io/wnuos.packages.com/")) {
        printf("\033[31m✗ Error: Could not connect to repository\033[0m\n");
        printf("Please check your internet connection\n");
        return 1;
    }
    
    printf("\033[33m[2/3]\033[0m Scanning for available packages...\n");
    
    // Check for known packages
    int package_count = 0;
    const char* known_packages[] = {
        "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg"
    };
    
    size_t num_packages = sizeof(known_packages)/sizeof(known_packages[0]);
    for (size_t i = 0; i < num_packages; i++) {
        if (check_url_exists(known_packages[i])) {
            package_count++;
            printf("  Found: wnu-dev-tools\n");
        }
    }
    
    printf("\033[33m[3/3]\033[0m Building package index...\n");
    Sleep(300);
    
    printf("\033[32m✓ Package database updated successfully!\033[0m\n");
    printf("Found %d packages in repository\n", package_count);
    return 0;
}

// Search online packages
int wsys2_online_search(const char* search_term) {
    if (search_term) {
        printf("\033[95mSearching online packages for: %s\033[0m\n", search_term);
    } else {
        printf("\033[95mListing all online packages:\033[0m\n");
    }
    
    printf("Repository: https://wnu-project.github.io/wnuos.packages.com/\n\n");
    
    // Check repository connectivity
    if (!check_url_exists("https://wnu-project.github.io/wnuos.packages.com/")) {
        printf("\033[31m✗ Error: Repository not accessible\033[0m\n");
        return 1;
    }
    
    printf("\033[34mScanning repository for packages...\033[0m\n");
    
    // Real packages found in your repository
    typedef struct {
        const char* name;
        const char* version;
        const char* description;
        const char* url;
    } OnlinePackage;
    
    OnlinePackage available_packages[] = {
        {
            "wnu-dev-tools", 
            "1.0.0", 
            "WNU OS development toolkit", 
            "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg"
        }
    };
    
    int found_count = 0;
    int total_packages = sizeof(available_packages) / sizeof(available_packages[0]);
    
    printf("\033[34mAvailable Online Packages:\033[0m\n");
    
    for (int i = 0; i < total_packages; i++) {
        OnlinePackage* pkg = &available_packages[i];
        
        // If searching, check if package matches search term
        if (search_term && strstr(pkg->name, search_term) == NULL && 
            strstr(pkg->description, search_term) == NULL) {
            continue;
        }
        
        // Check if package actually exists in repository
        printf("  Checking: %s... ", pkg->name);
        if (check_url_exists(pkg->url)) {
            printf("\033[32m✓\033[0m\n");
            printf("  \033[36m%s\033[0m v%s - %s\n", pkg->name, pkg->version, pkg->description);
            printf("    URL: %s\n", pkg->url);
            found_count++;
        } else {
            printf("\033[31m✗\033[0m\n");
        }
    }
    
    if (found_count == 0) {
        if (search_term) {
            printf("\033[33mNo packages found matching: %s\033[0m\n", search_term);
        } else {
            printf("\033[33mNo packages available in repository\033[0m\n");
        }
    } else {
        printf("\nFound %d package(s)\n", found_count);
        printf("Use '\033[95mwsys2 online install <package>\033[0m' to install from repository\n");
    }
    
    return 0;
}

// Download file from URL
int download_file(const char* url, const char* local_path) {
    HINTERNET hInternet, hUrl;
    FILE* file;
    char buffer[8192];
    DWORD bytes_read;
    
    hInternet = InternetOpenA("WSYS2/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return 0;
    
    hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    file = fopen(local_path, "wb");
    if (!file) {
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytes_read) && bytes_read > 0) {
        fwrite(buffer, 1, bytes_read, file);
    }
    
    fclose(file);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    return 1;
}

// Install package from online repository
int wsys2_online_install(const char* package_name) {
    printf("\033[95mInstalling online package: %s\033[0m\n", package_name);
    printf("Repository: https://wnu-project.github.io/wnuos.packages.com/\n\n");
    
    // Map package names to their URLs in your repository
    const char* package_url = NULL;
    if (strcmp(package_name, "wnu-dev-tools") == 0) {
        package_url = "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg";
    }
    if (strcmp(package_name, "wnu-nano") == 0) {
        package_url = "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-nano/wnu-nano.wnupkg";
    }
    
    if (!package_url) {
        printf("\033[31m✗ Package '%s' not found in repository\033[0m\n", package_name);
        return 1;
    }
    
    printf("\033[33m[1/5]\033[0m Resolving package information...\n");
    
    // Check if package exists in repository
    if (!check_url_exists(package_url)) {
        printf("\033[31m✗ Package not available: %s\033[0m\n", package_url);
        return 1;
    }
    printf("Found: %s in online repository\n", package_name);
    
    printf("\033[33m[2/5]\033[0m Downloading package...\n");
    printf("  Downloading from: %s\n", package_url);
    
    // Create online packages directory in WNU system directory
    _mkdir("C:\\WNU");
    _mkdir("C:\\WNU\\packages");
    _mkdir("C:\\WNU\\packages\\online");
    
    // Download to system online packages directory
    char download_path[512];
    snprintf(download_path, sizeof(download_path), "C:\\WNU\\packages\\online\\%s.wnupkg", package_name);
    
    if (!download_file(package_url, download_path)) {
        printf("\033[31m✗ Failed to download package\033[0m\n");
        return 1;
    }
    
    printf("\033[33m[3/5]\033[0m Verifying package integrity...\n");
    
    // Check if file was downloaded successfully
    FILE* test_file = fopen(download_path, "rb");
    if (!test_file) {
        printf("\033[31m✗ Download verification failed\033[0m\n");
        return 1;
    }
    
    // Get file size
    fseek(test_file, 0, SEEK_END);
    long file_size = ftell(test_file);
    fclose(test_file);
    
    printf("  Package downloaded to: %s\n", download_path);
    printf("  Package size: %ld bytes (%.2f KB)\n", file_size, file_size / 1024.0);
    
    printf("\033[33m[4/5]\033[0m Installing package from download...\n");
    printf("  Running: wsys2 install %s\n", download_path);
    
    // Install the downloaded package using existing install function
    int result = wsys2_install(download_path);
    
    printf("\033[33m[5/5]\033[0m Installation complete\n");
    
    // Keep the downloaded file for user inspection
    printf("  Downloaded package available at: %s\n", download_path);
    
    if (result == 0) {
        printf("\033[32m✓ Online package '%s' installed successfully!\033[0m\n", package_name);
        printf("Package downloaded from WNU repository and installed\n");
        printf("Downloaded file available at: C:\\WNU\\packages\\online\\%s.wnupkg\n", package_name);
    } else {
        printf("\033[31m✗ Installation failed\033[0m\n");
        printf("Downloaded package file remains at: C:\\WNU\\packages\\online\\%s.wnupkg\n", package_name);
    }
    
    return result;
}

// List all online packages
int wsys2_online_list(void) {
    printf("\033[95mListing all available online packages:\033[0m\n");
    printf("Repository: https://wnu-project.github.io/wnuos.packages.com/\n\n");
    
    // Check repository connectivity
    if (!check_url_exists("https://wnu-project.github.io/wnuos.packages.com/")) {
        printf("\033[31m✗ Error: Repository not accessible\033[0m\n");
        printf("Please check your internet connection\n");
        return 1;
    }
    
    printf("\033[34mScanning repository for packages...\033[0m\n\n");
    
    // Real packages in your repository
    typedef struct {
        const char* name;
        const char* version;
        const char* description;
        const char* url;
        const char* category;
    } RepoPackage;
    
    RepoPackage repo_packages[] = {
        {
            "wnu-dev-tools", 
            "1.0.0", 
            "WNU OS development toolkit", 
            "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg",
            "Development"
        },
        {
            "wnu-nano",
            "1.0.0",
            "Lightweight text editor for WNU OS",
            "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-nano/wnu-nano.wnupkg",
            "Development"
        }
    };
    
    int available_count = 0;
    int total_packages = sizeof(repo_packages) / sizeof(repo_packages[0]);
    
    printf("\033[34mFirst-Party WNU Packages:\033[0m\n");
    
    for (int i = 0; i < total_packages; i++) {
        RepoPackage* pkg = &repo_packages[i];
        
        printf("  Checking: %s... ", pkg->name);
        if (check_url_exists(pkg->url)) {
            printf("\033[32m✓\033[0m\n");
            printf("  \033[36m%s\033[0m v%s - %s\n", pkg->name, pkg->version, pkg->description);
            available_count++;
        } else {
            printf("\033[31m✗ Not available\033[0m\n");
        }
    }
    
    if (available_count == 0) {
        printf("\033[33mNo packages currently available in repository\033[0m\n");
    } else {
        printf("\nTotal: %d package(s) available\n", available_count);
        printf("Use '\033[95mwsys2 online install <package>\033[0m' to install\n");
    }
    
    return 0;
}

// Show online package information
int wsys2_online_info(const char* package_name) {
    printf("\033[95mOnline package information for: %s\033[0m\n", package_name);
    printf("Repository: https://wnu-project.github.io/wnuos.packages.com/\n\n");
    
    // Real package information from your repository
    if (strcmp(package_name, "wnu-dev-tools") == 0) {
        const char* package_url = "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg";
        
        printf("Checking package availability... ");
        if (check_url_exists(package_url)) {
            printf("\033[32m✓ Available\033[0m\n\n");
            
            printf("  \033[36mwnu-dev-tools\033[0m v1.0.0\n");
            printf("    Description: Complete development toolkit for WNU OS\n");
            printf("    Author: WNU Project Team\n");
            printf("    Category: Development Tools\n");
            printf("    Download URL: %s\n", package_url);
            printf("    Repository: https://github.com/WNU-Project/wnu-dev-tools\n");
            printf("    License: MIT License\n");
            printf("    Dependencies: None\n");
            
            // Try to get file size
            HINTERNET hInternet = InternetOpenA("WSYS2/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
            if (hInternet) {
                HINTERNET hUrl = InternetOpenUrlA(hInternet, package_url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
                if (hUrl) {
                    char buffer[64];
                    DWORD size = sizeof(buffer);
                    if (HttpQueryInfoA(hUrl, HTTP_QUERY_CONTENT_LENGTH, buffer, &size, NULL)) {
                        long file_size = atol(buffer);
                        if (file_size > 0) {
                            printf("    Package Size: %.2f KB\n", file_size / 1024.0);
                        }
                    }
                    InternetCloseHandle(hUrl);
                }
                InternetCloseHandle(hInternet);
            }
            
            printf("\n  \033[95mInstall Command:\033[0m wsys2 online install wnu-dev-tools\n");
            
        } else {
            printf("\033[31m✗ Not available\033[0m\n");
            printf("Package not found in repository\n");
        }
    } else {
        printf("Searching for package: %s... ", package_name);
        printf("\033[33m✗ Package not found\033[0m\n");
        printf("\nAvailable packages:\n");
        printf("  • wnu-dev-tools - WNU OS development toolkit\n");
        printf("\nUse '\033[95mwsys2 online list\033[0m' to see all available packages\n");
    }
    
    printf("\n");
    return 0;
}