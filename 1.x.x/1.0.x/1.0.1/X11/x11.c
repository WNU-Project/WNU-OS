/*
 * Integrated GUI terminal:
 * - Raylib desktop with a Terminal icon.
 * - Clicking the icon opens a draggable terminal window.
 * - The window runs C:\WNU\WNU OS\wnuos.exe "inside" the GUI via pipes
 *   (no external console window is created).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef NO_RAYLIB  /* GUI build only */

#include "raylib.h"
#include "x11_logo.h"   /* generated from X11.png via xxd -i or Python */

/* Prevent Windows/Raylib symbol collisions */
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOMINMAX
#include <windows.h>

#define TERM_MAX_LINES      1024
#define TERM_MAX_COLUMNS    512
#define READ_CHUNK_SIZE     4096

typedef struct {
    HANDLE hProcess;
    HANDLE hThread;
    HANDLE hStdOutRd; /* GUI reads from this */
    HANDLE hStdOutWr; /* child writes to this */
    HANDLE hStdInRd;  /* child reads from this */
    HANDLE hStdInWr;  /* GUI writes to this */
    BOOL   running;
} ChildProc;

/* Create child process with redirected stdin/stdout, no external console */
static BOOL LaunchShell(ChildProc *cp, const char *cmdline) {
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    /* Create pipes: stdout (child -> GUI) and stdin (GUI -> child) */
    if (!CreatePipe(&cp->hStdOutRd, &cp->hStdOutWr, &sa, 0)) return FALSE;
    if (!SetHandleInformation(cp->hStdOutRd, HANDLE_FLAG_INHERIT, 0)) return FALSE;

    if (!CreatePipe(&cp->hStdInRd, &cp->hStdInWr, &sa, 0)) return FALSE;
    if (!SetHandleInformation(cp->hStdInWr, HANDLE_FLAG_INHERIT, 0)) return FALSE;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = cp->hStdInRd;
    si.hStdOutput = cp->hStdOutWr;
    si.hStdError  = cp->hStdOutWr;

    /* Mutable command line required by CreateProcessA */
    char mutableCmd[512];
    snprintf(mutableCmd, sizeof(mutableCmd), "%s", cmdline);

    BOOL ok = CreateProcessA(
        NULL,
        mutableCmd,
        NULL, NULL,
        TRUE, /* inherit handles so child sees our pipe ends */
        CREATE_NO_WINDOW, /* run without spawning a separate console window */
        NULL, NULL,
        &si, &pi
    );

    if (!ok) {
        /* Cleanup pipe ends on failure */
        CloseHandle(cp->hStdOutRd);
        CloseHandle(cp->hStdOutWr);
        CloseHandle(cp->hStdInRd);
        CloseHandle(cp->hStdInWr);
        ZeroMemory(cp, sizeof(*cp));
        return FALSE;
    }

    /* Save handles/process info */
    cp->hProcess = pi.hProcess;
    cp->hThread  = pi.hThread;
    cp->running  = TRUE;

    /* Parent doesn't need child's ends of the pipes */
    CloseHandle(cp->hStdOutWr);
    CloseHandle(cp->hStdInRd);

    return TRUE;
}

static void CloseShell(ChildProc *cp) {
    if (!cp || !cp->running) return;
    /* Try to close stdin to signal EOF */
    CloseHandle(cp->hStdInWr);

    /* Wait for process to exit and clean up */
    WaitForSingleObject(cp->hProcess, 2000);
    CloseHandle(cp->hProcess);
    CloseHandle(cp->hThread);
    CloseHandle(cp->hStdOutRd);
    ZeroMemory(cp, sizeof(*cp));
}

/* Non-blocking read from child's stdout */
static int ReadShellOutput(ChildProc *cp, char *buffer, int bufSize) {
    if (!cp || !cp->running) return 0;
    DWORD avail = 0;
    if (!PeekNamedPipe(cp->hStdOutRd, NULL, 0, NULL, &avail, NULL)) return 0;
    if (avail == 0) return 0;
    DWORD toRead = (avail > (DWORD)bufSize) ? (DWORD)bufSize : avail;
    DWORD read = 0;
    if (!ReadFile(cp->hStdOutRd, buffer, toRead, &read, NULL)) return 0;
    return (int)read;
}

