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
    
    // Conservative update check: first prefer any already-downloaded package
    // in C:\WNU\packages\online, compare its manifest against the
    // installed package. If a repository URL is known and reachable, also
    // download the repository copy to temp and compare that too.
    typedef struct { const char* name; const char* version; const char* url; } KnownPkg;
    KnownPkg known[] = {
        {"wnu-dev-tools", "1.0.0", "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-dev-tools/wnu-dev-tools.wnupkg"},
        {"wnu-nano", "1.0.0", "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-nano/wnu-nano.wnupkg"},
        {"wnu-calculator", "1.0.0", "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-calculator/wnu-calculator.wnupkg"},
        {NULL, NULL, NULL}
    };

    int updates_found = 0;

    // Ensure temp dir exists for remote download comparisons
    char temp_dir[512];
    ExpandEnvironmentStringsA(WSYS2_TEMP_DIR, temp_dir, sizeof(temp_dir));
    _mkdir(temp_dir);

    for (int i = 0; i < count; i++) {
        Package* p = &packages[i];

        // Look for a locally downloaded package first
        char local_online_path[512];
        snprintf(local_online_path, sizeof(local_online_path), "C:\\WNU\\packages\\online\\%s.wnupkg", p->name);

        int considered_remote = 0;
        Package local_pkg = {0};
        Package remote_pkg = {0};

        if (GetFileAttributesA(local_online_path) != INVALID_FILE_ATTRIBUTES) {
            // Parse local downloaded package
            if (package_parse_wnupkg(local_online_path, &local_pkg) == 0) {
                // Compare installed -> local downloaded
                int cmp_local = version_compare(p->version, local_pkg.version);
                if (cmp_local < 0) {
                    printf("Update available (downloaded): %s v%s -> v%s\n", p->name, p->version, local_pkg.version);
                    printf("  Local file: %s\n", local_online_path);
                    printf("  To upgrade: wsys2 install %s\n", local_online_path);
                    updates_found++;
                    // Even if local is newer than installed, we still try to check remote repo
                } else if (cmp_local == 0) {
                    printf("Local downloaded package for %s matches installed version (v%s)\n", p->name, p->version);
                } else {
                    printf("Local downloaded package for %s is older than installed (local v%s, installed v%s)\n", p->name, local_pkg.version, p->version);
                }
            } else {
                printf("Warning: could not parse local downloaded package: %s\n", local_online_path);
            }
        }

        // If we have a known repo URL for this package, compare against repo
        const char* repo_url = NULL;
        for (int k = 0; known[k].name != NULL; k++) {
            if (strcmp(p->name, known[k].name) == 0) { repo_url = known[k].url; break; }
        }

        if (repo_url) {
            considered_remote = 1;
            // Check repo availability
            if (check_url_exists(repo_url)) {
                // Download remote package to temp and parse
                char remote_tmp[1024];
                snprintf(remote_tmp, sizeof(remote_tmp), "%s\\repo_%s.wnupkg", temp_dir, p->name);
                if (download_file(repo_url, remote_tmp)) {
                    if (package_parse_wnupkg(remote_tmp, &remote_pkg) == 0) {
                        // Compare installed -> remote
                        int cmp_remote = version_compare(p->version, remote_pkg.version);
                        if (cmp_remote < 0) {
                            printf("Update available (repo): %s v%s -> v%s\n", p->name, p->version, remote_pkg.version);
                            printf("  URL: %s\n", repo_url);
                            printf("  To upgrade: wsys2 online install %s\n", p->name);
                            updates_found++;
                        } else if (cmp_remote == 0) {
                            // If local downloaded exists, compare local vs remote
                            if (GetFileAttributesA(local_online_path) != INVALID_FILE_ATTRIBUTES && strlen(local_pkg.version) > 0) {
                                int cmp_local_remote = version_compare(local_pkg.version, remote_pkg.version);
                                if (cmp_local_remote < 0) {
                                    printf("Repository has newer package for %s (local v%s -> repo v%s). Consider re-downloading.\n", p->name, local_pkg.version, remote_pkg.version);
                                } else if (cmp_local_remote == 0) {
                                    // equal
                                } else {
                                    printf("Local downloaded package for %s is newer than repo (local v%s -> repo v%s).\n", p->name, local_pkg.version, remote_pkg.version);
                                }
                            }
                        }
                    } else {
                        printf("Warning: failed to parse remote package for %s\n", p->name);
                    }
                    // remove temp file
                    DeleteFileA(remote_tmp);
                } else {
                    printf("Warning: failed to download remote package for %s\n", p->name);
                }
            } else {
                // repo not accessible
                // nothing to do here; local comparisons already done above
            }
        }

        // If no local download and no known repo, fall back to message
        if (!considered_remote && GetFileAttributesA(local_online_path) == INVALID_FILE_ATTRIBUTES) {
            // no local or remote info available for this package
            // nothing to report for this package
        }
    }

    if (updates_found == 0) {
        printf("\033[32m✓ All packages are up to date!\033[0m\n");
    } else {
        printf("\033[33m%d updates available. Use the suggested install commands to upgrade.\033[0m\n", updates_found);
    }
    free(packages);
    return 0;
}

