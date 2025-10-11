#include "wsys2.h"
#include <direct.h>

// Parse a .wnupkg file and extract package information
int package_parse_wnupkg(const char* wnupkg_file, Package* pkg) {
    FILE* file = fopen(wnupkg_file, "r");
    if (!file) {
        return 1;
    }
    
    char line[1024];
    int in_manifest = 0;
    
    // Initialize package structure
    memset(pkg, 0, sizeof(Package));
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\0') continue;
        
        // Check for sections
        if (strcmp(line, "[MANIFEST]") == 0) {
            in_manifest = 1;
            continue;
        } else if (line[0] == '[') {
            in_manifest = 0;
            continue;
        }
        
        // Parse manifest entries
        if (in_manifest) {
            char* equals = strchr(line, '=');
            if (equals) {
                *equals = '\0';
                char* key = line;
                char* value = equals + 1;
                
                if (strcmp(key, "name") == 0) {
                    strncpy(pkg->name, value, sizeof(pkg->name) - 1);
                } else if (strcmp(key, "version") == 0) {
                    strncpy(pkg->version, value, sizeof(pkg->version) - 1);
                } else if (strcmp(key, "description") == 0) {
                    strncpy(pkg->description, value, sizeof(pkg->description) - 1);
                } else if (strcmp(key, "author") == 0) {
                    strncpy(pkg->author, value, sizeof(pkg->author) - 1);
                } else if (strcmp(key, "dependencies") == 0) {
                    strncpy(pkg->dependencies, value, sizeof(pkg->dependencies) - 1);
                }
            }
        }
    }
    
    fclose(file);
    
    // Validate required fields
    if (strlen(pkg->name) == 0 || strlen(pkg->version) == 0) {
        return 1;
    }
    
    return 0;
}

// Install package files from .wnupkg to destination directory
int package_install_files(const Package* pkg, const char* source_file, const char* dest_dir) {
    (void)pkg; // Mark as intentionally unused
    // Create destination directory
    _mkdir(dest_dir);
    
    char bin_dir[512];
    snprintf(bin_dir, sizeof(bin_dir), "%s\\bin", dest_dir);
    _mkdir(bin_dir);
    
    // Extract and install files from the .wnupkg
    FILE* wnupkg = fopen(source_file, "r");
    if (!wnupkg) {
        return 1;
    }
    
    char line[1024];
    int in_files_section = 0;
    FILE* current_file = NULL;
    char current_filename[256] = {0};
    
    while (fgets(line, sizeof(line), wnupkg)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Check for sections
        if (strcmp(line, "[FILES]") == 0) {
            in_files_section = 1;
            continue;
        } else if (line[0] == '[' && strcmp(line, "[FILES]") != 0) {
            // Close current file if open
            if (current_file) {
                fclose(current_file);
                current_file = NULL;
            }
            in_files_section = 0;
            continue;
        }
        
        if (in_files_section) {
            // Check for file markers (lines starting with # followed by filename)
            if (line[0] == '#' && line[1] == ' ') {
                // Close previous file
                if (current_file) {
                    fclose(current_file);
                    current_file = NULL;
                }
                
                // Extract filename
                strcpy(current_filename, line + 2);
                
                // Create full path
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s\\%s", dest_dir, current_filename);
                
                // Create directory if needed
                char* last_slash = strrchr(full_path, '\\');
                if (last_slash) {
                    *last_slash = '\0';
                    // Create directory recursively (simple approach)
                    char mkdir_cmd[600];
                    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir \"%s\" 2>nul", full_path);
                    system(mkdir_cmd);
                    *last_slash = '\\';
                }
                
                // Open file for writing
                current_file = fopen(full_path, "w");
                if (!current_file) {
                    printf("Warning: Could not create file %s\n", full_path);
                }
                
                printf("  Installing: %s\n", current_filename);
            } else if (current_file && line[0] != '\0') {
                // Write file content
                fprintf(current_file, "%s\n", line);
            }
        }
    }
    
    // Close any remaining open file
    if (current_file) {
        fclose(current_file);
    }
    
    fclose(wnupkg);
    return 0;
}

