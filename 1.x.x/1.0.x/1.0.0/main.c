#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>   // for _getcwd

int main(void) {
    char username[100];
    char computername[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computername);

    // Get computer name
    if (!GetComputerNameA(computername, &size)) {
        strcpy(computername, "unknown");
    }

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

    // --- Fake shell loop ---
    while (1) {
        char cwd[1024];
        if (!_getcwd(cwd, sizeof(cwd))) {
            strcpy(cwd, "?");
        }

        // Print prompt
        printf("%s@root#:%s ", computername, cwd);

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
            // Run external command
            system(command);
        }
    }

    return 0;
}
