#ifndef NO_RAYLIB
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "x11.h"
#include "x11_logo.h"
#include <string.h>
#include "raylib.h"
#include "x11_logo.h"
    // ...existing code...

// --- Terminal buffer and shell process definitions ---
#define TERM_MAX_LINES    512
#define TERM_MAX_COLUMNS  256
#define READ_CHUNK_SIZE   512



// Forward declaration for GUI build (real implementation elsewhere)
typedef struct {
    void* hProcess;
    void* hInput;
    void* hOutput;
    int running;
} ChildProc;

#ifndef NO_RAYLIB  /* GUI build only */

// Minimal Windows type shims for process status
typedef unsigned long DWORD;
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0
#endif




// --- Windows process code for GUI build ---
#if defined(_WIN32) && !defined(NO_RAYLIB)

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#include <windows.h>
#undef DrawText
#undef DrawTextEx
#undef CloseWindow
#undef Color
#undef Vector2

int LaunchShell(ChildProc* proc, const char* cmd) {
    HANDLE hChildStdinRd = NULL, hChildStdinWr = NULL;
    HANDLE hChildStdoutRd = NULL, hChildStdoutWr = NULL;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) return 0;
    if (!SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0)) return 0;
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) return 0;
    if (!SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0)) return 0;

    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = hChildStdinRd;
    si.hStdOutput = hChildStdoutWr;
    si.hStdError  = hChildStdoutWr;

    char cmdline[512];
    snprintf(cmdline, sizeof(cmdline), "%s", cmd);

    BOOL ok = CreateProcessA(
        NULL, cmdline, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    );
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
    if (!ok) {
        CloseHandle(hChildStdinWr);
        CloseHandle(hChildStdoutRd);
        return 0;
    }
    proc->hProcess = pi.hProcess;
    proc->hInput   = hChildStdinWr;
    proc->hOutput  = hChildStdoutRd;
    proc->running  = 1;
    CloseHandle(pi.hThread);
    return 1;
}

int ReadShellOutput(ChildProc* proc, char* buf, int buflen) {
    if (!proc || !proc->hOutput) return 0;
    DWORD bytesRead = 0;
    if (!PeekNamedPipe((HANDLE)proc->hOutput, NULL, 0, NULL, &bytesRead, NULL)) return 0;
    if (bytesRead == 0) return 0;
    if (ReadFile((HANDLE)proc->hOutput, buf, buflen, &bytesRead, NULL)) {
        return (int)bytesRead;
    }
    return 0;
}

int WriteShellInput(ChildProc* proc, const char* buf, int buflen) {
    if (!proc || !proc->hInput) return 0;
    DWORD bytesWritten = 0;
    if (WriteFile((HANDLE)proc->hInput, buf, buflen, &bytesWritten, NULL)) {
        return (int)bytesWritten;
    }
    return 0;
}

void CloseShell(ChildProc* proc) {
    if (!proc) return;
    if (proc->hInput)  { CloseHandle((HANDLE)proc->hInput);  proc->hInput = NULL; }
    if (proc->hOutput) { CloseHandle((HANDLE)proc->hOutput); proc->hOutput = NULL; }
    if (proc->hProcess) {
        TerminateProcess((HANDLE)proc->hProcess, 0);
        CloseHandle((HANDLE)proc->hProcess);
        proc->hProcess = NULL;
    }
    proc->running = 0;
}

#endif // _WIN32 && !NO_RAYLIB
#endif // _WIN32

/* Prevent Windows/Raylib symbol collisions */
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX

