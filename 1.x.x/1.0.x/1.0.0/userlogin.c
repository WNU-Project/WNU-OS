#include <stdio.h>       // for printf, scanf
#include <string.h>      // for strcmp, strcpy, snprintf
#include <windows.h>     // for GetUserNameA, Set environment, Sleep
#include <lmcons.h>      // for UNLEN (max username length)
#include <direct.h>      // for _chdir
#include "userlogin.h" // for user_login

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <lmcons.h>   // for UNLEN
#include <direct.h>   // for _chdir

void user_login(const char* username, const char* password) {
    char actual_user[UNLEN + 1];
    DWORD user_len = UNLEN + 1;

    // Get current Windows username
    if (!GetUserNameA(actual_user, &user_len)) {
        fprintf(stderr, "Failed to retrieve current user\n");
        return;
    }

    // Compare input username to actual Windows username
    if (strcmp(username, actual_user) == 0) {
        printf("Welcome, %s! Entering normal user mode...\n", username);

        // Set home directory to C:\Users\{username}
        char home_path[256];
        snprintf(home_path, sizeof(home_path), "C:\\Users\\%s", username);
        if (_chdir(home_path) != 0) {
            perror("Failed to set home directory");
        } else {
            printf("Home directory set to: %s\n", home_path);
        }

        // You can now launch your normal shell loop here
        // launch_normal_user_shell(username, home_path);
    } else {
        printf("Access denied for user: %s\n", username);
    }
}
