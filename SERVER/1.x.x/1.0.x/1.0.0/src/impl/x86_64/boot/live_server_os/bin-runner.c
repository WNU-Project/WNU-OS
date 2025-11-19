#include "vga.h"
// This was made so the Binaries At SERVER\1.x.x\1.0.x\1.0.0\targets\x86_64\iso\usr\bin Can be Runned By The OS

#include <stdint.h>

// Define NULL for freestanding environment
#ifndef NULL
#define NULL ((void*)0)
#endif

// VGA output functions
extern void vga_puts(const char* str);

// WNU OS string functions (defined in wnu_shell.c)
extern int wnu_strcmp(const char* s1, const char* s2);
extern char* wnu_strcpy(char* dest, const char* src);
extern char* wnu_strcat(char* dest, const char* src);
extern int wnu_strncmp(const char* s1, const char* s2, int n);

// Filesystem interface prototypes - now using ISO 9660
extern int iso9660_binary_exists(const char* path);
extern int iso9660_execute_binary(const char* path, int argc, char* argv[]);
extern void iso9660_list_binaries(const char* directory_path);
extern int iso9660_init(void* cd_device);

// Stub implementations for ISO 9660 functions (to be replaced with actual implementation)
int iso9660_binary_exists(const char* path) {
    // TODO: Implement actual ISO 9660 file lookup
    (void)path; // Suppress unused parameter warning
    return 0; // For now, return "not found"
}

int iso9660_execute_binary(const char* path, int argc, char* argv[]) {
    // TODO: Implement actual binary execution from ISO 9660
    (void)path; (void)argc; (void)argv; // Suppress unused parameter warnings
    vga_puts("Error: Binary execution not yet implemented\n");
    return -1;
}

void iso9660_list_binaries(const char* directory_path) {
    // TODO: Implement actual directory listing from ISO 9660
    (void)directory_path; // Suppress unused parameter warning
    vga_puts("Directory listing not yet implemented\n");
}

int iso9660_init(void* cd_device) {
    // TODO: Implement actual ISO 9660 filesystem initialization
    (void)cd_device; // Suppress unused parameter warning
    vga_puts("ISO 9660 filesystem initialized (stub)\n");
    return 0;
}

// Standard binary search paths
static const char* binary_search_paths[] = {
    "/usr/bin/",
    "/bin/",
    "/usr/local/bin/",
    "/sbin/",
    "/usr/sbin/",
    NULL
};

// Binary runner for GNU Binutils on WNU OS
// Supports filesystem-based binary execution with path validation

// Structure to define available binaries with filesystem paths
typedef struct {
    const char* name;
    const char* full_name;
    const char* description;
    const char* expected_path;
    void (*handler)(int argc, char* argv[]);
} wnu_binary_t;

// Forward declarations for binary handlers
void run_assembler(int argc, char* argv[]);
void run_linker(int argc, char* argv[]);
void run_archiver(int argc, char* argv[]);
void run_objcopy(int argc, char* argv[]);
void run_objdump(int argc, char* argv[]);
void run_nm(int argc, char* argv[]);
void run_readelf(int argc, char* argv[]);
void run_strip(int argc, char* argv[]);
void run_size(int argc, char* argv[]);
void run_strings(int argc, char* argv[]);
void run_addr2line(int argc, char* argv[]);
void run_ranlib(int argc, char* argv[]);
void run_elfedit(int argc, char* argv[]);
void run_gprof(int argc, char* argv[]);
void run_cppfilt(int argc, char* argv[]);

// Registry of all available GNU Binutils with filesystem paths
static const wnu_binary_t wnu_binaries[] = {
    {"as", "x86_64-wnuos-elf-as", "GNU Assembler for x86_64 WNU OS", "/usr/bin/x86_64-wnuos-elf-as", run_assembler},
    {"ld", "x86_64-wnuos-elf-ld", "GNU Linker for x86_64 WNU OS", "/usr/bin/x86_64-wnuos-elf-ld", run_linker},
    {"ld.bfd", "x86_64-wnuos-elf-ld.bfd", "GNU Linker (BFD backend)", "/usr/bin/x86_64-wnuos-elf-ld.bfd", run_linker},
    {"ar", "x86_64-wnuos-elf-ar", "Archive manager", "/usr/bin/x86_64-wnuos-elf-ar", run_archiver},
    {"objcopy", "x86_64-wnuos-elf-objcopy", "Object file copier/converter", "/usr/bin/x86_64-wnuos-elf-objcopy", run_objcopy},
    {"objdump", "x86_64-wnuos-elf-objdump", "Object file disassembler", "/usr/bin/x86_64-wnuos-elf-objdump", run_objdump},
    {"nm", "x86_64-wnuos-elf-nm", "Symbol table lister", "/usr/bin/x86_64-wnuos-elf-nm", run_nm},
    {"readelf", "x86_64-wnuos-elf-readelf", "ELF file reader", "/usr/bin/x86_64-wnuos-elf-readelf", run_readelf},
    {"strip", "x86_64-wnuos-elf-strip", "Symbol stripper", "/usr/bin/x86_64-wnuos-elf-strip", run_strip},
    {"size", "x86_64-wnuos-elf-size", "Object file size reporter", "/usr/bin/x86_64-wnuos-elf-size", run_size},
    {"strings", "x86_64-wnuos-elf-strings", "String extractor", "/usr/bin/x86_64-wnuos-elf-strings", run_strings},
    {"addr2line", "x86_64-wnuos-elf-addr2line", "Address to line converter", "/usr/bin/x86_64-wnuos-elf-addr2line", run_addr2line},
    {"ranlib", "x86_64-wnuos-elf-ranlib", "Archive index generator", "/usr/bin/x86_64-wnuos-elf-ranlib", run_ranlib},
    {"elfedit", "x86_64-wnuos-elf-elfedit", "ELF file editor", "/usr/bin/x86_64-wnuos-elf-elfedit", run_elfedit},
    {"gprof", "x86_64-wnuos-elf-gprof", "GNU Profiler", "/usr/bin/x86_64-wnuos-elf-gprof", run_gprof},
    {"c++filt", "x86_64-wnuos-elf-c++filt", "C++ symbol demangler", "/usr/bin/x86_64-wnuos-elf-c++filt", run_cppfilt},
    {NULL, NULL, NULL, NULL, NULL}  // Sentinel
};

