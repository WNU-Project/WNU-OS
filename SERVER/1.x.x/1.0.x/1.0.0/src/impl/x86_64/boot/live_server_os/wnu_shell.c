/*
 * WNU OS Advanced Shell Implementation
 * Modern shell with history, tab completion, and built-in apps
 * Copyright (c) 2025 WNU Project
 */

#include "wnu_shell.h"
#include "vga.h"

// Global shell state
static shell_state_t shell_state;

// Built-in commands table
static shell_command_t builtin_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"clear", "Clear the screen", cmd_clear},
    {"history", "Show command history", cmd_history},
    {"ls", "List directory contents", cmd_ls},
    {"dir", "List directory contents", cmd_ls},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"mkdir", "Create directory", cmd_mkdir},
    {"rmdir", "Remove directory", cmd_rmdir},
    {"cp", "Copy files", cmd_cp},
    {"copy", "Copy files", cmd_cp},
    {"mv", "Move/rename files", cmd_mv},
    {"move", "Move/rename files", cmd_mv},
    {"rm", "Remove files", cmd_rm},
    {"del", "Remove files", cmd_rm},
    {"cat", "Display file contents", cmd_cat},
    {"type", "Display file contents", cmd_cat},
    {"echo", "Display text", cmd_echo},
    {"whoami", "Show current user", cmd_whoami},
    {"uname", "Show system information", cmd_uname},
    {"date", "Show current date/time", cmd_date},
    {"uptime", "Show system uptime", cmd_uptime},
    {"ps", "Show running processes", cmd_ps},
    {"kill", "Terminate process", cmd_kill},
    {"apps", "List available applications", cmd_apps},
    {"run", "Run an application", cmd_run},
    {"calc", "Launch calculator", app_calc},
    {"calculator", "Launch calculator", app_calc},
    {"edit", "Launch text editor", app_edit},
    {"notepad", "Launch text editor", app_edit},
    {"files", "Launch file manager", app_filemanager},
    {"fm", "Launch file manager", app_filemanager},
    {"sysinfo", "Show detailed system info", app_sysinfo},
    {"hexdump", "Hex dump of file/memory", app_hexdump},
    {"memtest", "Memory test utility", app_memtest},
    {"shutdown", "Shutdown system", cmd_shutdown},
    {"poweroff", "Shutdown system", cmd_shutdown},
    {"reboot", "Restart system", cmd_reboot},
    {"restart", "Restart system", cmd_reboot},
    {NULL, NULL, NULL} // Terminator
};

// Built-in applications registry
static wnu_app_t builtin_apps[] = {
    {"Calculator", "1.0", "Scientific calculator with memory", app_calc},
    {"Text Editor", "1.0", "Simple text editor with syntax highlighting", app_edit},
    {"File Manager", "1.0", "Graphical file browser with operations", app_filemanager},
    {"System Info", "1.0", "Detailed hardware and software information", app_sysinfo},
    {"Hex Dump", "1.0", "Binary file viewer and memory inspector", app_hexdump},
    {"Memory Test", "1.0", "RAM testing and benchmarking utility", app_memtest},
    {NULL, NULL, NULL, NULL} // Terminator
};