/* Write a command line (with newline) to child's stdin */
static BOOL WriteShellInput(ChildProc *cp, const char *text, int len) {
    if (!cp || !cp->running) return FALSE;
    DWORD written = 0;
    return WriteFile(cp->hStdInWr, text, (DWORD)len, &written, NULL);
}

int x11(void) {
    const int screenWidth  = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "WNU OS X11");
    SetTargetFPS(60);

    /* Load embedded logo */
    Image     logoImage = LoadImageFromMemory(".png", X11_png, X11_png_len);
    Texture2D icon      = LoadTextureFromImage(logoImage);
    UnloadImage(logoImage);

    const float iconScale = 0.5f;
    int   icon_x = 32;
    int   icon_y = 80;
    int   icon_w = (int)(icon.width  * iconScale);
    int   icon_h = (int)(icon.height * iconScale);

    /* Terminal window state */
    int terminal_open = 0;
    Rectangle termWin = (Rectangle){ 300.0f, 200.0f, 640.0f, 420.0f };
    int dragging = 0;
    Vector2 dragOffset = {0};

    /* Terminal content buffer */
    char  lines[TERM_MAX_LINES][TERM_MAX_COLUMNS];
    int   lineCount = 0;
    char  inputLine[TERM_MAX_COLUMNS] = {0};
    int   inputLen = 0;

    ChildProc shell = {0};

    while (!WindowShouldClose()) {
        /* Handle click on terminal icon to launch shell in-window */
        if (!terminal_open && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (mouse.x >= icon_x && mouse.x <= icon_x + icon_w &&
                mouse.y >= icon_y && mouse.y <= icon_y + icon_h) {
                terminal_open = 1;
                /* Launch your shell without external console; pipes connected */
                LaunchShell(&shell, "\"C:\\WNU\\WNU OS\\wnuos.exe\"");
                /* Clear terminal buffer */
                lineCount = 0;
                inputLen = 0;
                inputLine[0] = '\0';
            }
        }

        /* Terminal window dragging */
        if (terminal_open) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 m = GetMousePosition();
                Rectangle titleBar = (Rectangle){ termWin.x, termWin.y, termWin.width, 24.0f };
                if (m.x >= titleBar.x && m.x <= titleBar.x + titleBar.width &&
                    m.y >= titleBar.y && m.y <= titleBar.y + titleBar.height) {
                    dragging = 1;
                    dragOffset.x = m.x - termWin.x;
                    dragOffset.y = m.y - termWin.y;
                }
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = 0;
            if (dragging) {
                Vector2 m = GetMousePosition();
                termWin.x = m.x - dragOffset.x;
                termWin.y = m.y - dragOffset.y;
            }
        }

        /* Read real shell output and append to terminal buffer */
        if (terminal_open && shell.running) {
            char chunk[READ_CHUNK_SIZE];
            int got = ReadShellOutput(&shell, chunk, sizeof(chunk));
            if (got > 0) {
                /* Split chunk into lines; append to buffer */
                int start = 0;
                for (int i = 0; i < got; i++) {
                    if (chunk[i] == '\r') continue; /* ignore CR on Windows */
                    if (chunk[i] == '\n') {
                        int len = i - start;
                        if (len > 0) {
                            int copyLen = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                            if (lineCount < TERM_MAX_LINES) {
                                memcpy(lines[lineCount], &chunk[start], copyLen);
                                lines[lineCount][copyLen] = '\0';
                                lineCount++;
                            } else {
                                /* scroll up: drop first line */
                                memmove(lines, lines + 1, sizeof(lines[0]) * (TERM_MAX_LINES - 1));
                                int copyLen2 = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                                memcpy(lines[TERM_MAX_LINES - 1], &chunk[start], copyLen2);
                                lines[TERM_MAX_LINES - 1][copyLen2] = '\0';
                            }
                        } else {
                            /* blank line */
                            if (lineCount < TERM_MAX_LINES) {
                                lines[lineCount][0] = '\0';
                                lineCount++;
                            }
                        }
                        start = i + 1;
                    }
                }
                /* Any remaining partial line (no newline yet) */
                if (start < got) {
                    int len = got - start;
                    int copyLen = (len < TERM_MAX_COLUMNS - 1) ? len : (TERM_MAX_COLUMNS - 1);
                    /* Append to the last line if it exists and isn't full */
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

            /* Keyboard input into terminal */
            int c;
            while ((c = GetCharPressed()) > 0) {
                if (c == 8) { /* backspace */
                    if (inputLen > 0) {
                        inputLen--;
                        inputLine[inputLen] = '\0';
                    }
                } else if (c == '\r' || c == '\n') {
                    /* Echo input line into buffer and send to child with newline */
                    if (lineCount < TERM_MAX_LINES) {
                        snprintf(lines[lineCount], TERM_MAX_COLUMNS, "> %s", inputLine);
                        lineCount++;
                    }
                    char toSend[TERM_MAX_COLUMNS + 2];
                    int sendLen = snprintf(toSend, sizeof(toSend), "%s\r\n", inputLine);
                    WriteShellInput(&shell, toSend, sendLen);
                    inputLen = 0;
                    inputLine[0] = '\0';
                } else if (c >= 32 && c < 127) { /* printable ASCII */
                    if (inputLen < TERM_MAX_COLUMNS - 1) {
                        inputLine[inputLen++] = (char)c;
                        inputLine[inputLen] = '\0';
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        /* Top bar */
        DrawRectangle(0, 0, screenWidth, 32, DARKGRAY);
        DrawTextEx(GetFontDefault(), "WNU OS X11 GUI", (Vector2){10.0f, 8.0f}, 16.0f, 1.0f, RAYWHITE);

        /* Desktop background */
        DrawRectangle(0, 32, screenWidth, screenHeight - 32, LIGHTGRAY);
        DrawText("Welcome to WNU OS X11 GUI (raylib)", 40, 60, 20, DARKGRAY);

        /* Terminal icon */
        DrawTextureEx(icon, (Vector2){(float)icon_x, (float)icon_y}, 0.0f, iconScale, WHITE);
        DrawText("Terminal", icon_x, icon_y + icon_h + 2, 12, DARKGRAY);

        /* Terminal window */
        if (terminal_open) {
            DrawRectangleRec(termWin, BLACK);
            DrawRectangle((int)termWin.x, (int)termWin.y, (int)termWin.width, 24, DARKGRAY);
            DrawText("Terminal", (int)(termWin.x + 8), (int)(termWin.y + 4), 16, RAYWHITE);

            /* Draw terminal contents (real shell output) */
            int y = (int)(termWin.y + 32);
            int x = (int)(termWin.x + 8);
            int maxY = (int)(termWin.y + termWin.height - 36);

            /* Show lines from bottom if too many: simple viewport */
            int first = 0;
            int maxLinesOnScreen = (maxY - y) / 18;
            if (lineCount > maxLinesOnScreen) first = lineCount - maxLinesOnScreen;

            for (int i = first; i < lineCount; i++) {
                DrawText(lines[i], x, y, 14, GREEN);
                y += 18;
                if (y > maxY) break;
            }

            /* Draw input prompt */
            DrawRectangle(x - 4, maxY + 8, (int)termWin.width - 16, 20, (Color){32,32,32,255});
            DrawText(TextFormat("~$ %s", inputLine), x, maxY + 10, 14, RAYWHITE);
        }

        EndDrawing();

        /* If child process exits, close it and keep window open (optional) */
        if (shell.running) {
            DWORD status = WaitForSingleObject(shell.hProcess, 0);
            if (status == WAIT_OBJECT_0) {
                CloseShell(&shell);
            }
        }
    }

    /* Cleanup */
    CloseShell(&shell);
    UnloadTexture(icon);
    CloseWindow();
    return 0;
}

#else  /* NO_RAYLIB build (shell-only) */

int x11(void) {
    /* Fallback: launch external GUI if someone calls x11() in shell build */
    int rc = system("cmd /c start \"\" \"C:\\WNU\\WNU OS\\x11.exe\"");
    (void)rc;
    return 0;
}

#endif
