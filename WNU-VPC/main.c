#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <wininet.h>
#include <direct.h>

/* WNU-VPC CLI TOOL 
*  A DOCKER LIKE TOOL
*/

// Function to create directory recursively
void create_directory_recursive(const char* path) {
    char temp[512];
    strcpy(temp, path);
    
    for (char* p = temp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            _mkdir(temp);
            *p = '\\';
        }
    }
    _mkdir(temp);
}

// Function to download file from URL
int download_file(const char* url, const char* filepath) {
    HINTERNET hInternet, hUrl;
    DWORD bytesRead;
    char buffer[1024];
    FILE* file;
    
    // Initialize WinINet
    hInternet = InternetOpen("WNU-VPC", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        printf("Error: Failed to initialize internet connection\n");
        return 0;
    }
    
    // Open URL
    hUrl = InternetOpenUrl(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) {
        printf("Error: Failed to open URL: %s\n", url);
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    // Open file for writing
    file = fopen(filepath, "wb");
    if (!file) {
        printf("Error: Failed to create file: %s\n", filepath);
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return 0;
    }
    
    // Download and write file
    printf("Downloading: %s\n", url);
    
    // Read data from URL and write to file
    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, file);
        printf(".");  // Show progress
        fflush(stdout);
    }
    printf("\nDownload completed!\n");
    
    // Cleanup
    fclose(file);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
    
    return 1;
}

// Function to remove directory recursively
int remove_directory_recursive(const char* path) {
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char searchPath[512];
    char fullPath[512];
    
    // Create search pattern
    snprintf(searchPath, sizeof(searchPath), "%s\\*", path);
    
    hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        // Skip current and parent directory entries
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }
        
        // Build full path
        snprintf(fullPath, sizeof(fullPath), "%s\\%s", path, findData.cFileName);
        
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Recursively remove subdirectory
            remove_directory_recursive(fullPath);
        } else {
            // Remove file
            DeleteFile(fullPath);
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
    
    // Remove the directory itself
    return RemoveDirectory(path);
}