// Simple string functions for shell use
static int wnu_strlen(const char* str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

static int wnu_strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static int wnu_strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

static char* wnu_strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

static char* wnu_strcat(char* dest, const char* src) {
    char* d = dest + wnu_strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

// Initialize the shell system
void shell_init(void) {
    // Initialize shell state
    shell_state.input_buffer[0] = '\0';
    shell_state.cursor_position = 0;
    shell_state.history_index = 0;
    shell_state.history_count = 0;
    shell_state.running = 1;
    
    // Set initial directory
    wnu_strcpy(shell_state.current_directory, "/");
    
    // Set initial prompt
    wnu_strcpy(shell_state.prompt, "root@live:~# ");
    
    // Clear command history
    for (int i = 0; i < SHELL_HISTORY_SIZE; i++) {
        shell_state.command_history[i][0] = '\0';
    }
    
    shell_print_success("WNU OS Advanced Shell v2.0 initialized!\n");
    shell_print_info("Type 'help' for available commands or 'apps' for applications.\n");
}

// Main shell loop
void shell_run(void) {
    while (shell_state.running) {
        shell_print_prompt();
        
        char* line = shell_readline();
        if (line && line[0] != '\0') {
            shell_add_to_history(line);
            shell_execute_command(line);
        }
    }
}

// Print the shell prompt
void shell_print_prompt(void) {
    shell_print_color(shell_state.prompt, VGA_COLOR_GREEN);
}

// Read a line of input from user
char* shell_readline(void) {
    shell_state.cursor_position = 0;
    shell_state.input_buffer[0] = '\0';
    
    char c;
    while (1) {
        // Get character from keyboard (this would need keyboard input integration)
        // For now, simulate with a simple command
        // c = keyboard_getchar();
        
        if (c == '\n' || c == '\r') {
            shell_print("\n");
            shell_state.input_buffer[shell_state.cursor_position] = '\0';
            return shell_state.input_buffer;
        }
        
        if (c == '\b') { // Backspace
            if (shell_state.cursor_position > 0) {
                shell_state.cursor_position--;
                shell_print("\b \b"); // Erase character on screen
            }
            continue;
        }
        
        if (c == '\t') { // Tab completion
            shell_tab_completion();
            continue;
        }
        
        if (c >= 32 && c <= 126) { // Printable characters
            if (shell_state.cursor_position < SHELL_BUFFER_SIZE - 1) {
                shell_state.input_buffer[shell_state.cursor_position++] = c;
                char str[2] = {c, '\0'};
                shell_print(str);
            }
        }
    }
}

// Execute a command
void shell_execute_command(char* command) {
    if (!command || command[0] == '\0') {
        return;
    }
    
    // Tokenize the command
    char** args = shell_tokenize(command);
    if (!args || !args[0]) {
        shell_free_tokens(args);
        return;
    }
    
    // Try built-in commands first
    if (shell_execute_builtin(args)) {
        shell_free_tokens(args);
        return;
    }
    
    // Try built-in applications
    if (shell_execute_app(args)) {
        shell_free_tokens(args);
        return;
    }
    
    // Command not found
    shell_print_error("WNU-SH: command not found: ");
    shell_print(args[0]);
    shell_print("\n");
    shell_print_info("Type 'help' for available commands.\n");
    
    shell_free_tokens(args);
}

// Execute built-in command
int shell_execute_builtin(char** args) {
    if (!args || !args[0]) return 0;
    
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (wnu_strcmp(args[0], builtin_commands[i].name) == 0) {
            // Count arguments
            int argc = 0;
            while (args[argc] != NULL) argc++;
            
            // Execute command
            int result = builtin_commands[i].handler(argc, args);
            return 1; // Command was found and executed
        }
    }
    
    return 0; // Command not found
}

// Execute built-in application
int shell_execute_app(char** args) {
    if (!args || !args[0]) return 0;
    
    for (int i = 0; builtin_apps[i].name != NULL; i++) {
        if (wnu_strcmp(args[0], builtin_apps[i].name) == 0) {
            // Count arguments
            int argc = 0;
            while (args[argc] != NULL) argc++;
            
            shell_print_info("Launching ");
            shell_print(builtin_apps[i].name);
            shell_print("...\n");
            
            // Execute application
            int result = builtin_apps[i].main_func(argc, args);
            return 1; // App was found and executed
        }
    }
    
    return 0; // App not found
}

// Add command to history
void shell_add_to_history(char* command) {
    if (!command || command[0] == '\0') return;
    
    // Add to circular buffer
    wnu_strcpy(shell_state.command_history[shell_state.history_count % SHELL_HISTORY_SIZE], command);
    shell_state.history_count++;
    shell_state.history_index = shell_state.history_count;
}

// Tab completion implementation
void shell_tab_completion(void) {
    char* partial = shell_state.input_buffer;
    int matches = 0;
    char* match = NULL;
    
    // Look for matching commands
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (wnu_strncmp(partial, builtin_commands[i].name, wnu_strlen(partial)) == 0) {
            matches++;
            match = builtin_commands[i].name;
            if (matches == 1) {
                // First match - complete it
                wnu_strcpy(shell_state.input_buffer, match);
                shell_state.cursor_position = wnu_strlen(match);
                shell_clear_line();
                shell_print_prompt();
                shell_print(shell_state.input_buffer);
            } else if (matches == 2) {
                // Multiple matches - show first one
                shell_print("\n");
                shell_print(builtin_commands[i-1].name);
                shell_print("  ");
                shell_print(match);
            } else {
                // Additional matches
                shell_print("  ");
                shell_print(match);
            }
        }
    }
    
    if (matches > 1) {
        shell_print("\n");
        shell_print_prompt();
        shell_print(shell_state.input_buffer);
    }
}