int x11(void) {
    int screenWidth  = 1024;
    int screenHeight = 768;
    // Context menu state (must be after raylib include)
    int showContextMenu = 0;
    Vector2 contextMenuPos = {0};
    int contextMenuHover = -1;
    InitWindow(screenWidth, screenHeight, "X11 Desktop");
    SetWindowState(FLAG_WINDOW_ALWAYS_RUN);
    SetWindowFocused();
    // No fullscreen flag; window will be sized to monitor but windowed
    SetTargetFPS(60);

        // Ensure drawing code is always executed
    // X11 color palette
    Color x11_blue    = (Color){ 40,  60, 180, 255}; // X11 blue
    Color x11_gray    = (Color){200, 200, 210, 255}; // X11 light gray
    Color x11_dkgray  = (Color){ 60,  60,  70, 255}; // X11 dark gray
    // Color x11_black   = (Color){  0,   0,   0, 255}; // Unused
    Color x11_white   = (Color){255, 255, 255, 255};
    Color x11_border  = (Color){ 80,  80, 120, 255};
    Color x11_title   = (Color){ 30,  30,  60, 255};
    Color x11_term_bg = (Color){ 18,  18,  18, 255};
    Color x11_term_fg = (Color){  0, 255, 128, 255};

    // Load X logo (use your PNG or draw a simple X)
    Image logoImage = LoadImageFromMemory(".png", X11_png, X11_png_len);
    Texture2D logo  = LoadTextureFromImage(logoImage);
    UnloadImage(logoImage);

    // Desktop icon (move lower, right of top bar)
    const float iconScale = 0.18f;
    int topBarH = 48;
    int   icon_x = 32;
    int   icon_y = topBarH + 32;
    int   icon_w = (int)(logo.width  * iconScale);
    int   icon_h = (int)(logo.height * iconScale);

    // Terminal window state
    int terminal_open = 0;
    Rectangle termWin = (Rectangle){ screenWidth * 0.31f, screenHeight * 0.23f, screenWidth * 0.62f, screenHeight * 0.55f };
    int dragging = 0;
    Vector2 dragOffset = {0};
    int close_hover = 0;

    // Terminal content buffer
    char  lines[TERM_MAX_LINES][TERM_MAX_COLUMNS];
    int   lineCount = 0;
    char  inputLine[TERM_MAX_COLUMNS] = {0};
    int   inputLen = 0;

    ChildProc shell = {0};

    int running = 1;
    int debug_window_should_close = 0;
    int frameCount = 0;
    // char lastInput[TERM_MAX_COLUMNS] = {0}; // Unused
    while (running) {
        frameCount++;
        // Wait a few frames before checking for close to avoid false positive
        if (frameCount > 5 && WindowShouldClose()) {
            debug_window_should_close = 1;
            running = 0;
        }

        // Handle right-click anywhere (for easier testing)
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            showContextMenu = 1;
            contextMenuPos = mouse;
        }
        // Handle left-click on context menu
        if (showContextMenu && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            int menuW = 180, menuH = 32;
            Rectangle xtermRect = {menuX, menuY, menuW, menuH};
            Rectangle aboutRect = {menuX, menuY + menuH, menuW, menuH};
            Rectangle exitRect  = {menuX, menuY + 2*menuH, menuW, menuH};
            if (CheckCollisionPointRec(mouse, xtermRect)) {
                // Launch terminal if not open
                if (!terminal_open) {
                    terminal_open = 1;
                    shell.running = 1;
                    LaunchShell(&shell, "\"C:\\WNU\\WNU OS\\wnuos.exe\"");
                    lineCount = 0;
                    inputLen = 0;
                    inputLine[0] = '\0';
                }
            } else if (CheckCollisionPointRec(mouse, aboutRect)) {
                printf("About X11 Desktop: WNU OS X11 GUI Made In: C With Raylib\n"); fflush(stdout);
            } else if (CheckCollisionPointRec(mouse, exitRect)) {
                printf("[X11] Exiting X11 Desktop...\n"); fflush(stdout);
                running = 0;
            }
            showContextMenu = 0;
        }
        // Handle click on terminal icon to launch shell in-window
        if (!terminal_open && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (mouse.x >= icon_x && mouse.x <= icon_x + icon_w &&
                mouse.y >= icon_y && mouse.y <= icon_y + icon_h) {
                // Only open terminal, never close window
                terminal_open = 1;
                shell.running = 1; // Always keep running for stub
                LaunchShell(&shell, "\"C:\\WNU\\WNU OS\\wnuos.exe\"");
                lineCount = 0;
                inputLen = 0;
                inputLine[0] = '\0';
            }
        }

        // Terminal window dragging and close button
        if (terminal_open) {
            Vector2 m = GetMousePosition();
            Rectangle titleBar = (Rectangle){ termWin.x, termWin.y, termWin.width, 28.0f };
            Rectangle closeBtn = (Rectangle){ termWin.x + termWin.width - 32, termWin.y, 28, 28 };
            close_hover = CheckCollisionPointRec(m, closeBtn);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(m, closeBtn)) {
                    // Only close terminal window, not the whole GUI
                    terminal_open = 0;
                    CloseShell(&shell);
                } else if (CheckCollisionPointRec(m, titleBar)) {
                    dragging = 1;
                    dragOffset.x = m.x - termWin.x;
                    dragOffset.y = m.y - termWin.y;
                }
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = 0;
            if (dragging) {
                termWin.x = m.x - dragOffset.x;
                termWin.y = m.y - dragOffset.y;
            }
        }

        // Read real shell output and append to terminal buffer
        if (terminal_open && shell.running) {
            char chunk[READ_CHUNK_SIZE];
            int got = ReadShellOutput(&shell, chunk, sizeof(chunk));
            if (got > 0) {
                int start = 0;
                for (int i = 0; i < got; i++) {
                    if (chunk[i] == '\r') continue;
                    if (chunk[i] == '\n') {
                        int len = i - start;
                        if (len > 0) {
                            int copyLen = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                            if (lineCount < TERM_MAX_LINES) {
                                memcpy(lines[lineCount], &chunk[start], copyLen);
                                lines[lineCount][copyLen] = '\0';
                                lineCount++;
                            } else {
                                memmove(lines, lines + 1, sizeof(lines[0]) * (TERM_MAX_LINES - 1));
                                int copyLen2 = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                                memcpy(lines[TERM_MAX_LINES - 1], &chunk[start], copyLen2);
                                lines[TERM_MAX_LINES - 1][copyLen2] = '\0';
                            }
                        } else {
                            if (lineCount < TERM_MAX_LINES) {
                                lines[lineCount][0] = '\0';
                                lineCount++;
                            }
                        }
                        start = i + 1;
                    }
                }
                if (start < got) {
                    int len = got - start;
                    int copyLen = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                    if (lineCount == 0 || lines[lineCount - 1][0] == '\0') {
                        if (lineCount < TERM_MAX_LINES) {
                            memcpy(lines[lineCount], &chunk[start], copyLen);
                            lines[lineCount][copyLen] = '\0';
                            lineCount++;
                        }
                    } else {
                        int curLen = (int)strlen(lines[lineCount - 1]);
                        int space  = TERM_MAX_COLUMNS - 1 - curLen;
                        int append = (copyLen < space) ? copyLen : space;
                        memcpy(lines[lineCount - 1] + curLen, &chunk[start], append);
                        lines[lineCount - 1][curLen + append] = '\0';
                    }
                }
            }
        }

        // Keyboard input into terminal (always active when terminal is open)
        if (terminal_open) {
            int c;
            char tempInput[TERM_MAX_COLUMNS];
            while ((c = GetCharPressed()) > 0) {
                if (c >= 32 && c < 127) {
                    if (inputLen < TERM_MAX_COLUMNS - 1) {
                        inputLine[inputLen++] = (char)c;
                        inputLine[inputLen] = '\0';
                    }
                }
            }
            // Check for ENTER using IsKeyPressed
            if (IsKeyPressed(KEY_ENTER)) {
                // Store input in temp before clearing
                strncpy(tempInput, inputLine, TERM_MAX_COLUMNS-1);
                tempInput[TERM_MAX_COLUMNS-1] = '\0';
                if (shell.running) {
                    char toSend[TERM_MAX_COLUMNS + 2];
                    int sendLen = snprintf(toSend, sizeof(toSend), "%s\r\n", inputLine);
                    WriteShellInput(&shell, toSend, sendLen);
                }
                inputLen = 0;
                inputLine[0] = '\0';
                // Always append tempInput to buffer after clearing
                if (lineCount < TERM_MAX_LINES) {
                    int maxPrompt = TERM_MAX_COLUMNS - 2;
                    snprintf(lines[lineCount], TERM_MAX_COLUMNS, "> %.*s", maxPrompt, tempInput);
                    lineCount++;
                }
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (inputLen > 0) {
                    inputLen--;
                    inputLine[inputLen] = '\0';
                }
            }
        }

        BeginDrawing();
        // X11 blue background
        ClearBackground(x11_blue);

        // Top bar (X11 style)
        DrawRectangle(0, 0, screenWidth, topBarH, x11_title);
        // Draw a smaller X logo, left-aligned
        float logoScaleBar = 0.18f;
        int logoBarW = (int)(logo.width * logoScaleBar);
        int logoBarH = (int)(logo.height * logoScaleBar);
        int logoBarX = 12;
        int logoBarY = (topBarH - logoBarH) / 2;
        DrawTextureEx(logo, (Vector2){(float)logoBarX, (float)logoBarY}, 0.0f, logoScaleBar, x11_white);
        // Draw X11 and WNU OS X11 text in white/gray, spaced right of logo
        int textX = logoBarX + logoBarW + 18;
        int textY = logoBarY + 2;
        DrawText("X11", textX, textY, 28, x11_white);
        DrawText("WNU OS X11", textX + 70, textY + 4, 20, x11_gray);

        // Desktop background
        DrawRectangle(0, topBarH, screenWidth, screenHeight - topBarH, x11_gray);
        DrawText("Welcome to X11 Desktop", (int)(screenWidth*0.06f), (int)(screenHeight*0.08f), (int)(topBarH*0.7f), x11_title);

        // Terminal icon
        DrawTextureEx(logo, (Vector2){(float)icon_x, (float)icon_y}, 0.0f, iconScale, x11_white);
        DrawText("xterm", icon_x, icon_y + icon_h + 8, 18, x11_title);

        // Draw context menu if active
        if (showContextMenu) {
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            int menuW = 180, menuH = 32;
            Color menuBg = (Color){60, 60, 70, 255};
            Color menuBorder = (Color){80, 80, 120, 255};
            Color menuHighlight = (Color){40, 60, 180, 255};
            Vector2 mouse = GetMousePosition();
            Rectangle xtermRect = {menuX, menuY, menuW, menuH};
            Rectangle aboutRect = {menuX, menuY + menuH, menuW, menuH};
            Rectangle exitRect  = {menuX, menuY + 2*menuH, menuW, menuH};
            int hoverXterm = CheckCollisionPointRec(mouse, xtermRect);
            int hoverAbout = CheckCollisionPointRec(mouse, aboutRect);
            int hoverExit  = CheckCollisionPointRec(mouse, exitRect);
            // Draw XTerm item
            DrawRectangleRec(xtermRect, hoverXterm ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY, menuW, menuH, menuBorder);
            DrawText("XTerm", menuX + 12, menuY + 7, 18, x11_white);
            // Draw About item
            DrawRectangleRec(aboutRect, hoverAbout ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY + menuH, menuW, menuH, menuBorder);
            DrawText("About X11 Desktop", menuX + 12, menuY + menuH + 7, 18, x11_white);
            // Draw Exit item
            DrawRectangleRec(exitRect, hoverExit ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY + 2*menuH, menuW, menuH, menuBorder);
            DrawText("Exit X11", menuX + 12, menuY + 2*menuH + 7, 18, x11_white);
        }

        // Terminal window
        if (terminal_open) {
            // Window border and background
            int border = (int)(screenWidth * 0.002f);
            int titleH = (int)(termWin.height * 0.07f);
            DrawRectangleRec(termWin, x11_border);
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, (int)termWin.height - 2*border, x11_term_bg);
            // Title bar
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, titleH, x11_title);
            DrawText("xterm", (int)termWin.x + 16, (int)termWin.y + border + 8, (int)(titleH*0.6f), x11_white);
            // Close button
            int closeBtnSz = titleH-4;
            DrawRectangleRec((Rectangle){termWin.x + termWin.width - closeBtnSz - border, termWin.y + border + 2, closeBtnSz, closeBtnSz}, close_hover ? RED : x11_dkgray);
            DrawText("X", (int)(termWin.x + termWin.width - closeBtnSz/1.5f - border), (int)(termWin.y + border + 6), (int)(titleH*0.5f), x11_white);

            // Terminal contents
            int y = (int)(termWin.y + titleH + 12);
            int x = (int)(termWin.x + 16);
            int maxY = (int)(termWin.y + termWin.height - 36);
            int first = 0;
            int lineH = (int)(termWin.height * 0.045f);
            int maxLinesOnScreen = (maxY - y) / lineH;
            if (lineCount > maxLinesOnScreen) first = lineCount - maxLinesOnScreen;
            for (int i = first; i < lineCount; i++) {
                DrawTextEx(GetFontDefault(), lines[i], (Vector2){(float)x, (float)y}, (float)lineH, 1.0f, x11_term_fg);
                y += lineH;
                if (y > maxY) break;
            }
            // Input prompt
            int promptH = (int)(termWin.height * 0.055f);
            DrawRectangle(x - 4, maxY + 8, (int)termWin.width - 32, promptH, x11_dkgray);
            // Always draw prompt and current input at the bottom, not as a line in the buffer
            DrawText("> ", x, maxY + 10, (int)(promptH*0.7f), x11_white);
            DrawText(inputLine, x + MeasureText("> ", (int)(promptH*0.7f)), maxY + 10, (int)(promptH*0.7f), x11_white);
        }

        // Debug overlay if window close event triggered
        if (debug_window_should_close) {
            DrawRectangle(0, 0, 400, 40, RED);
            DrawText("[DEBUG] WindowShouldClose() triggered!", 10, 10, 20, WHITE);
        }
        EndDrawing();

        // If child process exits, close it and keep terminal window open
        // Only check for shell exit if the terminal is open and shell was running
        if (terminal_open && shell.running) {
            DWORD status = WaitForSingleObject(shell.hProcess, 0);
            if (status == WAIT_OBJECT_0) {
                CloseShell(&shell);
                shell.running = 0;
                // Show message in terminal buffer
                if (lineCount < TERM_MAX_LINES) {
                    snprintf(lines[lineCount], TERM_MAX_COLUMNS, "[X11] Shell exited. Click Terminal icon to restart.");
                    Sleep(100); // Small delay to ensure message is seen
                    terminal_open = 0;
                    lineCount++;

                }
            }
        }
    }

    // Cleanup
    CloseShell(&shell);
    UnloadTexture(logo);

        ((void (*)(void))CloseWindow)();
        return 0;
}

#else /* NO_RAYLIB build (shell-only) */

#include <windows.h>

typedef struct {
    HANDLE hProcess;
    HANDLE hInput;
    HANDLE hOutput;
    int running;
} ChildProc;

// (Implementations for LaunchShell, ReadShellOutput, WriteShellInput, CloseShell go here for shell-only build)

int x11(void) {
    /* Fallback: launch external GUI if someone calls x11() in shell build */

    int rc = system("cmd /c start \"\" \"C:\\WNU\\WNU OS\\x11.exe\"");
    (void)rc;
    return 0;
}



#endif /* NO_RAYLIB */
