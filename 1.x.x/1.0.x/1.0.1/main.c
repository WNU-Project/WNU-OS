#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <direct.h>  // for _getcwd and _chdir
#include <lmcons.h>  // for UNLEN
#include <conio.h>   // for _getch function
#include <time.h>
#include <stdbool.h> // for bool, true, false
#include "boot.h"
#include "userlogin.h"
#include "poweroff.h"
#include "halt.h"
#include "reboot.h"
#include "wsys2/wsys2.h"  // Include WSYS2 package manager
#include "tty_session.h" // Include TTY session management
#include "motd.h"      // Include motd function
// Include GUI header so main can call x11()
#include "X11/x11.h"
#include "fastfetch.h"

#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64 12
#endif

// Define the global variable
int isnormaluser = 0;
// Global boot target (defined once for whole program)
char* target = NULL;

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
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    char username[100];
    char computername[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computername);
    char current_user[100];
    DWORD user_len = sizeof(current_user);
    char current_tty[16];

    // Get computer name
    if (!GetComputerNameA(computername, &size)) {
        strcpy(computername, "unknown");
    }

    // Run boot sequence
    if (boot_sequence() == 0) {
        /*Go To Login*/
    } else {
        exit(1);
    }
    while (1) {
         // Get current Windows username
        if (!GetUserNameA(current_user, &user_len)) {
            strcpy(current_user, "unknown");
        }

        strcpy(current_tty, "tty1");

        // --- Getty target login screen ---
        printf("\n");
        printf("WNU OS 1.0.1 Update 2 %s %s %s %s US EDT UTC -4:00\n", computername, current_tty, __DATE__, __TIME__);
        printf("\n");
        printf("%s login: ", computername);
        fflush(stdout);

        if (!fgets(username, sizeof(username), stdin)) {
            fprintf(stderr, "Login incorrect\n");
            continue;
        }
        
        // Remove newline character from username if present
        username[strcspn(username, "\n")] = 0;

        // Check if user is root
        if (strcmp(username, "root") == 0) {
            printf("Welcome, root!\n");
            _chdir("C:\\");
            isnormaluser = 0; // Root user mode
        } else if (strcmp(username, current_user) == 0) {
            // User entered their actual Windows username, now verify password
            char password[100];
            printf("Password: ");
            fflush(stdout);
            
            // No need to clear input buffer since fgets() already consumed the newline
            
            // Read password with Unix-style hidden input (no visual feedback)
            int i = 0;
            int c;
            while (i < 99) {
                c = _getch(); // Get character without echoing to console
                
                if (c == '\r' || c == '\n') { // Enter key pressed
                    break;
                } else if (c == '\b' || c == 127) { // Backspace pressed
                    if (i > 0) {
                        // No visual feedback for backspace in Unix style
                        i--;
                    }
                } else if (c >= 32 && c <= 126) { // Printable characters
                    password[i] = c;
                    // No visual feedback - completely hidden like Unix
                    i++;
                }
            }
            password[i] = '\0'; // Null-terminate the password
            printf("\n"); // Move to next line after password input
            
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
                continue;
            }
        } else {
            printf("Access denied for user: %s\n", username);
            continue;
        }
        if (strcmp(target, "Graphical") == 0) {
            printf("Starting graphical interface...\n");
            Sleep(1000);
            x11(); // Start X11 graphical interface
            system("cls");
            poweroff_sequence();
            exit(0);
        }
        if (strcmp(target, "Multi-User") == 0) {
            /* proceed to shell loop (fall-through) */
        }

        motd(); // Display message of the day

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

        // Print prompt (# for root, $ for normal user) with colors
        if (isnormaluser) {
            // Green $ for normal user
            printf("\033[32m%s@%s\033[0m:\033[34m%s\033[0m\033[32m$\033[0m ", computername, username, display_path);
        } else {
            // Red # for root user
            printf("%s@%s:%s# ", computername, username, display_path);
        }

        // Read command
        char command[256];
        if (!fgets(command, sizeof(command), stdin)) {
            break; // EOF
        }

        // Strip newline
        command[strcspn(command, "\n")] = 0;

        // Handle built-in commands
        if (strcmp(command, "poweroff") == 0) {
            printf("In 10 Seconds, the shell will power off.\n");
            Sleep(10000);
            system("cls");
            if (poweroff_sequence() == 0) {
                exit(0); // Terminate shell and program
            }
        } 
        else if (strcmp(command, "poweroff now") == 0) {
            printf("Shutting down immediately...\n");
            Sleep(5);
            system("cls");
            if (poweroff_sequence() == 0) {
                exit(0); // Terminate shell and program
            }
        }
        else if (strcmp(command, "halt") == 0) {
            printf("Halting system...\n");
            printf("Press Ctrl+S+H during or after halt to restart shell\n");
            Sleep(2000); // Give user time to see the message
            system("cls");
            
            int halt_result = halt_sequence();
            
            // Check if user pressed Ctrl+S+H to restart
            if (halt_result == -1) {
                // Halt was interrupted/restarted by Ctrl+S+H
                printf("\n\033[33m[SYSTEM RESTART]\033[0m Shell restart requested by user!\n");
                Sleep(1500);
                system("cls");
                
                // Restart the shell (simulate shell restart)
                printf("\033[32m[SYSTEM]\033[0m WNU OS Shell Restarting...\n");
                Sleep(1000);
                printf("\033[36mShell restart complete.\033[0m\n\n");
                
                // Continue with shell loop (don't break)
                continue;
            }
            // Note: halt_result should never be 0 since we stay in halt state until restart
            // The halt function now stays in an infinite loop until Ctrl+S+H is pressed
        }
        else if (strcmp(command, "reboot") == 0) {
            printf("Rebooting system...\n");
            printf("Press Ctrl+C, ESC, or I key to interrupt reboot\n");
            Sleep(2000);
            system("cls");
            
            int reboot_result = reboot_sequence();
            
            // Check if reboot was interrupted
            if (reboot_result == -1) {
                // Reboot was interrupted by user
                printf("\n\033[33m[REBOOT CANCELLED]\033[0m Reboot interrupted by user!\n");
                Sleep(1500);
                system("cls");
                
                // Return to shell (simulate cancellation)
                printf("\033[32m[SYSTEM]\033[0m Reboot cancelled, returning to shell...\n");
                Sleep(1000);
                printf("\033[36mBack to WNU OS shell.\033[0m\n\n");
                
                // Continue with shell loop (don't break)
                continue;
            } else if (reboot_result == 0) {     
                break;
            }
        } 
        else if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0) {
            system("cls");
        }
        else if (strncmp(command, "cd ", 3) == 0) {
            if (_chdir(command + 3) != 0) {
                perror("cd failed");
            }
        } else if (strcmp(command, "whoami") == 0) {
            printf("FULL USERNAME: %s/%s\n", computername, username);
            if (isnormaluser) {
                printf("USER'S HOME DIRECTORY: C:/Users/%s\n", username);
            } else {
                printf("USER'S HOME DIRECTORY: C:/%s\n", username);
            }
            printf("USERNAME: %s\n", username);
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
                // Convert backslashes to forward slashes for Unix-style output
                char unix_path[1024];
                strcpy(unix_path, cwd + 3);
                for (int i = 0; unix_path[i]; i++) {
                    if (unix_path[i] == '\\') {
                        unix_path[i] = '/';
                    }
                }
                printf("/%s\n", unix_path);
            }
            else if(strncmp(cwd, "\\", 3 ) == 0) {
                // Convert backslashes to forward slashes for Unix-style output
                char unix_path[1024];
                strcpy(unix_path, cwd + 3);
                for (int i = 0; unix_path[i]; i++) {
                    if (unix_path[i] == '\\') {
                        unix_path[i] = '/';
                    }
                }
                printf("/%s\n", unix_path);
            }
            else {
                printf("%s\n", cwd);
            }
        

        } 
        else if (strcmp(command, "uname -a") == 0) {
         SYSTEM_INFO sysinfo;
         GetSystemInfo(&sysinfo);

         const char* arch;
         switch (sysinfo.wProcessorArchitecture) {
         case PROCESSOR_ARCHITECTURE_AMD64:
            arch = "x86_64";
            break;
         case PROCESSOR_ARCHITECTURE_INTEL:
            // Check if it's actually 64-bit Intel running in 32-bit mode
            if (sysinfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM || 
                sysinfo.dwProcessorType == PROCESSOR_INTEL_IA64 ||
                sysinfo.wProcessorLevel >= 6) {
                // Modern Intel processors (Core series, etc.) - assume 64-bit capable
                arch = "x86_64";
            } else {
                arch = "i686"; // Older 32-bit Intel
            }
            break;
         case PROCESSOR_ARCHITECTURE_ARM:
            arch = "arm";
            break;
         case PROCESSOR_ARCHITECTURE_ARM64:
            arch = "aarch64";
            break;
         default:
            arch = "unknown";
            break;
         }

         printf("WNU OS %s 1.0.1 WNU-Kernel-1.0.1 #1 SMP FULL_RELEASE WNU/2025 %s WNU\n", computername, arch);
        }
        else if (strncmp(command, "ls", 2) == 0) {
            // Custom ls implementation
            WIN32_FIND_DATAA findData;
            HANDLE hFind;
            
            // Check for flags
            int show_hidden = 0;
            int long_format = 0;
            
            if (strstr(command, "-a")) show_hidden = 1;
            if (strstr(command, "-l")) long_format = 1;
            
            // Start finding files
            hFind = FindFirstFileA("*", &findData);
            if (hFind == INVALID_HANDLE_VALUE) {
                printf("ls: cannot access '.': No such file or directory\n");
            } else {
                do {
                    // Skip hidden files unless -a flag is used
                    if (!show_hidden && findData.cFileName[0] == '.') {
                        continue;
                    }
                    
                    if (long_format) {
                        // Long format: show permissions, size, date, name
                        char* type = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "d" : "-";
                        char permissions[] = "rwxrwxrwx"; // Simplified permissions
                        
                        // Convert file time to readable format
                        SYSTEMTIME st;
                        FileTimeToSystemTime(&findData.ftLastWriteTime, &st);
                        
                        // File size (directories show as 0)
                        DWORD fileSize = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 0 : findData.nFileSizeLow;
                        
                        printf("%s%s %8lu %02d/%02d/%04d %02d:%02d %s\n",
                               type, permissions, fileSize,
                               st.wMonth, st.wDay, st.wYear,
                               st.wHour, st.wMinute,
                               findData.cFileName);
                    } else {
                        // Simple format: just names with colors
                        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                            // Blue for directories
                            printf("\033[34m%s\033[0m\n", findData.cFileName);
                        } else {
                            // White for files
                            printf("%s\n", findData.cFileName);
                        }
                    }
                } while (FindNextFileA(hFind, &findData));
                FindClose(hFind);
            }
        }
        else if (strcmp(command, "wsys2") == 0 && isnormaluser == 0) {
            // Initialize WSYS2 system
            if (wsys2_init() != 0) {
                printf("\033[31mError:\033[0m Failed to initialize WSYS2 system\n");
                continue;
            }
            
            printf("\033[36m----------------------------------------\033[0m\n");
            printf("\033[36m-         WSYS2 Package Manager        -\033[0m\n");
            printf("\033[36m-            WNU OS 1.0.1              -\033[0m\n");
            printf("\033[36m----------------------------------------\033[0m\n");
            printf("Type 'help' for available commands, 'exit' to return to WNU OS shell\n\n");
            
            // WSYS2 shell loop
            while (1) {
                printf("\033[33mwsys2>\033[0m ");
                fflush(stdout);
                
                char wsys2_input[256];
                if (!fgets(wsys2_input, sizeof(wsys2_input), stdin)) {
                    break; // EOF or error
                }
                
                // Remove newline
                wsys2_input[strcspn(wsys2_input, "\n")] = 0;
                
                // Skip empty input
                if (strlen(wsys2_input) == 0) {
                    continue;
                }
                
                // Tokenize input
                char* args[10];
                int arg_count = 0;
                char* token = strtok(wsys2_input, " ");
                while (token && arg_count < 10) {
                    args[arg_count++] = token;
                    token = strtok(NULL, " ");
                }
                
                if (arg_count == 0) continue; // Empty input
                
                // Handle WSYS2 commands
                if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) {
                    printf("Returning to WNU OS shell...\n\n");
                    break;
                } else if (strcmp(args[0], "install") == 0) {
                    if (arg_count < 2) {
                        printf("\033[31mError:\033[0m No package specified for installation\n");
                        printf("Usage: install <package.wnupkg>\n");
                        continue;
                    }
                    wsys2_install(args[1]);
                } else if (strcmp(args[0], "remove") == 0 || strcmp(args[0], "uninstall") == 0) {
                    if (arg_count < 2) {
                        printf("\033[31mError:\033[0m No package specified for removal\n");
                        printf("Usage: remove <package>\n");
                        continue;
                    }
                    wsys2_remove(args[1]);
                } else if (strcmp(args[0], "update") == 0) {
                    wsys2_update();
                } else if (strcmp(args[0], "search") == 0) {
                    if (arg_count < 2) {
                        wsys2_search(NULL); // Show all available packages
                    } else {
                        wsys2_search(args[1]);
                    }
                } else if (strcmp(args[0], "list") == 0) {
                    wsys2_list();
                } else if (strcmp(args[0], "info") == 0) {
                    if (arg_count < 2) {
                        printf("\033[31mError:\033[0m No package specified for info\n");
                        printf("Usage: info <package>\n");
                        continue;
                    }
                    wsys2_info(args[1]);
                } else if (strcmp(args[0], "online") == 0 && arg_count >= 2) {
                    if (strcmp(args[1], "update") == 0) {
                        wsys2_online_update();
                    } else if (strcmp(args[1], "search") == 0) {
                        if (arg_count >= 3) {
                            wsys2_online_search(args[2]);
                        } else {
                            wsys2_online_search(NULL);
                        }
                    } else if (strcmp(args[1], "install") == 0) {
                        if (arg_count >= 3) {
                            wsys2_online_install(args[2]);
                        } else {
                            printf("\033[31mError:\033[0m No package specified for online installation\n");
                            printf("Usage: online install <package>\n");
                        }
                    } else if (strcmp(args[1], "list") == 0) {
                        wsys2_online_list();
                    } else {
                        printf("\033[31mError:\033[0m Unknown online command '%s'\n", args[1]);
                        printf("Available online commands: update, search, install, list\n");
                    }
                } else if (strcmp(args[0], "run") == 0) {
                    if (arg_count < 2) {
                        printf("\033[31mError:\033[0m No package specified to run\n");
                        printf("Usage: run <package>\n");
                        continue;
                    }
                    
                    const char* package_spec = args[1];
                    const char* program_name = (arg_count > 2) ? args[2] : NULL;
                    char** run_args = (arg_count > 3) ? &args[3] : NULL;
                    int run_arg_count = (arg_count > 3) ? arg_count - 3 : 0;
                    
                    wsys2_run(package_spec, program_name, run_args, run_arg_count);
                } else if (strcmp(args[0], "info") == 0) {
                    if (arg_count < 2) {
                        printf("\033[31mError:\033[0m No package specified for info\n");
                        printf("Usage: info <package>\n");
                        continue;
                    }
                    wsys2_info(args[1]);
                
                } else if (strcmp(args[0], "upgrade") == 0) {
                    wsys2_upgrade(args[1]);
                } else if (strcmp(args[0], "version") == 0 || strcmp(args[0], "--version") == 0) {
                    printf("WSYS2 Package Manager v1.0.0\n");
                    printf("Integrated into WNU OS 1.0.1\n");
                    printf("Package format: .wnupkg\n");
                } else if (strcmp(args[0], "help") == 0 || strcmp(args[0], "--help") == 0) {
                    printf("WSYS2 Package Manager Commands:\n\n");
                    printf("Local Package Management:\n");
                    printf("  \033[32minstall\033[0m <package>    Install a package from .wnupkg file\n");
                    printf("  \033[31mremove\033[0m  <package>    Remove an installed package\n");
                    printf("  \033[33mupdate\033[0m              Update all installed packages\n");
                    printf("  \033[34msearch\033[0m  [term]      Search for available packages\n");
                    printf("  \033[35mlist\033[0m                List all installed packages\n");
                    printf("  \033[36minfo\033[0m    <package>    Show package information\n");
                    printf("  \033[37mrun\033[0m     <pkg> [prog] [args...]  Run a program from a package\n");
                    printf("  \033[37mupgrade\033[0m [package] Upgrade an installed package to latest version\n");
                    printf("\nOnline Package Management:\n");
                    printf("  \033[32monline install\033[0m <pkg> Install package from online repository\n");
                    printf("  \033[34monline search\033[0m [term] Search online repository\n");
                    printf("  \033[35monline list\033[0m          List available online packages\n");
                    printf("  \033[33monline update\033[0m        Update online package cache\n");
                    printf("\nSystem Commands:\n");
                    printf("  \033[37mversion\033[0m             Show WSYS2 version\n");
                    printf("  \033[37mhelp\033[0m                Show this help message\n");
                    printf("  \033[31mexit\033[0m                Return to WNU OS shell\n");
                    printf("\nExamples:\n");
                    printf("  install hello-world.wnupkg\n");
                    printf("  online install wnu-dev-tools\n");
                    printf("  online list\n");
                    printf("  remove hello-world\n");
                } else {
                    printf("\033[31mError:\033[0m Unknown command '%s'\n", args[0]);
                    printf("Type 'help' for available commands or 'exit' to return to WNU OS shell\n");
                }
            }
        }
        else if (strcmp(command, "wsys2") == 0 && isnormaluser == 1) {
            printf("Error: WSYS2 can only be run as root user. Use 'su root' to switch to root.\n");
        }
        else if (strncmp(command, "switch tty", 10) == 0) {
            // Command: switch tty <number>
            // Parse the tty number after the command
            const char* p = command + 10;
            while (*p == ' ') p++;
            if (*p == '\0') {
                printf("Usage: switch tty <NUMBER>\n");
            } else {
                // Only accept numeric tty numbers
                char tty_num[16];
                int i = 0;
                while (*p && *p != ' ' && i < (int)sizeof(tty_num)-1) {
                    if (*p < '0' || *p > '9') {
                        printf("Error: tty must be a number!\n");
                        tty_num[0] = '\0';
                        break;
                    }
                    tty_num[i++] = *p++;
                }
                tty_num[i] = '\0';
                if (tty_num[0] == '\0') {
                    // Invalid tty number
                    return 0;
                }

                // Save current session
                char save_cwd[1024];
                if (!_getcwd(save_cwd, sizeof(save_cwd))) save_cwd[0] = '\0';
                if (tty_session_save(current_tty, username, save_cwd) == 0) {
                    printf("Saved session for %s\n", current_tty);
                } else {
                    printf("Warning: failed to save session for %s\n", current_tty);
                }

                // Always use 'tty' prefix
                snprintf(current_tty, sizeof(current_tty), "tty%s", tty_num);

                // Load target session if present
                char loaded_user[256];
                char loaded_cwd[1024];
                int r = tty_session_load(current_tty, loaded_user, sizeof(loaded_user), loaded_cwd, sizeof(loaded_cwd));
                if (r == 0) {
                    // Restore user and cwd
                    printf("Switched to %s (restored user=%s cwd=%s)\n", current_tty, loaded_user, loaded_cwd);
                    // Copy username
                    strncpy(username, loaded_user, sizeof(username)-1);
                    username[sizeof(username)-1] = '\0';

                    // Try to change to saved directory
                    if (strlen(loaded_cwd) > 0) {
                        if (_chdir(loaded_cwd) != 0) {
                            perror("Failed to change to saved cwd");
                        }
                    }

                    // Adjust user mode: if username == root -> root mode, else normal user
                    if (strcmp(username, "root") == 0) {
                        isnormaluser = 0;
                    } else {
                        isnormaluser = 1;
                    }
                } else {
                    printf("Switched to %s (no saved session)\n", current_tty);
                    // Set username to default (root)
                    strncpy(username, "root", sizeof(username)-1);
                    username[sizeof(username)-1] = '\0';
                    isnormaluser = 0;
                }
            }
        }
        else if (strcmp(command, "tty") == 0) {
            printf("%s\n", current_tty);
        }
        else if (strcmp(command, "startx") == 0) {
            printf("Starting graphical environment...\n");
            x11();
        }
        else if (strcmp(command, "fastfetch") == 0) {
            snprintf(username_str, sizeof(username_str), "%s", username);
            fastfetch();
        }
        else if (strcmp(command, "devsnote") == 0) {
            printf("Hello There, User! nice to see you!\n");
            printf("It Is Nov 1st 12:01-12:15 PM EDT 2025 as i am writing this right now.\n");
            printf("Since i made this, it was just a simple shell Proejct, but now, it just....... wow.\n");
            printf("It has it's own X11 GUI, Package manager, Fastfetch, and Unix stuff, like switching TTYs!\n");
            printf("But, The Dev Of WNU OS 1.0.1 Update 2 is going to end very soon, i am in a Technology Competition, Named The \"The 2025-2026 Fulton County Student Technology Competition\" And It Starts In Jan 10th, 2025 In North Springs High School.\n");
            printf("My parnets are pround of me making this, but they told me on Nov 2nd is the LAST DAY to work on WNU OS 1.0.1 Update 2, before i work on my Presentation.\n");
            printf("Don't worry, WNU OS is not ending, it just the Development Of WNU OS 1.0.1 Update 2 is ending, and the GUI was the HARDEST PART!\n");
            printf("It took So long to fix the error when it closes when it opens! so i rewrote and it works.\n");
            printf("It was a problem because i was OPPSESED of winning it! i did not want to get 2nd or 3rd palce i wanted 1ST!\n");
            printf("but other thne that, in the beginning of 2025, is where i got hooked on Programming, starting with sratch, then Python, then C#, then C++ (GOSH THAT WAS HARD), then C! and it jsut all in ONE YEAR, YES ALL IN 2025!\n");
            printf("So Thank you users, for using WNU OS 1.0.1 Update 2, get ready For Update 3! I have Plans For it already!\n");
            printf("- @Coolis1362, The Founder Of the WNU Proejct, an the dev Of WNU OS\n");
        }
        else if (strcmp(command, "help") == 0) {
            system("help");
            printf("\n\nWNU OS COMMANDS:\n");
            printf("  poweroff         - Shutdown the system\n");
            printf("  poweroff now     - Immediate shutdown\n");
            printf("  halt             - Halt the system (Ctrl+S+H to restart shell)\n");
            printf("  reboot           - Reboot the system (Ctrl+C, ESC, or I to cancel)\n");
            printf("  clear / cls     - Clear the screen\n");
            printf("  cd <dir>        - Change directory\n");
            printf("  pwd             - Print working directory\n");
            printf("  whoami         - Display current user information\n");
            printf("  su <user>      - Switch user (root or your Windows username)\n");
            printf("  ls [-a] [-l]   - List directory contents\n");
            printf("  wsys2          - Launch WSYS2 Package Manager (root only)\n");
            printf("  switch tty <N> - Switch to TTY number N\n");
            printf("  tty             - Display current TTY\n");
            printf("  startx         - Start graphical environment\n");
            printf("  fastfetch      - Display system information\n");
            printf("  devsnote       - Message from the developer\n");
            printf("  help           - Show this help message\n");
        }
        else if (strlen(command) > 0) {
            system(command); // Run external command
        }
    }
    }
    return 0;
}
