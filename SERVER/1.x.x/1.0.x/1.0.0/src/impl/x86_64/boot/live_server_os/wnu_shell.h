/*
 * WNU OS Advanced Shell System
 * Modern shell interface with command history, tab completion, and apps
 * Copyright (c) 2025 WNU Project
 */

#ifndef WNU_SHELL_H
#define WNU_SHELL_H

#include <stdint.h>
#include <stddef.h>

// Shell configuration
#define SHELL_BUFFER_SIZE 256
#define SHELL_HISTORY_SIZE 16
#define SHELL_MAX_ARGS 32
#define SHELL_PROMPT_SIZE 64

// Command structure
typedef struct {
    char* name;
    char* description;
    int (*handler)(int argc, char* argv[]);
} shell_command_t;

// Shell state structure
typedef struct {
    char input_buffer[SHELL_BUFFER_SIZE];
    char command_history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
    char current_directory[256];
    char prompt[SHELL_PROMPT_SIZE];
    int history_index;
    int history_count;
    int cursor_position;
    int running;
} shell_state_t;

// Application structure
typedef struct {
    char* name;
    char* version;
    char* description;
    int (*main_func)(int argc, char* argv[]);
} wnu_app_t;

// Shell functions
void shell_init(void);
void shell_run(void);
void shell_print_prompt(void);
void shell_process_input(char c);
void shell_execute_command(char* command);
void shell_add_to_history(char* command);
void shell_show_history(void);
void shell_tab_completion(void);
void shell_clear_line(void);

// Built-in command functions
int cmd_help(int argc, char* argv[]);
int cmd_clear(int argc, char* argv[]);
int cmd_history(int argc, char* argv[]);
int cmd_ls(int argc, char* argv[]);
int cmd_cd(int argc, char* argv[]);
int cmd_pwd(int argc, char* argv[]);
int cmd_mkdir(int argc, char* argv[]);
int cmd_rmdir(int argc, char* argv[]);
int cmd_cp(int argc, char* argv[]);
int cmd_mv(int argc, char* argv[]);
int cmd_rm(int argc, char* argv[]);
int cmd_cat(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_whoami(int argc, char* argv[]);
int cmd_uname(int argc, char* argv[]);
int cmd_date(int argc, char* argv[]);
int cmd_uptime(int argc, char* argv[]);
int cmd_ps(int argc, char* argv[]);
int cmd_kill(int argc, char* argv[]);
int cmd_apps(int argc, char* argv[]);
int cmd_run(int argc, char* argv[]);
int cmd_shutdown(int argc, char* argv[]);
int cmd_reboot(int argc, char* argv[]);

// Built-in applications
int app_calc(int argc, char* argv[]);
int app_edit(int argc, char* argv[]);
int app_filemanager(int argc, char* argv[]);
int app_sysinfo(int argc, char* argv[]);
int app_hexdump(int argc, char* argv[]);
int app_memtest(int argc, char* argv[]);

// Utility functions
void shell_print(const char* str);
void shell_printf(const char* format, ...);
char* shell_readline(void);
char** shell_tokenize(char* line);
void shell_free_tokens(char** tokens);
int shell_execute_builtin(char** args);
int shell_execute_app(char** args);

// Color output functions
void shell_print_color(const char* str, uint8_t color);
void shell_print_success(const char* str);
void shell_print_error(const char* str);
void shell_print_warning(const char* str);
void shell_print_info(const char* str);

// File system helpers
int shell_file_exists(const char* path);
int shell_is_directory(const char* path);
long shell_get_file_size(const char* path);
int shell_list_directory(const char* path);

#endif // WNU_SHELL_H