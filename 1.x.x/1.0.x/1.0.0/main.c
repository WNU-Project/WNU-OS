#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>  // for _getcwd and _chdir
#include "boot.h"

int main(void) {
    char username[100];
    char computername[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computername);

    // Get computer name
    if (!GetComputerNameA(computername, &size)) {
        strcpy(computername, "unknown");
    }

    // Run boot sequence
    boot_sequence();

    // --- Login screen ---
    printf("WNU OS 1.0.0\n");
    printf("%s login: ", computername);

    if (scanf("%99s", username) != 1) {
        fprintf(stderr, "No username entered\n");
        return 1;
    }

    if (strcmp(username, "root") != 0) {
        printf("Access denied for user: %s\n", username);
        return 1;
    }

    printf("Welcome, root!\n");
    _chdir("C:\\");

    // --- Shell loop ---
    while (1) {
        char cwd[1024];
        if (!_getcwd(cwd, sizeof(cwd))) {
            strcpy(cwd, "?");
        }

        // Replace "C:\" with "~" and convert backslashes to slashes
        char display_path[1024];
        if (strncmp(cwd, "C:\\", 3) == 0) {
            snprintf(display_path, sizeof(display_path), "~/%s", cwd + 3);
        } else {
            strncpy(display_path, cwd, sizeof(display_path));
        }

        for (int i = 0; display_path[i]; ++i) {
            if (display_path[i] == '\\') {
                display_path[i] = '/';
            }
        }

        // Print prompt
        printf("%s@root#:%s ", computername, display_path);

        // Read command
        char command[256];
        if (!fgets(command, sizeof(command), stdin)) {
            break; // EOF
        }

        // Strip newline
        command[strcspn(command, "\n")] = 0;

        // Handle built-in commands
        if (strcmp(command, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        } else if (strncmp(command, "cd ", 3) == 0) {
            if (_chdir(command + 3) != 0) {
                perror("cd failed");
            }
        } else if (strlen(command) > 0) {
            system(command); // Run external command
        }
    }

    return 0;
}