int wsys2_upgrade(const char* package_name) {
    if (!package_name || strlen(package_name) == 0) {
        printf("\033[31mError:\033[0m No package specified for upgrade\n");
        return 1;
    }

    printf("Upgrading: %s\n", package_name);
    printf("Continue? [y/n] ");

    // Read a single non-whitespace character from stdin safely
    int c = getchar();
    while (c != EOF && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) c = getchar();

    if (c == 'y' || c == 'Y') {
        printf("Proceeding with upgrade of %s\n", package_name);

        // Remove current package first (best-effort)
        if (wsys2_remove(package_name) != 0) {
            printf("\033[33mWarning:\033[0m Failed to remove existing package; continuing to attempt install\n");
        }

        // Install package from online repository
        int res = wsys2_online_install(package_name);
        if (res == 0) {
            printf("\033[32m✓ Upgrade of %s completed successfully\033[0m\n", package_name);
            return 0;
        } else {
            printf("\033[31m✗ Upgrade of %s failed\033[0m\n", package_name);
            return 1;
        }
    } else {
        printf("Upgrade cancelled\n");
        return 0;
    }
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

// Compare two semantic version strings ("1.2.3").
// Returns -1 if a < b, 0 if equal, 1 if a > b
int version_compare(const char* a, const char* b) {
    if (!a || !b) return 0;
    int ai = 0, bi = 0;
    char a_copy[128];
    char b_copy[128];
    strncpy(a_copy, a, sizeof(a_copy)-1); a_copy[sizeof(a_copy)-1] = '\0';
    strncpy(b_copy, b, sizeof(b_copy)-1); b_copy[sizeof(b_copy)-1] = '\0';

    char* atok = strtok(a_copy, ".");
    char* btok = strtok(b_copy, ".");
    while (atok || btok) {
        ai = atok ? atoi(atok) : 0;
        bi = btok ? atoi(btok) : 0;
        if (ai < bi) return -1;
        if (ai > bi) return 1;
        atok = atok ? strtok(NULL, ".") : NULL;
        btok = btok ? strtok(NULL, ".") : NULL;
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
    if (strcmp(package_name, "wnu-calculator") == 0) {
        package_url = "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-calculator/wnu-calculator.wnupkg";
    }
    
    if (!package_url) {
        printf("\033[31m✗ Package '%s' not found in repository\033[0m\033[33m Or Not Approved\033[0m\n", package_name);
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
    
    RepoPackage repo_packages_1st_party[] = {
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
        },
        {
            "wnu-calculator",
            "1.0.0",
            "Simple calculator application for WNU OS",
            "https://wnu-project.github.io/wnuos.packages.com/WNU-Project@wnu-calculator/wnu-calculator.wnupkg",
            "Math"
        }
    };
    int available_count = 0;
    int total_packages = sizeof(repo_packages_1st_party) / sizeof(repo_packages_1st_party[0]);

    printf("\033[34mFirst-Party WNU Packages:\033[0m\n");

    for (int i = 0; i < total_packages; i++) {
        RepoPackage* pkg = &repo_packages_1st_party[i];

        printf("  Checking: %s... ", pkg->name);
        if (check_url_exists(pkg->url)) {
            printf("\033[32m✓\033[0m\n");
            printf("  \033[36m%s\033[0m v%s - %s\n", pkg->name, pkg->version, pkg->description);
            available_count++;
        } else {
            printf("\033[31m✗ Not available\033[0m\n");
        }
    }

    /* Second-Party and Third-Party lists: none available at the moment */
    printf("\n\033[34mSecond-Party WNU Packages:\033[0m\n");
    printf("  None as of now, However you can reach out To The WNU Project, To Collab To Make a 2nd party package!\n");

    printf("\n\033[34mThird-Party WNU Packages:\033[0m\n");
    printf("  None as of now, However, you can Add them to Repo, but it had to be verifed!\n");
    
    if (available_count == 0) {
        printf("\033[33mNo packages currently available in repository\033[0m\n");
    } else {
        printf("\nTotal: %d package(s) available\n", available_count);
        printf("Use '\033[95mwsys2 online install <package>\033[0m' to install\n");
    }
    
    return 0;
}

// Recursive function to find .exe files in package directory
int wsys2_find_executable_recursive(const char* base_path, char* exe_name, char* exe_dir, char* exe_path) {
    char search_pattern[1024];
    
    // Search for .exe files in current directory
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*.exe", base_path);
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(search_pattern, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        // Found an exe file, use it
        strcpy(exe_name, findData.cFileName);
        char* dot = strrchr(exe_name, '.');
        if (dot) *dot = '\0';  // Remove .exe extension
        
        // Get relative directory path
        const char* relative_path = base_path + strlen(wsys2_get_package_dir(0)) + 1;
        const char* package_name_end = strchr(relative_path, '\\');
        if (package_name_end) {
            strcpy(exe_dir, package_name_end + 1);
        } else {
            strcpy(exe_dir, "root");
        }
        
        snprintf(exe_path, 1024, "%s\\%s", base_path, findData.cFileName);
        FindClose(hFind);
        return 1;
    }
    
    // Search in subdirectories
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*", base_path);
    hFind = FindFirstFileA(search_pattern, &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
                strcmp(findData.cFileName, ".") != 0 && 
                strcmp(findData.cFileName, "..") != 0) {
                
                char subdir_path[1024];
                snprintf(subdir_path, sizeof(subdir_path), "%s\\%s", base_path, findData.cFileName);
                
                // Recursively search in subdirectory
                if (wsys2_find_executable_recursive(subdir_path, exe_name, exe_dir, exe_path)) {
                    FindClose(hFind);
                    return 1;
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    return 0;
}

// Run a program from an installed package
int wsys2_run(const char* package_spec, const char* program_name, char** args, int arg_count) {
    printf("Running program from package: %s\n", package_spec);
    
    // Parse package specification (Maintainer@pkgname or just pkgname)
    char maintainer[128] = {0};
    char pkgname[256] = {0};
    
    const char* at_symbol = strchr(package_spec, '@');
    if (at_symbol) {
        // Format: Maintainer@pkgname
        size_t maintainer_len = at_symbol - package_spec;
        if (maintainer_len >= sizeof(maintainer)) maintainer_len = sizeof(maintainer) - 1;
        strncpy(maintainer, package_spec, maintainer_len);
        maintainer[maintainer_len] = '\0';
        
        strncpy(pkgname, at_symbol + 1, sizeof(pkgname) - 1);
    } else {
        // Format: just pkgname
        strncpy(pkgname, package_spec, sizeof(pkgname) - 1);
    }
    
    printf("Looking for package: %s", pkgname);
    if (strlen(maintainer) > 0) {
        printf(" (by %s)", maintainer);
    }
    printf("\n");
    
    // Find the installed package
    Package* pkg = package_find_installed(pkgname);
    if (!pkg) {
        printf("\033[31m✗ Package '%s' is not installed\033[0m\n", pkgname);
        printf("Use 'wsys2 list' to see installed packages\n");
        return 1;
    }
    
    // Verify maintainer if specified
    if (strlen(maintainer) > 0 && strlen(pkg->author) > 0) {
        if (strcmp(pkg->author, maintainer) != 0) {
            printf("\033[33m! Warning: Package author '%s' doesn't match specified maintainer '%s'\033[0m\n", 
                   pkg->author, maintainer);
        }
    }
    
    printf("Found installed package: %s v%s by %s\n", pkg->name, pkg->version, pkg->author);
    printf("Package installed at: %s\n", pkg->install_path);
    
    // Debug: List all directories and files in the package
    printf("Package contents:\n");
    char search_pattern[1024];
    snprintf(search_pattern, sizeof(search_pattern), "%s\\*", pkg->install_path);
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(search_pattern, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    printf("  [DIR]  %s/\n", findData.cFileName);
                    
                    // List files in subdirectories
                    char subdir_pattern[1024];
                    snprintf(subdir_pattern, sizeof(subdir_pattern), "%s\\%s\\*", pkg->install_path, findData.cFileName);
                    WIN32_FIND_DATAA subFindData;
                    HANDLE hSubFind = FindFirstFileA(subdir_pattern, &subFindData);
                    if (hSubFind != INVALID_HANDLE_VALUE) {
                        do {
                            if (subFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                                if (strcmp(subFindData.cFileName, ".") != 0 && strcmp(subFindData.cFileName, "..") != 0) {
                                    printf("    [DIR]  %s/%s/\n", findData.cFileName, subFindData.cFileName);
                                    
                                    // Check one more level deep
                                    char deep_pattern[1024];
                                    snprintf(deep_pattern, sizeof(deep_pattern), "%s\\%s\\%s\\*", pkg->install_path, findData.cFileName, subFindData.cFileName);
                                    WIN32_FIND_DATAA deepFindData;
                                    HANDLE hDeepFind = FindFirstFileA(deep_pattern, &deepFindData);
                                    if (hDeepFind != INVALID_HANDLE_VALUE) {
                                        do {
                                            if (!(deepFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                                printf("      [FILE] %s/%s/%s\n", findData.cFileName, subFindData.cFileName, deepFindData.cFileName);
                                            }
                                        } while (FindNextFileA(hDeepFind, &deepFindData));
                                        FindClose(hDeepFind);
                                    }
                                }
                            } else {
                                printf("    [FILE] %s/%s\n", findData.cFileName, subFindData.cFileName);
                            }
                        } while (FindNextFileA(hSubFind, &subFindData));
                        FindClose(hSubFind);
                    }
                }
            } else {
                printf("  [FILE] %s\n", findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    } else {
        printf("  Could not read package directory: %s\n", pkg->install_path);
    }
    
    // Build path to executable
    char exe_path[1024];
    int found_executable = 0;
    
    if (program_name) {
        // Specific program requested
        snprintf(exe_path, sizeof(exe_path), "%s\\bin\\%s.exe", pkg->install_path, program_name);
        if (GetFileAttributesA(exe_path) != INVALID_FILE_ATTRIBUTES) {
            found_executable = 1;
        }
    } else {
        // Try to find main executable - try multiple naming patterns
        char* possible_names[] = {
            pkg->name,                    // wnu-nano
            NULL,                        // Will be set to wnuos-{name}
            NULL                         // Sentinel
        };
        
        // Generate wnuos-prefixed name
        char wnuos_name[512];
        snprintf(wnuos_name, sizeof(wnuos_name), "wnuos-%s", pkg->name);
        possible_names[1] = wnuos_name;
        
        // Try each possible name
        for (int i = 0; possible_names[i] != NULL; i++) {
            snprintf(exe_path, sizeof(exe_path), "%s\\bin\\%s.exe", pkg->install_path, possible_names[i]);
            if (GetFileAttributesA(exe_path) != INVALID_FILE_ATTRIBUTES) {
                found_executable = 1;
                printf("Found executable: %s\n", possible_names[i]);
                break;
            }
        }
        
        // If still not found, try to find any .exe file in multiple directories
        if (!found_executable) {
            // Recursive search function for finding .exe files
            char exe_name[512];
            char exe_dir[512];
            found_executable = wsys2_find_executable_recursive(pkg->install_path, exe_name, exe_dir, exe_path);
            
            if (found_executable) {
                printf("Auto-detected executable: %s in %s directory\n", exe_name, exe_dir);
            }
        }
    }
    
    // Check if we found an executable
    if (!found_executable) {
        printf("\033[31m✗ No executable found for package: %s\033[0m\n", pkg->name);
        printf("Searched directories: bin/, root, exe/, programs/\n");
        
        // List all .exe files found anywhere in the package
        printf("Searching for ALL .exe files in package...\n");
        char search_pattern[1024];
        snprintf(search_pattern, sizeof(search_pattern), "%s\\**\\*.exe", pkg->install_path);
        
        // Manual recursive search since Windows FindFirstFile doesn't do recursive search
        char* search_locations[] = {".", "files\\bin", "files\\exe", "files\\programs", NULL};
        int found_any = 0;
        
        for (int i = 0; search_locations[i] != NULL; i++) {
            char location_pattern[1024];
            if (strcmp(search_locations[i], ".") == 0) {
                snprintf(location_pattern, sizeof(location_pattern), "%s\\*.exe", pkg->install_path);
            } else {
                snprintf(location_pattern, sizeof(location_pattern), "%s\\%s\\*.exe", pkg->install_path, search_locations[i]);
            }
            
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA(location_pattern, &findData);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                if (!found_any) {
                    printf("Found .exe files in package:\n");
                    found_any = 1;
                }
                do {
                    char* dot = strrchr(findData.cFileName, '.');
                    if (dot) *dot = '\0';  // Remove .exe extension for display
                    printf("  %s (in %s directory)\n", findData.cFileName, 
                           strcmp(search_locations[i], ".") == 0 ? "root" : search_locations[i]);
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }
        }
        
        if (!found_any) {
            printf("No .exe files found anywhere in the package directory\n");
            printf("This might indicate a package installation issue\n");
        } else {
            printf("Usage: wsys2 run %s <program_name>\n", package_spec);
        }
        
        free(pkg);
        return 1;
    }
    
    printf("Executing: %s\n", exe_path);
    
    // Build command line arguments
    char command_line[2048];
    strcpy(command_line, exe_path);
    
    // Add arguments if provided
    for (int i = 0; i < arg_count; i++) {
        strcat(command_line, " ");
        strcat(command_line, args[i]);
    }
    
    printf("Command: %s\n", command_line);
    
    // Execute the program
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    if (CreateProcessA(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("\033[32m✓ Program started successfully\033[0m\n");
        printf("Process ID: %lu\n", pi.dwProcessId);
        
        // Wait for process to complete
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        // Get exit code
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        printf("Program exited with code: %lu\n", exit_code);
        
        free(pkg);
        return (exit_code == 0) ? 0 : 1;
    } else {
        printf("\033[31m✗ Failed to start program: %s\033[0m\n", exe_path);
        printf("Error code: %lu\n", GetLastError());
        free(pkg);
        return 1;
    }
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