// Add package to installed database
int package_add_to_database(const Package* pkg) {
    char db_path[512];
    char* pkg_dir = wsys2_get_package_dir(0);
    snprintf(db_path, sizeof(db_path), "%s\\%s", pkg_dir, WSYS2_DATABASE_FILE);
    
    FILE* db = fopen(db_path, "a");
    if (!db) {
        return 1;
    }
    
    fprintf(db, "%s|%s|%s|%s|%s\n", 
            pkg->name, pkg->version, pkg->description, 
            pkg->author, pkg->install_path);
    
    fclose(db);
    return 0;
}

// Remove package from installed database
int package_remove_from_database(const char* package_name) {
    char db_path[512];
    char temp_path[512];
    char* pkg_dir = wsys2_get_package_dir(0);
    
    snprintf(db_path, sizeof(db_path), "%s\\%s", pkg_dir, WSYS2_DATABASE_FILE);
    snprintf(temp_path, sizeof(temp_path), "%s\\%s.tmp", pkg_dir, WSYS2_DATABASE_FILE);
    
    FILE* db = fopen(db_path, "r");
    FILE* temp = fopen(temp_path, "w");
    
    if (!db || !temp) {
        if (db) fclose(db);
        if (temp) fclose(temp);
        return 1;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), db)) {
        char name[256];
        if (sscanf(line, "%255[^|]", name) == 1) {
            if (strcmp(name, package_name) != 0) {
                fputs(line, temp);
            }
        }
    }
    
    fclose(db);
    fclose(temp);
    
    // Replace original with temp
    DeleteFileA(db_path);
    MoveFileA(temp_path, db_path);
    
    return 0;
}

// Find an installed package
Package* package_find_installed(const char* package_name) {
    char db_path[512];
    char* pkg_dir = wsys2_get_package_dir(0);
    snprintf(db_path, sizeof(db_path), "%s\\%s", pkg_dir, WSYS2_DATABASE_FILE);
    
    FILE* db = fopen(db_path, "r");
    if (!db) {
        return NULL;
    }
    
    char line[1024];
    while (fgets(line, sizeof(line), db)) {
        Package* pkg = malloc(sizeof(Package));
        if (!pkg) continue;
        
        char* token = strtok(line, "|");
        if (!token) { free(pkg); continue; }
        strcpy(pkg->name, token);
        
        if (strcmp(pkg->name, package_name) == 0) {
            // Parse rest of the line
            token = strtok(NULL, "|");
            if (token) strcpy(pkg->version, token);
            
            token = strtok(NULL, "|");
            if (token) strcpy(pkg->description, token);
            
            token = strtok(NULL, "|");
            if (token) strcpy(pkg->author, token);
            
            token = strtok(NULL, "|\n");
            if (token) strcpy(pkg->install_path, token);
            
            pkg->installed = 1;
            fclose(db);
            return pkg;
        }
        
        free(pkg);
    }
    
    fclose(db);
    return NULL;
}

// List all installed packages
int package_list_installed(Package** packages, int* count) {
    char db_path[512];
    char* pkg_dir = wsys2_get_package_dir(0);
    snprintf(db_path, sizeof(db_path), "%s\\%s", pkg_dir, WSYS2_DATABASE_FILE);
    
    FILE* db = fopen(db_path, "r");
    if (!db) {
        *packages = NULL;
        *count = 0;
        return 0;
    }
    
    // Count lines first
    *count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), db)) {
        (*count)++;
    }
    
    if (*count == 0) {
        fclose(db);
        *packages = NULL;
        return 0;
    }
    
    // Allocate memory for packages
    *packages = malloc(sizeof(Package) * (*count));
    if (!*packages) {
        fclose(db);
        return 1;
    }
    
    // Read packages
    rewind(db);
    int i = 0;
    while (fgets(line, sizeof(line), db) && i < *count) {
        Package* pkg = &(*packages)[i];
        memset(pkg, 0, sizeof(Package));
        
        char* token = strtok(line, "|");
        if (token) strcpy(pkg->name, token);
        
        token = strtok(NULL, "|");
        if (token) strcpy(pkg->version, token);
        
        token = strtok(NULL, "|");
        if (token) strcpy(pkg->description, token);
        
        token = strtok(NULL, "|");
        if (token) strcpy(pkg->author, token);
        
        token = strtok(NULL, "|\n");
        if (token) strcpy(pkg->install_path, token);
        
        pkg->installed = 1;
        i++;
    }
    
    fclose(db);
    return 0;
}