// Function to list all available binaries from filesystem
void list_binaries(void) {
    vga_puts("WNU OS Development Environment\n");
    vga_puts("===============================\n");
    vga_puts("Scanning filesystem for GNU Binutils...\n\n");
    
    // List binaries in common directories
    iso9660_list_binaries("/usr/bin");
    vga_puts("\n");
    iso9660_list_binaries("/bin");
    vga_puts("\n");
    
    vga_puts("Usage: run <binary-name> [args...]\n");
    vga_puts("   or: run <full-path> [args...]\n");
    vga_puts("Examples:\n");
    vga_puts("  run as -o kernel.o kernel.s\n");
    vga_puts("  run /usr/bin/x86_64-wnuos-elf-ld -T script.ld -o kernel kernel.o\n\n");
}

// Function to find a binary by name in filesystem
const char* find_binary_in_filesystem(const char* name) {
    // First check if it's already a full path
    if (name[0] == '/') {
        if (iso9660_binary_exists(name)) {
            return name;
        }
        return NULL;
    }
    
    // Search in our known binary list
    for (int i = 0; wnu_binaries[i].name != NULL; i++) {
        if (wnu_strcmp(wnu_binaries[i].name, name) == 0) {
            // Check the expected path first
            if (iso9660_binary_exists(wnu_binaries[i].expected_path)) {
                return wnu_binaries[i].expected_path;
            }
            
            // Try alternative paths
            for (int j = 0; binary_search_paths[j] != NULL; j++) {
                static char full_path[256];
                wnu_strcpy(full_path, binary_search_paths[j]);
                wnu_strcat(full_path, wnu_binaries[i].full_name);
                
                if (iso9660_binary_exists(full_path)) {
                    return full_path;
                }
        }
    }
    
    return NULL;  // Binary not found
}    return NULL;
}

// Main binary execution entry point with filesystem integration
int execute_binary(int argc, char* argv[]) {
    if (argc < 2) {
        vga_puts("WNU OS Binary Runner with ISO 9660 Filesystem\n");
        vga_puts("GNU Binutils 2.43.1 for x86_64-wnuos-elf\n\n");
        list_binaries();
        return 1;
    }
    
    // Find the binary in the filesystem
    const char* binary_path = find_binary_in_filesystem(argv[1]);
    
    if (binary_path == NULL) {
        vga_puts("Error: Binary '");
        vga_puts(argv[1]);
        vga_puts("' not found in filesystem.\n");
        vga_puts("Use 'run' without arguments to see available binaries.\n");
        return 1;
    }
    
    // Validate path starts with allowed directories
    if (wnu_strncmp(binary_path, "/usr/bin/", 9) != 0 && 
        wnu_strncmp(binary_path, "/bin/", 5) != 0) {
        vga_puts("Error: Binary path must start with /usr/bin or /bin\n");
        vga_puts("Found path: ");
        vga_puts(binary_path);
        vga_puts("\n");
        return 1;
    }
    
    // Execute the binary from filesystem
    vga_puts("Executing: ");
    vga_puts(binary_path);
    vga_puts("\n");
    
    return iso9660_execute_binary(binary_path, argc - 1, &argv[1]);
}

// Initialize the binary runner system
int init_binary_runner(void* cd_device) {
    vga_puts("Initializing WNU OS Binary Runner...\n");
    
    // Initialize the ISO 9660 filesystem
    if (iso9660_init(cd_device) != 0) {
        vga_puts("Error: Failed to initialize ISO 9660 filesystem\n");
        return -1;
    }
    
    vga_puts("Binary Runner initialized successfully\n");
    vga_puts("GNU development tools available from filesystem\n\n");
    
    return 0;
}

