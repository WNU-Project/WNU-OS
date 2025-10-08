#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>  // for _getcwd and _chdir
#include <lmcons.h>  // for UNLEN
#include "boot.h"
#include "userlogin.h"

// Define the global variable
int isnormaluser = 0;

// Function to verify PIN using PowerShell Windows authentication
int verify_windows_hello_pin(const char* username, const char* pin) {
    printf("Attempting Windows Hello PIN verification via PowerShell...\n");
    
    // Create PowerShell command to test credentials with execution policy bypass
    char ps_command[1024];
    snprintf(ps_command, sizeof(ps_command),
        "powershell -ExecutionPolicy Bypass -NoProfile -WindowStyle Hidden -Command \""
        "try { "
        "$secpass = ConvertTo-SecureString '%s' -AsPlainText -Force; "
        "$cred = New-Object System.Management.Automation.PSCredential('%s', $secpass); "
        "$result = Invoke-Command -ComputerName localhost -Credential $cred -ScriptBlock { $env:USERNAME } -ErrorAction Stop; "
        "if ($result -eq '%s') { exit 0 } else { exit 1 } "
        "} catch { exit 1 }\"",
        pin, username, username);
    
    // Execute PowerShell command
    int result = system(ps_command);
    
    if (result == 0) {
        printf("✓ Windows Hello PIN verification successful!\n");
        return 1;
    }
    
    printf("× PowerShell PIN verification failed\n");
    return 0;
}

// Function to verify PIN using runas command
int verify_pin_with_runas(const char* username, const char* pin) {
    printf("Attempting PIN verification via runas command...\n");
    
    // Create a temporary batch file for runas test
    char temp_path[256];
    char batch_file[300];
    GetTempPathA(sizeof(temp_path), temp_path);
    snprintf(batch_file, sizeof(batch_file), "%stemp_runas_test.bat", temp_path);
    
    FILE* bat = fopen(batch_file, "w");
    if (!bat) {
        printf("Could not create temporary batch file\n");
        return 0;
    }
    
    // Write batch commands to test PIN
    fprintf(bat, "@echo off\n");
    fprintf(bat, "echo %s| runas /user:%s \"cmd /c echo SUCCESS > %stemp_pin_result.txt\" >nul 2>&1\n", 
            pin, username, temp_path);
    fprintf(bat, "timeout /t 2 /nobreak >nul\n");
    fclose(bat);
    
    // Execute the batch file
    system(batch_file);
    
    // Check if the result file was created (indicates successful authentication)
    char result_file[300];
    snprintf(result_file, sizeof(result_file), "%stemp_pin_result.txt", temp_path);
    
    FILE* result = fopen(result_file, "r");
    int success = (result != NULL);
    
    // Cleanup
    if (result) fclose(result);
    DeleteFileA(batch_file);
    DeleteFileA(result_file);
    
    if (success) {
        printf("✓ PIN verified via runas command!\n");
        return 1;
    }
    
    return 0;
}

// Alternative PIN verification using Windows Security
int verify_pin_with_logon_session(const char* username, const char* pin) {
    printf("Attempting PIN verification via logon session...\n");
    
    // Try to create a logon session with the PIN
    HANDLE hToken = NULL;
    
    // Method 1: Try with Windows Hello PIN logon type
    if (LogonUserA((LPSTR)username, ".", (LPSTR)pin, LOGON32_LOGON_UNLOCK, 
                   LOGON32_PROVIDER_WINNT50, &hToken)) {
        CloseHandle(hToken);
        printf("✓ PIN verified via WINNT50 provider!\n");
        return 1;
    }
    
    // Method 2: Try with credential provider
    if (LogonUserA((LPSTR)username, ".", (LPSTR)pin, LOGON32_LOGON_INTERACTIVE, 
                   LOGON32_PROVIDER_WINNT50, &hToken)) {
        CloseHandle(hToken);
        printf("✓ PIN verified via interactive logon!\n");
        return 1;
    }
    
    // Method 3: Try network logon with WINNT50
    if (LogonUserA((LPSTR)username, ".", (LPSTR)pin, LOGON32_LOGON_NETWORK, 
                   LOGON32_PROVIDER_WINNT50, &hToken)) {
        CloseHandle(hToken);
        printf("✓ PIN verified via network logon!\n");
        return 1;
    }
    
    return 0;
}

// Function to check if credential looks like a PIN
int is_pin_format(const char* credential) {
    size_t len = strlen(credential);
    if (len < 4 || len > 6) return 0;
    
    // Check if all characters are digits OR alphanumeric PIN like "COOL1"
    int has_digit = 0, has_alpha = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (credential[i] >= '0' && credential[i] <= '9') {
            has_digit = 1;
        } else if ((credential[i] >= 'A' && credential[i] <= 'Z') || 
                   (credential[i] >= 'a' && credential[i] <= 'z')) {
            has_alpha = 1;
        } else {
            return 0; // Invalid character for PIN
        }
    }
    
    // Accept if it's all digits OR a mix (like "COOL1")
    return (has_digit || has_alpha);
}

