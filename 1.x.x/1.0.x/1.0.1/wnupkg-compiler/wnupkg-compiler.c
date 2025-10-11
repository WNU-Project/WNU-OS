#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>

void print_banner() {
    printf("\033[36m╔══════════════════════════════════════╗\033[0m\n");
    printf("\033[36m║        WNUPKG Package Compiler       ║\033[0m\n");
    printf("\033[36m║            WNU OS 1.0.1              ║\033[0m\n");
    printf("\033[36m╚══════════════════════════════════════╝\033[0m\n");
    printf("\n");
}

void print_help() {
    printf("Usage: wnupkg-compiler <source_directory> [output.wnupkg]\n\n");
    printf("Description:\n");
    printf("  Compiles a directory structure into a .wnupkg package file\n\n");
    printf("Arguments:\n");
    printf("  source_directory    Directory containing package files and manifest.txt\n");
    printf("  output.wnupkg       Output package file (optional, defaults to package_name.wnupkg)\n\n");
    printf("Directory structure expected:\n");
    printf("  source_directory/\n");
    printf("  ├── manifest.txt    (package metadata)\n");
    printf("  ├── install.bat     (installation script - optional)\n");
    printf("  ├── uninstall.bat   (uninstallation script - optional)\n");
    printf("  └── files/          (files to be installed)\n");
    printf("      ├── bin/\n");
    printf("      ├── docs/\n");
    printf("      └── ...\n\n");
    printf("manifest.txt format:\n");
    printf("  name=package_name\n");
    printf("  version=1.0.0\n");
    printf("  description=Package description\n");
    printf("  author=Author Name\n");
    printf("  install_path=C:\\\\Program Files\\\\PackageName\n\n");
    printf("Examples:\n");
    printf("  wnupkg-compiler my-app/\n");
    printf("  wnupkg-compiler my-app/ custom-name.wnupkg\n");
}

// Function to read manifest file
int read_manifest(const char* manifest_path, char* name, char* version, char* description, char* author, char* install_path) {
    FILE* file = fopen(manifest_path, "r");
    if (!file) {
        printf("\033[31mError:\033[0m Cannot open manifest file: %s\n", manifest_path);
        return 0;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n\r")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') continue;
        
        // Parse key=value pairs
        char* equals = strchr(line, '=');
        if (!equals) continue;
        
        *equals = 0; // Split at equals
        char* key = line;
        char* value = equals + 1;
        
        if (strcmp(key, "name") == 0) {
            strcpy(name, value);
        } else if (strcmp(key, "version") == 0) {
            strcpy(version, value);
        } else if (strcmp(key, "description") == 0) {
            strcpy(description, value);
        } else if (strcmp(key, "author") == 0) {
            strcpy(author, value);
        } else if (strcmp(key, "install_path") == 0) {
            strcpy(install_path, value);
        }
    }
    
    fclose(file);
    return 1;
}