// Clear current line
void shell_clear_line(void) {
    // Move cursor to beginning and clear line
    shell_print("\r");
    for (int i = 0; i < 80; i++) {
        shell_print(" ");
    }
    shell_print("\r");
}

// Print functions using VGA output
void shell_print(const char* str) {
    while (*str) {
        vga_print(0, VGA_COLOR_WHITE, str); // Use VGA print function
        str++;
    }
}

void shell_print_color(const char* str, uint8_t color) {
    while (*str) {
        vga_print(0, color, str);
        str++;
    }
}

void shell_print_success(const char* str) {
    shell_print_color(str, VGA_COLOR_GREEN);
}

void shell_print_error(const char* str) {
    shell_print_color(str, VGA_COLOR_RED);
}

void shell_print_warning(const char* str) {
    shell_print_color(str, VGA_COLOR_YELLOW);
}

void shell_print_info(const char* str) {
    shell_print_color(str, VGA_COLOR_CYAN);
}

// Simple tokenizer
char** shell_tokenize(char* line) {
    static char* tokens[SHELL_MAX_ARGS];
    int position = 0;
    char* token;
    
    // Simple space-based tokenization
    token = line;
    tokens[position] = token;
    
    while (*line) {
        if (*line == ' ') {
            *line = '\0';
            line++;
            while (*line == ' ') line++; // Skip multiple spaces
            if (*line != '\0') {
                tokens[++position] = line;
            }
        } else {
            line++;
        }
    }
    
    tokens[position + 1] = NULL;
    return tokens;
}

void shell_free_tokens(char** tokens) {
    // Nothing to free for our simple implementation
}

// Built-in commands implementation

int cmd_help(int argc, char* argv[]) {
    shell_print_info("WNU OS Advanced Shell Commands:\n\n");
    
    shell_print_color("File Operations:\n", VGA_COLOR_YELLOW);
    shell_print("  ls, dir     - List directory contents\n");
    shell_print("  cd          - Change directory\n");
    shell_print("  pwd         - Print working directory\n");
    shell_print("  mkdir       - Create directory\n");
    shell_print("  rmdir       - Remove directory\n");
    shell_print("  cp, copy    - Copy files\n");
    shell_print("  mv, move    - Move/rename files\n");
    shell_print("  rm, del     - Remove files\n");
    shell_print("  cat, type   - Display file contents\n");
    
    shell_print_color("\nSystem Commands:\n", VGA_COLOR_YELLOW);
    shell_print("  clear       - Clear screen\n");
    shell_print("  history     - Show command history\n");
    shell_print("  whoami      - Show current user\n");
    shell_print("  uname       - Show system information\n");
    shell_print("  date        - Show current date/time\n");
    shell_print("  uptime      - Show system uptime\n");
    shell_print("  ps          - Show running processes\n");
    shell_print("  shutdown    - Shutdown system\n");
    shell_print("  reboot      - Restart system\n");
    
    shell_print_color("\nBuilt-in Applications:\n", VGA_COLOR_YELLOW);
    shell_print("  calc        - Scientific calculator\n");
    shell_print("  edit        - Text editor\n");
    shell_print("  files       - File manager\n");
    shell_print("  sysinfo     - Detailed system info\n");
    shell_print("  hexdump     - Binary file viewer\n");
    shell_print("  memtest     - Memory testing utility\n");
    
    shell_print_color("\nTips:\n", VGA_COLOR_CYAN);
    shell_print("  - Use TAB for command completion\n");
    shell_print("  - Use UP/DOWN arrows for history\n");
    shell_print("  - Type 'apps' to see all applications\n");
    
    return 0;
}