// Function to authenticate user with password or PIN
int authenticate_user(const char* username, const char* credential) {
    HANDLE hToken;
    
    printf("Attempting authentication for user: %s\n", username);
    
    // Determine credential type
    int looks_like_pin = is_pin_format(credential);
    
    if (looks_like_pin) {
        printf("Detected PIN format credential (length: %zu)\n", strlen(credential));
        
        // Try Windows Hello PIN verification first
        if (verify_windows_hello_pin(username, credential)) {
            return 1;
        }
        
        // Try PIN verification with runas command
        if (verify_pin_with_runas(username, credential)) {
            return 1;
        }
        
        // Try PIN verification with logon session
        if (verify_pin_with_logon_session(username, credential)) {
            return 1;
        }
        
        // Try standard PIN authentication methods
        
        // Method 1: LOGON32_LOGON_UNLOCK with default provider
        if (LogonUserA((LPSTR)username, ".", (LPSTR)credential, LOGON32_LOGON_UNLOCK, 
                       LOGON32_PROVIDER_DEFAULT, &hToken)) {
            CloseHandle(hToken);
            printf("✓ PIN authentication via LOGON32_LOGON_UNLOCK successful!\n");
            return 1;
        }
        
        // Method 2: Try with local domain
        if (LogonUserA((LPSTR)username, NULL, (LPSTR)credential, LOGON32_LOGON_INTERACTIVE, 
                       LOGON32_PROVIDER_DEFAULT, &hToken)) {
            CloseHandle(hToken);
            printf("✓ PIN authentication via local domain successful!\n");
            return 1;
        }
        
        // Method 3: Network logon for PIN
        if (LogonUserA((LPSTR)username, ".", (LPSTR)credential, LOGON32_LOGON_NETWORK, 
                       LOGON32_PROVIDER_DEFAULT, &hToken)) {
            CloseHandle(hToken);
            printf("✓ PIN authentication via network logon successful!\n");
            return 1;
        }
        
        printf("× PIN authentication methods failed, trying as password...\n");
    } else {
        printf("Detected password format credential (length: %zu)\n", strlen(credential));
    }
    
    // Try traditional password authentication methods
    
    // Method 1: Interactive logon (standard password method)
    if (LogonUserA((LPSTR)username, ".", (LPSTR)credential, LOGON32_LOGON_INTERACTIVE, 
                   LOGON32_PROVIDER_DEFAULT, &hToken)) {
        CloseHandle(hToken);
        printf("✓ Password authentication successful!\n");
        return 1;
    }
    
    // Method 2: Try with computer name as domain
    char computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computer_name);
    if (GetComputerNameA(computer_name, &size)) {
        if (LogonUserA((LPSTR)username, computer_name, (LPSTR)credential, LOGON32_LOGON_INTERACTIVE, 
                       LOGON32_PROVIDER_DEFAULT, &hToken)) {
            CloseHandle(hToken);
            printf("✓ Password authentication via computer domain successful!\n");
            return 1;
        }
    }
    
    // Method 3: Network logon as fallback
    if (LogonUserA((LPSTR)username, ".", (LPSTR)credential, LOGON32_LOGON_NETWORK, 
                   LOGON32_PROVIDER_DEFAULT, &hToken)) {
        CloseHandle(hToken);
        printf("✓ Network logon authentication successful!\n");
        return 1;
    }
    
    printf("× All authentication methods failed\n");
    return 0; // Authentication failed
}

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

    // Get current Windows username
    char current_user[100];
    DWORD user_len = sizeof(current_user);
    if (!GetUserNameA(current_user, &user_len)) {
        strcpy(current_user, "unknown");
    }

    // --- Getty target login screen ---
    printf("\n");
    printf("WNU OS 1.0.0 LTS %s tty1\n", computername);
    printf("\n");
    printf("%s login: ", computername);
    fflush(stdout);

    if (scanf("%99s", username) != 1) {
        fprintf(stderr, "Login incorrect\n");
        return 1;
    }

    // Check if user is root
    if (strcmp(username, "root") == 0) {
        printf("Welcome, root!\n");
        _chdir("C:\\");
        isnormaluser = 0; // Root user mode
    } else if (strcmp(username, current_user) == 0) {
        // User entered their actual Windows username, now verify password
        char password[100];
        printf("Password: ");
        
        // Clear the input buffer from the previous scanf
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        
        // Read the entire line including spaces
        if (!fgets(password, sizeof(password), stdin)) {
            fprintf(stderr, "No password entered\n");
            return 1;
        }
        
        // Remove the trailing newline from fgets
        password[strcspn(password, "\n")] = 0;
        
        // Authenticate with password or PIN
        if (authenticate_user(username, password)) {
            // Authentication successful
            printf("Welcome, %s!\n", username);
            
            // Determine if it's a PIN or password based on length and content
            if (strlen(password) >= 4 && strlen(password) <= 6 && 
                (strspn(password, "0123456789") == strlen(password) || 
                 strcmp(password, "COOL1") == 0)) {
                printf("PIN authentication successful\n");
            } else {
                printf("Password authentication successful\n");
            }
            
            // Set home directory to C:\Users\{username}
            char home_path[256];
            snprintf(home_path, sizeof(home_path), "C:\\Users\\%s", username);
            if (_chdir(home_path) != 0) {
                perror("Failed to set home directory");
            } else {
                printf("Home directory set to: %s\n", home_path);
            }
            
            isnormaluser = 1; // Normal user mode
        } else {
            // Authentication failed
            printf("Authentication failed for user: %s\n", username);
            printf("Debug: You entered '%s' (length: %zu)\n", password, strlen(password));
            printf("Tip: Try your Windows password or PIN (4-6 digits)\n");
            return 1;
        }
    } else {
        printf("Access denied for user: %s\n", username);
        return 1;
    }

    // --- Shell loop ---
    while (1) {
        char cwd[1024];
        if (!_getcwd(cwd, sizeof(cwd))) {
            strcpy(cwd, "?");
        }

        // Replace path with "~" for normal users
        char display_path[1024];
        if (isnormaluser == 1) {
            // For normal users, replace C:\Users\{username} with ~
            char user_home[256];
            snprintf(user_home, sizeof(user_home), "C:\\Users\\%s", current_user);
            
            if (strncmp(cwd, user_home, strlen(user_home)) == 0) {
                // We're in the user's home directory or subdirectory
                if (strlen(cwd) == strlen(user_home)) {
                    // Exactly in home directory
                    strcpy(display_path, "~");
                } else if (cwd[strlen(user_home)] == '\\') {
                    // In a subdirectory of home
                    snprintf(display_path, sizeof(display_path), "~/%s", cwd + strlen(user_home) + 1);
                } else {
                    // Not actually in home directory, show full path
                    strncpy(display_path, cwd, sizeof(display_path));
                }
            } else {
                // Not in home directory, show full path
                strncpy(display_path, cwd, sizeof(display_path));
            }
        } else {
            // For root user, replace "C:\" with "~"
            if (strcmp(cwd, "C:\\") == 0) {
                strcpy(display_path, "~");
            } else if (strncmp(cwd, "C:\\", 3) == 0) {
                snprintf(display_path, sizeof(display_path), "~/%s", cwd + 3);
            } else {
                strncpy(display_path, cwd, sizeof(display_path));
            }
        }

        for (int i = 0; display_path[i]; ++i) {
            if (display_path[i] == '\\') {
                display_path[i] = '/';
            }
        }

        // Print prompt (# for root, $ for normal user)
        char prompt_symbol = isnormaluser ? '$' : '#';
        printf("%s@%s%c:%s ", computername, username, prompt_symbol, display_path);

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
        } else if (strcmp(command, "whoami") == 0) {
            printf("%s\n", username);
        } else if (strncmp(command, "su ", 3) == 0) {
            char target_user[100];
            sscanf(command + 3, "%99s", target_user);
            if (strcmp(target_user, "root") == 0) {
                printf("Switching to root user...\n");
                strcpy(username, "root");
                isnormaluser = 0;
                _chdir("C:\\");
            } else if (strcmp(target_user, current_user) == 0) {
                printf("Switching to normal user %s...\n", current_user);
                strcpy(username, current_user);
                isnormaluser = 1;
                char home_path[256];
                snprintf(home_path, sizeof(home_path), "C:\\Users\\%s", current_user);
                if (_chdir(home_path) != 0) {
                    perror("Failed to set home directory");
                } else {
                    printf("Home directory set to: %s\n", home_path);
                }
            } else {
                printf("Access denied for user: %s\n", target_user);
            }
        } else if (strcmp(command, "pwd") == 0) {
            char cwd[1024];
            if (!_getcwd(cwd, sizeof(cwd))) {
                strcpy(cwd, "?");
            }
            if(strcmp(cwd, "C:\\") == 0) {
                printf("/\n");
            } 
            else if(strncmp(cwd, "C:\\", 3) == 0) {
                printf("/%s\n", cwd + 3);
            }
            else if(strncmp(cwd, "\\", 3 ) == 0) {
                printf("/%s\n", cwd + 3);
            }
            else {
                printf("%s\n", cwd);
            }

        } else if (strlen(command) > 0) {
            system(command); // Run external command
        }
    }

    return 0;
}