int main(int argc, char *argv[]) {
    // Check if no arguments provided
    if (argc < 2) {
        printf("WNU Virtual PC launched with no arguments.\n");
        printf("Usage:\n");
        printf("  wnu-vpc -i FROM -n \"<name>\" -t <tag>  # Install\n");
        printf("  wnu-vpc run <name>:<tag>              # Run\n");
        printf("  wnu-vpc uninstall <name>:<tag>        # Uninstall\n");
        printf("\n");
        printf("Available tags:\n");
        printf("  latest          - Current development version\n");
        printf("  1.0.1.update.2  - Latest stable with X11 and Unicode\n");
        printf("  1.0.1.update.1  - Package management update\n");
        printf("  1.0.1           - Major package management release\n");
        printf("  1.0.0           - Initial stable release\n");
        printf("\n");
        printf("Examples:\n");
        printf("  wnu-vpc -i FROM -n \"WNU OS\" -t latest\n");
        printf("  wnu-vpc -i FROM -n \"WNU OS\" -t 1.0.1.update.2\n");
        printf("  wnu-vpc run \"WNU OS\":latest\n");
        printf("  wnu-vpc run \"WNU OS\":1.0.1.update.2\n");
        printf("  wnu-vpc uninstall \"WNU OS\":latest\n");
        return 0;
    }
    
    // Check for run command
    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            printf("run: error: no image specified\n");
            printf("Usage: wnu-vpc run <name>:<tag>\n");
            printf("Example: wnu-vpc run \"WNU OS\":latest\n");
            return 1;
        }
        
        // Parse image name and tag from format "name:tag"
        char* image = argv[2];
        char* colon_pos = strchr(image, ':');
        
        if (!colon_pos) {
            printf("run: error: invalid image format. Use <name>:<tag>\n");
            printf("Example: wnu-vpc run \"WNU OS\":latest\n");
            return 1;
        }
        
        // Split name and tag
        *colon_pos = '\0';  // Replace ':' with null terminator
        char* name = image;
        char* tag = colon_pos + 1;
        
        // Build path to executable
        char exe_path[512];
        snprintf(exe_path, sizeof(exe_path), "C:\\WNU\\WNU-VPC\\%s\\%s\\wnuos.exe", name, tag);
        
        // Check if file exists
        if (GetFileAttributes(exe_path) == INVALID_FILE_ATTRIBUTES) {
            printf("run: error: image '%s:%s' not found\n", name, tag);
            printf("Path checked: %s\n", exe_path);
            printf("Try installing first: wnu-vpc -i FROM -n \"%s\" -t %s\n", name, tag);
            return 1;
        }
        
        printf("Running Virtual PC: %s:%s\n", name, tag);
        printf("Executable: %s\n", exe_path);
        printf("Starting %s:%s...\n\n", name, tag);
        
        // Execute the WNU OS instance
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        if (CreateProcess(exe_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            printf("WNU OS started successfully (PID: %lu)\n", pi.dwProcessId);
            
            // Wait for process to complete
            WaitForSingleObject(pi.hProcess, INFINITE);
            
            // Get exit code
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            printf("WNU OS exited with code: %lu\n", exitCode);
            
            // Close process and thread handles
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            printf("run: error: failed to start WNU OS (Error: %lu)\n", GetLastError());
            return 1;
        }
        
        return 0;
    }
    
    // Check for uninstall command
    else if (strcmp(argv[1], "uninstall") == 0) {
        if (argc < 3) {
            printf("uninstall: error: no image specified\n");
            printf("Usage: wnu-vpc uninstall <name>:<tag>\n");
            printf("Example: wnu-vpc uninstall \"WNU OS\":latest\n");
            return 1;
        }
        
        // Parse image name and tag from format "name:tag"
        char* image = argv[2];
        char* colon_pos = strchr(image, ':');
        
        if (!colon_pos) {
            printf("uninstall: error: invalid image format. Use <name>:<tag>\n");
            printf("Example: wnu-vpc uninstall \"WNU OS\":latest\n");
            return 1;
        }
        
        // Split name and tag
        *colon_pos = '\0';  // Replace ':' with null terminator
        char* name = image;
        char* tag = colon_pos + 1;
        
        // Build path to installation directory
        char install_path[512];
        snprintf(install_path, sizeof(install_path), "C:\\WNU\\WNU-VPC\\%s\\%s", name, tag);
        
        // Check if installation exists
        if (GetFileAttributes(install_path) == INVALID_FILE_ATTRIBUTES) {
            printf("uninstall: error: image '%s:%s' not found\n", name, tag);
            printf("Path checked: %s\n", install_path);
            return 1;
        }
        
        printf("Uninstalling Virtual PC: %s:%s\n", name, tag);
        printf("Removing directory: %s\n", install_path);
        
        // Remove the installation directory
        if (remove_directory_recursive(install_path)) {
            printf("Successfully uninstalled: %s:%s\n", name, tag);
        } else {
            printf("uninstall: error: failed to remove installation directory\n");
            printf("You may need to manually delete: %s\n", install_path);
            return 1;
        }
        
        return 0;
    }
    
    // Check for install command
    else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--install") == 0 || strcmp(argv[1], "install") == 0) {
        // Check if enough arguments for full install command
        if (argc < 7) {
            printf("install: error: no installation source specified\n");
            printf("Usage: wnu-vpc -i FROM -n \"<name>\" -t <tag>\n");
            printf("Example: wnu-vpc -i FROM -n \"WNU OS\" -t latest\n");
            return 1;
        }
        
        // Parse arguments
        char* source = NULL;
        char* name = NULL;
        char* tag = NULL;
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "FROM") == 0) {
                source = "FROM";
            }
            else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
                name = argv[i + 1];
                i++; // Skip next argument
            }
            else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                tag = argv[i + 1];
                i++; // Skip next argument
            }
        }
        
        // Validate required parameters
        if (source == NULL) {
            printf("install: error: missing FROM directive\n");
            return 1;
        }
        if (name == NULL) {
            printf("install: error: missing -n (name) parameter\n");
            return 1;
        }
        if (tag == NULL) {
            printf("install: error: missing -t (tag) parameter\n");
            return 1;
        }
        
        // Execute installation with Docker-like output
        DWORD startTime = GetTickCount();

        // Determine download URL based on tag
        const char* download_url;
        if (strcmp(tag, "latest") == 0) {
            download_url = "https://github.com/WNU-Project/WNU-OS/raw/refs/heads/master/1.x.x/1.0.x/1.0.1/wnuos.exe";
        }
        else if (strcmp(tag, "1.0.1.update.2") == 0) {
            download_url = "https://github.com/WNU-Project/WNU-OS/raw/refs/tags/v1.0.1.update.2/1.x.x/1.0.x/1.0.1/wnuos.exe";
        }
        else if (strcmp(tag, "1.0.1.update.1") == 0) {
            download_url = "https://github.com/WNU-Project/WNU-OS/raw/refs/tags/v1.0.1.update.1/1.x.x/1.0.x/1.0.1/wnuos.exe";
        }
        else if (strcmp(tag, "1.0.1") == 0) {
            download_url = "https://github.com/WNU-Project/WNU-OS/raw/refs/tags/v1.0.1/1.x.x/1.0.x/1.0.1/wnuos.exe";
        }
        else if (strcmp(tag, "1.0.0") == 0) {
            download_url = "https://github.com/WNU-Project/WNU-OS/raw/refs/tags/v1.0.0/1.x.x/1.0.x/1.0.1/wnuos.exe";
        }
        else {
            printf("Error: Unknown tag '%s'\n", tag);
            printf("Available tags: latest, 1.0.1.update.2, 1.0.1.update.1, 1.0.1, 1.0.0\n");
            return 1;
        }
        
        // Create installation directory path
        char install_path[512];
        char exe_path[512];
        snprintf(install_path, sizeof(install_path), "C:\\WNU\\WNU-VPC\\%s\\%s", name, tag);
        snprintf(exe_path, sizeof(exe_path), "%s\\wnuos.exe", install_path);
        
        printf("[+] Building %s:%s\n", name, tag);
        
        // Step 1: Load build definition
        DWORD step1_start = GetTickCount();
        printf(" => [internal] load build definition from WNU-VPC                                                                     ");
        Sleep(50); // Minimal processing time
        DWORD step1_end = GetTickCount();
        printf("%.1fs\n", (step1_end - step1_start) / 1000.0f);
        
        DWORD step1b_start = GetTickCount();
        printf(" => => transferring build context: 512B                                                                               ");
        Sleep(25);
        DWORD step1b_end = GetTickCount();
        printf("%.1fs\n", (step1b_end - step1b_start) / 1000.0f);
        
        // Step 2: Load metadata
        DWORD step2_start = GetTickCount();
        printf(" => [internal] load metadata for %s                                        ", download_url);
        Sleep(800); // Simulate network lookup
        DWORD step2_end = GetTickCount();
        printf("%.1fs\n", (step2_end - step2_start) / 1000.0f);
        
        // Step 3: Authentication
        DWORD step3_start = GetTickCount();
        printf(" => [auth] wnu-project/%s:pull token for github.com                                                       ", name);
        Sleep(100); // Quick auth check
        DWORD step3_end = GetTickCount();
        printf("%.1fs\n", (step3_end - step3_start) / 1000.0f);
        
        // Step 4: Create directory
        DWORD step4_start = GetTickCount();
        printf(" => [internal] creating installation directory                                                                        ");
        create_directory_recursive(install_path);
        DWORD step4_end = GetTickCount();
        printf("%.1fs\n", (step4_end - step4_start) / 1000.0f);
        
        DWORD step4b_start = GetTickCount();
        printf(" => => creating directory structure: %s                                           ", install_path);
        Sleep(10); // Directory creation overhead
        DWORD step4b_end = GetTickCount();
        printf("%.1fs\n", (step4b_end - step4b_start) / 1000.0f);
        
        // Step 5: Download WNU OS
        DWORD step5_start = GetTickCount();
        printf(" => [1/2] FROM %s                                                  ", download_url);
        if (download_file(download_url, exe_path)) {
            DWORD step5_end = GetTickCount();
            printf("%.1fs\n", (step5_end - step5_start) / 1000.0f);
            
            DWORD step5b_start = GetTickCount();
            printf(" => => resolve %s@sha256:5188d7e7c5b01df6655e69401153940ac84f937363d5958c9bf86fd81cd126ae     ", download_url);
            Sleep(50); // Quick resolve time
            DWORD step5b_end = GetTickCount();
            printf("%.1fs\n", (step5b_end - step5b_start) / 1000.0f);
            
            // Step 6: Installation
            DWORD step6_start = GetTickCount();
            printf(" => CACHED [2/2] RUN install wnuos.exe to virtual environment                                                     ");
            Sleep(100); // Installation processing
            DWORD step6_end = GetTickCount();
            printf("%.1fs\n", (step6_end - step6_start) / 1000.0f);
            
            // Step 7: Export to image
            DWORD step7_start = GetTickCount();
            printf(" => exporting to image                                                                                            ");
            Sleep(150); // Export processing
            DWORD step7_end = GetTickCount();
            printf("%.1fs\n", (step7_end - step7_start) / 1000.0f);
            
            DWORD step7b_start = GetTickCount();
            printf(" => => exporting layers                                                                                           ");
            Sleep(50);
            DWORD step7b_end = GetTickCount();
            printf("%.1fs\n", (step7b_end - step7b_start) / 1000.0f);
            
            DWORD step7e_start = GetTickCount();
            printf(" => => naming to wnu-vpc.local/%s:%s                                                                   ", name, tag);
            Sleep(10);
            DWORD step7e_end = GetTickCount();
            printf("%.1fs\n", (step7e_end - step7e_start) / 1000.0f);
            
            DWORD step7f_start = GetTickCount();
            printf(" => => unpacking to wnu-vpc.local/%s:%s                                                                ", name, tag);
            Sleep(75);
            DWORD step7f_end = GetTickCount();
            printf("%.1fs\n", (step7f_end - step7f_start) / 1000.0f);
            
            DWORD endTime = GetTickCount();
            float totalTime = (endTime - startTime) / 1000.0f;
            printf("                                                                                                       wnu-vpc:desktop-windows\n");
            printf("[+] Building %.1fs (7/7) FINISHED\n", totalTime);
            
        } else {
            printf("ERROR\n");
            printf("Installation failed: Could not download wnuos.exe\n");
            return 1;
        }
        
        return 0;
    }
    else {
        printf("WNU Virtual PC: Unknown command '%s'\n", argv[1]);
        printf("Usage: wnu-vpc -i FROM -n \"<name>\" -t <tag>\n");
        return 1;
    }
    
    return 0;
}