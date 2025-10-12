#include "wsys2.h"
#include <direct.h>

// Parse a .wnupkg file and extract package information
int package_parse_wnupkg(const char* wnupkg_file, Package* pkg) {
    FILE* file = fopen(wnupkg_file, "rb");
    if (!file) {
        return 1;
    }
    
    // Initialize package structure
    memset(pkg, 0, sizeof(Package));
    
    // Read file into buffer to handle both text and binary sections
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return 1;
    }
    
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    
    // Find manifest section
    char* manifest_start = strstr(buffer, "[MANIFEST]");
    if (!manifest_start) {
        free(buffer);
        return 1;
    }
    
    // Find end of manifest (next section or FILES section)
    char* manifest_end = strstr(manifest_start + 10, "[");
    if (!manifest_end) {
        manifest_end = buffer + file_size;
    }
    
    // Parse manifest line by line
    char* line_start = manifest_start;
    while (line_start < manifest_end) {
        char* line_end = strchr(line_start, '\n');
        if (!line_end) break;
        
        // Extract line
        size_t line_len = line_end - line_start;
        char line[1024];
        if (line_len >= sizeof(line)) line_len = sizeof(line) - 1;
        strncpy(line, line_start, line_len);
        line[line_len] = '\0';
        
        // Remove carriage return
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';
        
        // Skip section header and empty lines
        if (line[0] == '[' || line[0] == '\0' || line[0] == '#') {
            line_start = line_end + 1;
            continue;
        }
        
        // Parse key=value pairs
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
        
        line_start = line_end + 1;
    }
    
    free(buffer);
    
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
    
    // Open the .wnupkg file in binary mode
    FILE* wnupkg = fopen(source_file, "rb");
    if (!wnupkg) {
        printf("  Error: Could not open package file\n");
        return 1;
    }
    
    // Read the entire file as binary data
    fseek(wnupkg, 0, SEEK_END);
    long file_size = ftell(wnupkg);
    fseek(wnupkg, 0, SEEK_SET);
    
    unsigned char* buffer = malloc(file_size);
    if (!buffer) {
        fclose(wnupkg);
        return 1;
    }
    
    fread(buffer, 1, file_size, wnupkg);
    fclose(wnupkg);
    
    // Find [FILES] section using binary search
    unsigned char* files_start = NULL;
    for (long i = 0; i <= file_size - 7; i++) {
        if (memcmp(buffer + i, "[FILES]", 7) == 0) {
            files_start = buffer + i;
            break;
        }
    }
    
    if (!files_start) {
        printf("  Warning: No [FILES] section found in package\n");
        free(buffer);
        return 0;
    }
    
    // Skip to after [FILES] line
    unsigned char* current_pos = files_start + 7;
    while (current_pos < buffer + file_size && (*current_pos == '\r' || *current_pos == '\n')) {
        current_pos++;
    }
    
    unsigned char* end_pos = buffer + file_size;
    
    int file_count = 0;
    while (current_pos < end_pos) {
        // Look for "FILE:" marker using binary search
        unsigned char* file_marker = NULL;
        for (unsigned char* p = current_pos; p <= end_pos - 5; p++) {
            if (memcmp(p, "FILE:", 5) == 0) {
                file_marker = p;
                break;
            }
        }
        
        if (!file_marker) break;
        
        file_count++;
        
        // Extract filename (skip "FILE:")
        unsigned char* filename_start = file_marker + 5;
        unsigned char* filename_end = filename_start;
        
        // Find end of filename line
        while (filename_end < end_pos && *filename_end != '\r' && *filename_end != '\n') {
            filename_end++;
        }
        
        if (filename_end >= end_pos) break;
        
        // Copy filename
        char filename[512];
        size_t filename_len = filename_end - filename_start;
        if (filename_len >= sizeof(filename)) filename_len = sizeof(filename) - 1;
        memcpy(filename, filename_start, filename_len);
        filename[filename_len] = '\0';
        
        printf("  Installing: %s\n", filename);
        
        // Create full destination path
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s\\%s", dest_dir, filename);
        
        // Create directories recursively
        char dir_path[1024];
        strcpy(dir_path, full_path);
        char* last_slash = strrchr(dir_path, '\\');
        if (last_slash) {
            *last_slash = '\0';
            
            // Create each directory level manually
            char temp_path[1024];
            strcpy(temp_path, dir_path);
            char working_path[1024];
            working_path[0] = '\0';
            
            char* token = strtok(temp_path, "\\");
            while (token != NULL) {
                if (strlen(working_path) > 0) {
                    strcat(working_path, "\\");
                }
                strcat(working_path, token);
                _mkdir(working_path);
                token = strtok(NULL, "\\");
            }
        }
        
        // Skip to start of binary data (skip exactly one newline sequence after filename)
        unsigned char* binary_start = filename_end;
        
        // Skip exactly one newline sequence (either \r\n or \n)
        if (binary_start < end_pos && *binary_start == '\r') {
            binary_start++; // Skip \r
        }
        if (binary_start < end_pos && *binary_start == '\n') {
            binary_start++; // Skip \n
        }
        
        // Don't skip any additional whitespace - binary data starts immediately
        
        // Look for the end of binary data by finding the NEXT FILE: marker or section markers
        unsigned char* binary_end = end_pos;
        
        // Search for next FILE: marker first (indicates another file)
        for (unsigned char* p = binary_start; p <= end_pos - 5; p++) {
            if (memcmp(p, "FILE:", 5) == 0) {
                // Found another file, binary data ends at previous newline
                unsigned char* newline_before = p - 1;
                while (newline_before > binary_start && (*newline_before == '\r' || *newline_before == '\n')) {
                    newline_before--;
                }
                binary_end = newline_before + 1;
                break;
            }
        }
        
        // If no next FILE: found, look for section markers
        if (binary_end == end_pos) {
            for (unsigned char* p = binary_start; p <= end_pos - 9; p++) {
                // Look for newline followed by section markers
                if (*p == '\n') {
                    if (p <= end_pos - 10 && memcmp(p + 1, "[INSTALL]", 9) == 0) {
                        binary_end = p;
                        break;
                    }
                    if (p <= end_pos - 12 && memcmp(p + 1, "[UNINSTALL]", 11) == 0) {
                        binary_end = p;
                        break;
                    }
                }
            }
        }
        
        // Remove trailing newlines from binary data
        while (binary_end > binary_start && (*(binary_end-1) == '\n' || *(binary_end-1) == '\r')) {
            binary_end--;
        }
        
        size_t binary_length = binary_end - binary_start;
        
        // Write binary data to file
        FILE* output_file = fopen(full_path, "wb");
        if (output_file) {
            size_t written = fwrite(binary_start, 1, binary_length, output_file);
            fclose(output_file);
    printf("    Extracted %zu bytes", written);
            
            // Debug: show first few bytes to verify PE header
            if (binary_length >= 6) {
                printf(" (starts with: %02X %02X %02X %02X %02X %02X", 
                       binary_start[0], binary_start[1], binary_start[2], 
                       binary_start[3], binary_start[4], binary_start[5]);
                printf(")");
            }
            printf("\n");
        } else {
            printf("    Error: Could not create %s\n", full_path);
        }
        
        // Move to next file - continue searching from after the current binary data
        current_pos = binary_end;
        
        // Skip any section markers we might encounter (but continue looking for more FILES)
        while (current_pos < end_pos) {
            // Skip over section markers like [INSTALL] or [UNINSTALL]
            if (*current_pos == '[') {
                // Find end of section line
                while (current_pos < end_pos && *current_pos != '\n') {
                    current_pos++;
                }
                if (current_pos < end_pos) current_pos++; // Skip the newline
            } else if (*current_pos == '\r' || *current_pos == '\n') {
                current_pos++; // Skip whitespace
            } else {
                break; // Found content, continue looking for FILE: markers
            }
        }
    }
    
    printf("  Total files processed: %d\n", file_count);
    
    free(buffer);
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