// Function to get file size
long get_file_size(const char* filepath) {
    struct stat st;
    if (stat(filepath, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

// Function to write file content to package
void write_file_to_package(FILE* pkg_file, const char* filepath, const char* relative_path) {
    FILE* src_file = fopen(filepath, "rb");
    if (!src_file) {
        printf("Warning: Cannot read file %s\n", filepath);
        return;
    }
    
    // Write file header
    fprintf(pkg_file, "FILE:%s\n", relative_path);
    
    // Write file content
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        fwrite(buffer, 1, bytes_read, pkg_file);
    }
    
    fprintf(pkg_file, "\n");
    fclose(src_file);
}

// Function to recursively scan directory and add files
void scan_directory(FILE* pkg_file, const char* base_path, const char* current_path, const char* relative_base) {
    char full_path[512];
    char search_path[512];
    
    if (strlen(current_path) > 0) {
        snprintf(full_path, sizeof(full_path), "%s\\%s", base_path, current_path);
        snprintf(search_path, sizeof(search_path), "%s\\*", full_path);
    } else {
        strcpy(full_path, base_path);
        snprintf(search_path, sizeof(search_path), "%s\\*", base_path);
    }
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(search_path, &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }
    
    do {
        // Skip . and ..
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }
        
        char item_full_path[512];
        char item_relative_path[512];
        
        if (strlen(current_path) > 0) {
            snprintf(item_full_path, sizeof(item_full_path), "%s\\%s", full_path, findData.cFileName);
            snprintf(item_relative_path, sizeof(item_relative_path), "%s\\%s\\%s", relative_base, current_path, findData.cFileName);
        } else {
            snprintf(item_full_path, sizeof(item_full_path), "%s\\%s", full_path, findData.cFileName);
            snprintf(item_relative_path, sizeof(item_relative_path), "%s\\%s", relative_base, findData.cFileName);
        }
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively scan subdirectory
            char new_current_path[512];
            if (strlen(current_path) > 0) {
                snprintf(new_current_path, sizeof(new_current_path), "%s\\%s", current_path, findData.cFileName);
            } else {
                strcpy(new_current_path, findData.cFileName);
            }
            scan_directory(pkg_file, base_path, new_current_path, relative_base);
        } else {
            // Add file to package
            printf("Adding file: %s\n", item_relative_path);
            write_file_to_package(pkg_file, item_full_path, item_relative_path);
        }
        
    } while (FindNextFileA(hFind, &findData));
    
    FindClose(hFind);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_banner();
        print_help();
        return 1;
    }
    
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_banner();
        print_help();
        return 0;
    }
    
    const char* source_dir = argv[1];
    char output_file[256];
    
    // Check if source directory exists
    struct stat st;
    if (stat(source_dir, &st) != 0 || !(st.st_mode & S_IFDIR)) {
        printf("\033[31mError:\033[0m Source directory does not exist: %s\n", source_dir);
        return 1;
    }
    
    // Read manifest
    char manifest_path[512];
    snprintf(manifest_path, sizeof(manifest_path), "%s\\manifest.txt", source_dir);
    
    char name[256] = "";
    char version[256] = "";
    char description[512] = "";
    char author[256] = "";
    char install_path[512] = "";
    
    if (!read_manifest(manifest_path, name, version, description, author, install_path)) {
        return 1;
    }
    
    // Validate required fields
    if (strlen(name) == 0) {
        printf("\033[31mError:\033[0m Package name is required in manifest.txt\n");
        return 1;
    }
    
    // Determine output file
    if (argc >= 3) {
        strcpy(output_file, argv[2]);
    } else {
        snprintf(output_file, sizeof(output_file), "%s.wnupkg", name);
    }
    
    printf("Compiling package: %s\n", name);
    printf("Version: %s\n", strlen(version) ? version : "1.0.0");
    printf("Output: %s\n\n", output_file);
    
    // Create output file
    FILE* pkg_file = fopen(output_file, "w");
    if (!pkg_file) {
        printf("\033[31mError:\033[0m Cannot create output file: %s\n", output_file);
        return 1;
    }
    
    // Write manifest section
    fprintf(pkg_file, "[MANIFEST]\n");
    fprintf(pkg_file, "name=%s\n", name);
    fprintf(pkg_file, "version=%s\n", strlen(version) ? version : "1.0.0");
    if (strlen(description)) fprintf(pkg_file, "description=%s\n", description);
    if (strlen(author)) fprintf(pkg_file, "author=%s\n", author);
    if (strlen(install_path)) fprintf(pkg_file, "install_path=%s\n", install_path);
    fprintf(pkg_file, "\n");
    
    // Write files section
    fprintf(pkg_file, "[FILES]\n");
    
    // Check if files directory exists
    char files_dir[512];
    snprintf(files_dir, sizeof(files_dir), "%s\\files", source_dir);
    if (stat(files_dir, &st) == 0 && (st.st_mode & S_IFDIR)) {
        printf("Scanning files directory...\n");
        scan_directory(pkg_file, files_dir, "", "files");
    }
    
    fprintf(pkg_file, "\n");
    
    // Write install section
    fprintf(pkg_file, "[INSTALL]\n");
    char install_script[512];
    snprintf(install_script, sizeof(install_script), "%s\\install.bat", source_dir);
    if (stat(install_script, &st) == 0) {
        printf("Adding install script...\n");
        FILE* install_file = fopen(install_script, "r");
        if (install_file) {
            char line[512];
            while (fgets(line, sizeof(line), install_file)) {
                fprintf(pkg_file, "%s", line);
            }
            fclose(install_file);
        }
    } else {
        fprintf(pkg_file, "@echo off\n");
        fprintf(pkg_file, "echo Package installed successfully!\n");
    }
    fprintf(pkg_file, "\n");
    
    // Write uninstall section
    fprintf(pkg_file, "[UNINSTALL]\n");
    char uninstall_script[512];
    snprintf(uninstall_script, sizeof(uninstall_script), "%s\\uninstall.bat", source_dir);
    if (stat(uninstall_script, &st) == 0) {
        printf("Adding uninstall script...\n");
        FILE* uninstall_file = fopen(uninstall_script, "r");
        if (uninstall_file) {
            char line[512];
            while (fgets(line, sizeof(line), uninstall_file)) {
                fprintf(pkg_file, "%s", line);
            }
            fclose(uninstall_file);
        }
    } else {
        fprintf(pkg_file, "@echo off\n");
        fprintf(pkg_file, "echo Package uninstalled successfully!\n");
    }
    
    fclose(pkg_file);
    
    printf("\n\033[32m✓ Package compiled successfully!\033[0m\n");
    printf("Output: %s\n", output_file);
    
    return 0;
}