int cmd_clear(int argc, char* argv[]) {
    // Clear screen using VGA functions
    for (int i = 0; i < 2000; i++) {
        // Clear VGA text buffer
        volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
        vga_buffer[i] = 0x0720; // Space with normal color
    }
    return 0;
}

int cmd_history(int argc, char* argv[]) {
    shell_print_info("Command History:\n");
    
    int start = (shell_state.history_count > SHELL_HISTORY_SIZE) ? 
                 shell_state.history_count - SHELL_HISTORY_SIZE : 0;
    
    for (int i = start; i < shell_state.history_count; i++) {
        shell_printf("%3d  %s\n", i + 1, 
                     shell_state.command_history[i % SHELL_HISTORY_SIZE]);
    }
    
    return 0;
}

int cmd_apps(int argc, char* argv[]) {
    shell_print_info("Available Applications:\n\n");
    
    for (int i = 0; builtin_apps[i].name != NULL; i++) {
        shell_print_color(builtin_apps[i].name, VGA_COLOR_YELLOW);
        shell_print(" v");
        shell_print(builtin_apps[i].version);
        shell_print(" - ");
        shell_print(builtin_apps[i].description);
        shell_print("\n");
    }
    
    shell_print("\nUse 'run <appname>' or just '<appname>' to launch.\n");
    return 0;
}

// Placeholder implementations for other commands
int cmd_ls(int argc, char* argv[]) {
    shell_print_info("Directory listing:\n");
    shell_print("README.md\n");
    shell_print("CHANGELOG.md\n");
    shell_print("LICENSE.txt\n");
    shell_print("1.x.x/\n");
    shell_print("packages/\n");
    shell_print("SERVER/\n");
    return 0;
}

int cmd_pwd(int argc, char* argv[]) {
    shell_print(shell_state.current_directory);
    shell_print("\n");
    return 0;
}

int cmd_whoami(int argc, char* argv[]) {
    shell_print("root\n");
    return 0;
}

int cmd_uname(int argc, char* argv[]) {
    shell_print("WNU OS SERVER 1.0.0.11.15.2025 X86_64 ASM C\n");
    return 0;
}

// Simple printf implementation
void shell_printf(const char* format, ...) {
    // For now, just print the format string
    // A real implementation would handle format specifiers
    shell_print(format);
}

// Stub implementations for remaining commands and apps
int cmd_cd(int argc, char* argv[]) { return 0; }
int cmd_mkdir(int argc, char* argv[]) { return 0; }
int cmd_rmdir(int argc, char* argv[]) { return 0; }
int cmd_cp(int argc, char* argv[]) { return 0; }
int cmd_mv(int argc, char* argv[]) { return 0; }
int cmd_rm(int argc, char* argv[]) { return 0; }
int cmd_cat(int argc, char* argv[]) { return 0; }
int cmd_echo(int argc, char* argv[]) { return 0; }
int cmd_date(int argc, char* argv[]) { return 0; }
int cmd_uptime(int argc, char* argv[]) { return 0; }
int cmd_ps(int argc, char* argv[]) { return 0; }
int cmd_kill(int argc, char* argv[]) { return 0; }
int cmd_run(int argc, char* argv[]) { return 0; }
int cmd_shutdown(int argc, char* argv[]) { return 0; }
int cmd_reboot(int argc, char* argv[]) { return 0; }