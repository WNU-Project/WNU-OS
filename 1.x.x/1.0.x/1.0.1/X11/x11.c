#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

// Include raylib first
#include "raylib.h"

// Only include what we need from Windows API
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windef.h>
#include <winbase.h>
#include <winver.h>

// Avoid the conflicting Windows APIs by using only what we need

#include "x11.h"
#include "x11_logo.h"
#include "xterm_logo.h"
#include "xclock_logo.h"
#include "xcalc_logo.h"
#include "xcalc.h"
#include "shell_win.h"

// --- Terminal buffer and shell process definitions ---
#define TERM_MAX_LINES    512
#define TERM_MAX_COLUMNS  256
#define READ_CHUNK_SIZE   512



// The Windows-specific implementation lives in X11/shell_win.c. Include the
// wrapper header and treat ChildProc as an opaque pointer in this file.

// Reintroduce a compact, self-contained xcalc widget implementation here.
// This lightweight version is intentionally smaller and simpler than the
// original to make isolation/debugging easier while preserving the UI and
// interactions (open/close, drag, resize, basic calculator operations).
static void draw_xcalc_widget(Rectangle *xcalcWin, int *xcalc_open, int *xcalc_minimized, int *xcalc_resizing, int *xcalc_close_hover,
                              int *xcalc_workspace, int *xcalc_sticky, Font guiFont, Texture2D xcalclogo,
                              int *last_clicked, int WIN_XCALC,
                              int *focused_window, int workspace, int screenWidth, int screenHeight) {
    if (!xcalcWin || !xcalc_open) return;

    // Persistent widget state (kept inside function for simplicity)
    static int drag = 0;
    static Vector2 dragOffset = {0,0};
    static char display[128] = "0";
    static double pending = 0.0;
    static char op = 0; // '+','-','*','/' or 0
    static int entering = 0;
    static int error = 0;

    int xb = (int)(screenWidth * 0.002f);
    int titleH = 28;

    // If window is open and visible on current workspace (or sticky)
    if (*xcalc_open && !*xcalc_minimized && (*xcalc_workspace == workspace || *xcalc_sticky)) {
        // Ensure default size
        if (xcalcWin->width < 220) xcalcWin->width = 220;
        if (xcalcWin->height < 200) xcalcWin->height = 200;

        // Window frame
        DrawRectangle((int)xcalcWin->x, (int)xcalcWin->y, (int)xcalcWin->width, (int)xcalcWin->height, (Color){80,80,120,255});
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, (int)xcalcWin->height - 2*xb, (Color){255,255,255,255});
        // Title bar
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, titleH, (Color){60,60,80,255});
        DrawTextEx(guiFont, "xcalc", (Vector2){xcalcWin->x + xb + 8, xcalcWin->y + xb + 6}, 14.0f, 0.0f, (Color){255,255,255,255});

        // Close button
        int closeSz = titleH - 6;
        int closeX = (int)(xcalcWin->x + xcalcWin->width - closeSz - xb - 4);
        int closeY = (int)(xcalcWin->y + xb + 3);
        Vector2 mpos = GetMousePosition();
        *xcalc_close_hover = CheckCollisionPointRec(mpos, (Rectangle){(float)closeX,(float)closeY,(float)closeSz,(float)closeSz});
        DrawRectangle(closeX, closeY, closeSz, closeSz, *xcalc_close_hover ? RED : (Color){120,120,120,255});
        DrawTextEx(guiFont, "X", (Vector2){(float)(closeX+4),(float)(closeY+2)}, 14.0f, 0.0f, (Color){255,255,255,255});

        // Display area
        float clientX = xcalcWin->x + xb + 8;
        float clientY = xcalcWin->y + xb + titleH + 8;
        float clientW = xcalcWin->width - 2*(xb + 8);
        float dispH = 48;
        Rectangle disp = { clientX, clientY, clientW, dispH };
        DrawRectangleRec(disp, (Color){18,18,18,255});
        DrawRectangleLinesEx(disp, 2, (Color){120,120,160,255});
        float df = 24.0f;
        Vector2 dsz = MeasureTextEx(guiFont, display, df, 0.0f);
        DrawTextEx(guiFont, display, (Vector2){ disp.x + disp.width - dsz.x - 12, disp.y + (disp.height - dsz.y)/2 }, df, 0.0f, (Color){255,255,255,255});

        // Buttons grid (simple 4x5 layout)
        const char* labels[] = {"7","8","9","/","4","5","6","*","1","2","3","-","0",".","=","+","AC","CE","+/-","%"};
        int n = sizeof(labels)/sizeof(labels[0]);
        int cols = 4;
        int rows = (n + cols - 1) / cols;
        float gap = 8.0f;
        float btnW = (clientW - (cols-1)*gap) / cols;
        float btnH = (xcalcWin->height - (titleH + dispH + 8 + xb + 24) - (rows+1)*gap) / rows;
        float bx0 = clientX;
        float by0 = clientY + dispH + gap;

        // Click handling
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int r=0;r<rows;r++) for (int c=0;c<cols;c++) {
                int idx = r*cols + c;
                if (idx >= n) continue;
                Rectangle br = { bx0 + c*(btnW+gap), by0 + r*(btnH+gap), btnW, btnH };
                if (CheckCollisionPointRec(mpos, br)) {
                    const char* lab = labels[idx];
                    // Handle button actions
                    if (strcmp(lab,"AC")==0) { strcpy(display,"0"); pending=0.0; op=0; entering=0; error=0; }
                    else if (strcmp(lab,"CE")==0) { strcpy(display,"0"); entering=0; error=0; }
                    else if (strcmp(lab,"+/-")==0) { if (display[0]=='-') memmove(display, display+1, strlen(display)); else if (strcmp(display,"0")!=0) { char t[128]; snprintf(t,sizeof(t),"-%s",display); strncpy(display,t,sizeof(display)-1);} }
                    else if (strcmp(lab,"%")==0) { double v = atof(display); v = v/100.0; snprintf(display,sizeof(display),"%g",v); entering=0; }
                    else if (strcmp(lab,"=")==0) {
                        double cur = atof(display);
                        double res = cur;
                        if (op) {
                            if (op=='+') res = pending + cur;
                            else if (op=='-') res = pending - cur;
                            else if (op=='*') res = pending * cur;
                            else if (op=='/') { if (cur==0.0) { error=1; strncpy(display,"ERR",sizeof(display)-1);} else res = pending / cur; }
                            if (!error) snprintf(display,sizeof(display),"%g",res);
                            pending = res; op=0; entering=0;
                        }
                    } else if (strchr("+-*/", lab[0]) && lab[1]=='\0') {
                        double cur = atof(display);
                        if (op) {
                            double res = cur;
                            if (op=='+') res = pending + cur;
                            else if (op=='-') res = pending - cur;
                            else if (op=='*') res = pending * cur;
                            else if (op=='/') { if (cur==0.0) { error=1; strncpy(display,"ERR",sizeof(display)-1);} else res = pending / cur; }
                            if (!error) snprintf(display,sizeof(display),"%g",res);
                            pending = res;
                        } else pending = cur;
                        op = lab[0]; entering=0;
                    } else if (strcmp(lab,".")==0) {
                        if (!strchr(display,'.')) { if (!entering) { strncpy(display,"0",sizeof(display)-1); entering=1; } strncat(display,".", sizeof(display)-strlen(display)-1); }
                    } else { // digits
                        if (!entering || (strcmp(display,"0")==0 && strchr("0123456789", lab[0]))) { snprintf(display,sizeof(display),"%s",lab); entering=1; }
                        else { strncat(display, lab, sizeof(display)-strlen(display)-1); }
                    }
                }
            }
        }

        // Draw buttons
        for (int r=0;r<rows;r++) for (int c=0;c<cols;c++) {
            int idx = r*cols + c; if (idx >= n) continue;
            Rectangle br = { bx0 + c*(btnW+gap), by0 + r*(btnH+gap), btnW, btnH };
            int hov = CheckCollisionPointRec(mpos, br);
            DrawRectangleRec(br, hov ? (Color){60,80,140,255} : (Color){220,220,230,255});
            DrawRectangleLinesEx(br, 1, (Color){80,80,100,255});
            Vector2 lsz = MeasureTextEx(guiFont, labels[idx], 18.0f, 0.0f);
            DrawTextEx(guiFont, labels[idx], (Vector2){ br.x + (br.width - lsz.x)/2, br.y + (br.height - lsz.y)/2 }, 18.0f, 0.0f, (Color){30,30,40,255});
        }

        // Window interactions: close, drag, resize
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mpos, (Rectangle){(float)closeX,(float)closeY,(float)closeSz,(float)closeSz})) {
                *xcalc_open = 0; if (*last_clicked == WIN_XCALC) *last_clicked = -1;
            } else {
                Rectangle titleRect = {(float)xcalcWin->x + xb, (float)xcalcWin->y + xb, (float)xcalcWin->width - 2*xb, (float)titleH};
                if (CheckCollisionPointRec(mpos, titleRect)) { drag = 1; dragOffset.x = mpos.x - xcalcWin->x; dragOffset.y = mpos.y - xcalcWin->y; *last_clicked = WIN_XCALC; }
                else {
                    Rectangle rsz = { xcalcWin->x + xcalcWin->width - 18, xcalcWin->y + xcalcWin->height - 18, 18, 18 };
                    if (CheckCollisionPointRec(mpos, rsz)) { *xcalc_resizing = 1; *last_clicked = WIN_XCALC; *focused_window = WIN_XCALC; }
                }
            }
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) { drag = 0; *xcalc_resizing = 0; }
        if (drag && *last_clicked == WIN_XCALC) { Vector2 mm = GetMousePosition(); xcalcWin->x = mm.x - dragOffset.x; xcalcWin->y = mm.y - dragOffset.y; }
        if (*xcalc_resizing) { Vector2 rm = GetMousePosition(); float nw = rm.x - xcalcWin->x; if (nw < 220) nw = 220; if (nw > screenWidth - xcalcWin->x) nw = screenWidth - xcalcWin->x - 8; float nh = rm.y - xcalcWin->y; if (nh < 200) nh = 200; if (nh > screenHeight - xcalcWin->y) nh = screenHeight - xcalcWin->y - 8; xcalcWin->width = nw; xcalcWin->height = nh; }
    }

    // Draw minimized icon if iconified
    if (*xcalc_minimized) {
        int icon_x = 32;
        int icon_y = (int)(48 + 32);
        int minIconX = icon_x + (int)(32 * 0.18f) + 24;
        int minIconY = icon_y;
        DrawTextureEx(xcalclogo, (Vector2){(float)minIconX, (float)minIconY}, 0.0f, 0.18f, (Color){255,255,255,255});
        DrawTextEx(guiFont, "xcalc", (Vector2){(float)minIconX, (float)(minIconY + (int)(xcalclogo.height*0.18f) + 4)}, 14.0f, 0.0f, (Color){30,30,40,255});
    }
}

int x11(void) {
    char currentos_buf[128] = {0};
    /* Keep Windows API usage out of this file; use a stable string here. */
    strncpy(currentos_buf, "Windows (unknown version)", sizeof(currentos_buf)-1);
    currentos_buf[sizeof(currentos_buf)-1] = '\0';
    char* currentos = currentos_buf;

    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    if (GetVersionExW((OSVERSIONINFOW*)&osvi)) {
        printf("[Windows version: %lu.%lu (build %lu), platform %lu\n",
            osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, osvi.dwPlatformId);
    } else {
        printf("Could not determine Windows version.\n");
    }
    // Removed debug and informational prints for production build
    int initialScreenWidth  = 1024;
    int initialScreenHeight = 768;
    // Context menu state (must be after raylib include)
    int showContextMenu = 0;
    Vector2 contextMenuPos = {0};
    int contextMenuHover = -1;
    int contextMenuSetFrame = -1;
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_RESIZABLE);
    InitWindow(initialScreenWidth, initialScreenHeight, "X11 Desktop (FVWM 3.x)");
    SetWindowFocused();
    SetTargetFPS(60);

    // X11 color palette
    Color x11_blue    = (Color){ 40,  60, 180, 255}; // X11 blue
    Color x11_gray    = (Color){200, 200, 210, 255}; // X11 light gray
    Color x11_dkgray  = (Color){ 60,  60,  70, 255}; // X11 dark gray
    Color x11_white   = (Color){255, 255, 255, 255};
    Color x11_border  = (Color){ 80,  80, 120, 255};
    Color x11_title   = (Color){ 30,  30,  60, 255};
    Color x11_term_bg = (Color){ 18,  18,  18, 255};
    Color x11_term_fg = (Color){  0, 255, 128, 255};

    // Load resources ONCE before main loop
    Image logoImage = LoadImageFromMemory(".png", X11_png, X11_png_len);
    Texture2D logo  = LoadTextureFromImage(logoImage);
    Image xtermlgoImage = LoadImageFromMemory(".png", xterm_png, xterm_png_len);
    Texture2D xtermlogo  = LoadTextureFromImage(xtermlgoImage);
    Image xclocklgoImage = LoadImageFromMemory(".png", xclock_png, xclock_png_len);
    Texture2D xclocklogo  = LoadTextureFromImage(xclocklgoImage);
    Image xcalclgoImage = LoadImageFromMemory(".png", xcalc_png, xcalc_png_len);
    Texture2D xcalclogo  = LoadTextureFromImage(xcalclgoImage);
    UnloadImage(logoImage);
    UnloadImage(xtermlgoImage);
    UnloadImage(xclocklgoImage);
    UnloadImage(xcalclgoImage);
    // Load GUI font (fall back to a common Windows font)
    Font guiFont = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, NULL, 0);

    // Desktop icon (move lower, right of top bar)
    const float iconScale = 0.18f;
    int topBarH = 48;
    int terminal_open = 0;
    int xclock_open = 0;
    Rectangle termWin = {0};
    Rectangle xclockWin = {0};
    int xcalc_open = 0;
    Rectangle xcalcWin = {0};
    int xcalc_minimized = 0;
    int xcalc_resizing = 0;
    int xcalc_close_hover = 0;
    // xcalc_dragging and xcalcDragOffset are now managed inside draw_xcalc_widget
    /* Calculator state moved into draw_xcalc_widget(); no outer duplicates */
    int utilities_open = 0;
    Rectangle utilitiesWin = {0};
    int dragging = 0;
    int xclock_dragging = 0;
    int utilities_dragging = 0;
    Vector2 dragOffset = {0};
    Vector2 xclockDragOffset = {0};
    Vector2 utilitiesDragOffset = {0};
    int close_hover = 0;
    int xclock_close_hover = 0;
    int utilities_close_hover = 0;

    /* FVWM-like extra state */
    int focus_follows_mouse = 0; /* toggleable */
    int focused_window = -1;     /* WIN_* id currently focused for keyboard ops */
    /* resize state per-window */
    int term_resizing = 0;
    int xclock_resizing = 0;
    int utilities_resizing = 0;
    /* minimized/iconified flags */
    int xclock_minimized = 0;
    int utilities_minimized = 0;
    /* sticky windows (appear on all workspaces) */
    int term_sticky = 0;
    int xclock_sticky = 0;
    int utilities_sticky = 0;

    // Terminal content buffer
    char  lines[TERM_MAX_LINES][TERM_MAX_COLUMNS];
    int   lineCount = 0;
    char  inputLine[TERM_MAX_COLUMNS] = {0};
    int   inputLen = 0;

    ChildProc* shell = NULL;

    int running = 1;
    int debug_window_should_close = 0;
    (void)debug_window_should_close; // intentionally kept for future debug hooks
    (void)x11_border; // silence unused-variable warning; keep color available for future UI tweaks
    int frameCount = 0;
    int terminal_minimized = 0;
    int workspace = 1, workspaceCount = 4;
    /* Window IDs for z-order/overlap handling */
    #define WIN_TERM 0
    #define WIN_XCLOCK 1
    #define WIN_UTILS 2
    #define WIN_XCALC 3
    int last_clicked = -1; /* last clicked window id; drawn on top */
    /* Per-window workspace assignments (FVWM-style virtual desktops) */
    int term_workspace = 1;
    int xclock_workspace = 1;
    int utilities_workspace = 1;
    int xcalc_workspace = 1;
    int xcalc_sticky = 0;
    // Main loop
    while (running && !WindowShouldClose()) {
        frameCount++;
        
        // Begin drawing
        BeginDrawing();
        ClearBackground(x11_blue); // X11 blue background
        
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        topBarH = (int)(48 * (screenHeight / 768.0f));
        
        // Draw FVWM-like top bar
        DrawRectangle(0, 0, screenWidth, topBarH, (Color){60, 60, 80, 255});
        
        // Menu button (left)
        int menuBtnH = topBarH - 8;
        int menuBtnW = menuBtnH; // square button
        int menuBtnX = 8, menuBtnY = 4;
        DrawRectangle(menuBtnX, menuBtnY, menuBtnW, menuBtnH, (Color){80, 80, 120, 255});
        DrawTextEx(guiFont, "≡", (Vector2){(float)(menuBtnX + menuBtnW/4), (float)(menuBtnY + menuBtnH/6)}, (float)(menuBtnH/2), 0.0f, x11_white);
        // Anchor icon position and size
        int icon_x = 32;
        int icon_y = topBarH + 32;
        int icon_w = (int)(logo.width  * iconScale);
        int icon_h = (int)(logo.height * iconScale);
        // Anchor terminal window size/position if not dragging
        if (!dragging && !terminal_open) {
            termWin.x = screenWidth * 0.31f;
            termWin.y = screenHeight * 0.23f;
            termWin.width = screenWidth * 0.62f;
            termWin.height = screenHeight * 0.55f;
        } else if (!dragging && terminal_open && (termWin.width > screenWidth || termWin.height > screenHeight)) {
            // Clamp terminal window to fit inside window
            if (termWin.x + termWin.width > screenWidth) termWin.x = screenWidth - termWin.width;
            if (termWin.y + termWin.height > screenHeight) termWin.y = screenHeight - termWin.height;
            if (termWin.x < 0) termWin.x = 0;
            if (termWin.y < 0) termWin.y = 0;
        }
    // Compute title bar and button metrics so dragging and drawing share coordinates
    int border = (int)(screenWidth * 0.002f);
    int titleH = (int)(termWin.height * 0.07f);
    int closeBtnSz = titleH - 4;
    // Global mouse position for focus-follows-mouse and interactions
    Vector2 globalMouse = GetMousePosition();

        // Toggle focus-follows-mouse with Ctrl+F
        if (IsKeyPressed(KEY_F) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
            focus_follows_mouse = !focus_follows_mouse;
        }

        // If focus-follows-mouse is enabled, set focused_window under pointer
        if (focus_follows_mouse) {
            focused_window = -1;
            if (terminal_open && !terminal_minimized && (term_workspace == workspace || term_sticky)) {
                if (CheckCollisionPointRec(globalMouse, termWin)) focused_window = WIN_TERM;
            }
            if (focused_window == -1 && xclock_open && !xclock_minimized && (xclock_workspace == workspace || xclock_sticky)) {
                Rectangle xr = (Rectangle){ xclockWin.x, xclockWin.y, xclockWin.width, xclockWin.height };
                if (CheckCollisionPointRec(globalMouse, xr)) focused_window = WIN_XCLOCK;
            }
            if (focused_window == -1 && utilities_open && !utilities_minimized && (utilities_workspace == workspace || utilities_sticky)) {
                Rectangle ur = (Rectangle){ utilitiesWin.x, utilitiesWin.y, utilitiesWin.width, utilitiesWin.height };
                if (CheckCollisionPointRec(globalMouse, ur)) focused_window = WIN_UTILS;
            }
        }

        // Handle right-click anywhere (for easier testing)
        // Wait a few frames before accepting input to avoid spurious startup events
        if (frameCount > 10 && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            showContextMenu = 1;
            contextMenuPos = mouse;
            contextMenuSetFrame = frameCount;
        }
    // Handle left-click on context menu (ignore clicks on the same frame the menu was opened)
    // Use release so holding the button while launching the app won't immediately select an item
        if (showContextMenu && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && frameCount > contextMenuSetFrame) {
            Vector2 mouse = GetMousePosition();
            int menuW = 180, menuH = 32;
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            // Clamp menu so all 4 rows fit on screen
            int totalH = menuH * 4;
            if (menuX + menuW > screenWidth) menuX = screenWidth - menuW - 8;
            if (menuY + totalH > screenHeight) menuY = screenHeight - totalH - 8;
            Rectangle xtermRect = {menuX, menuY, menuW, menuH};
            Rectangle aboutRect = {menuX, menuY + menuH, menuW, menuH};
            Rectangle utilitiesRect = {menuX, menuY + 2*menuH, menuW, menuH};
            Rectangle exitRect  = {menuX, menuY + 3*menuH, menuW, menuH};
            if (CheckCollisionPointRec(mouse, xtermRect)) {
                // Launch terminal if not open
                if (!terminal_open) {
                    terminal_open = 1;
                    if (!shell) shell = CreateChildProc();
                    LaunchShell(shell, "\"C:\\WNU\\WNU OS\\wnuos.exe\"");
                    term_workspace = workspace; /* assign to current workspace */
                    lineCount = 0;
                    inputLen = 0;
                    inputLine[0] = '\0';
                }
            } else if (CheckCollisionPointRec(mouse, aboutRect)) {
            } else if (CheckCollisionPointRec(mouse, utilitiesRect)) {
                // Open Utilities window (larger by default so tiles fit)
                utilities_open = 1;
                utilities_workspace = workspace; /* show utilities on current workspace */
                utilitiesWin.width = 520;
                utilitiesWin.height = 360;
                utilitiesWin.x = (screenWidth - utilitiesWin.width) / 2;
                utilitiesWin.y = (screenHeight - utilitiesWin.height) / 2;
            } else if (CheckCollisionPointRec(mouse, exitRect)) {
                running = 0;
            }
            showContextMenu = 0;
        }
        // Handle click on terminal icon to launch or restore shell window
        if ((terminal_minimized || !terminal_open) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (mouse.x >= icon_x && mouse.x <= icon_x + icon_w &&
                mouse.y >= icon_y && mouse.y <= icon_y + icon_h) {
                if (terminal_minimized) {
                    // Restore window without restarting shell
                    terminal_minimized = 0;
                    focused_window = WIN_TERM; last_clicked = WIN_TERM;
                } else {
                    // Only open terminal, never close window
                    terminal_open = 1;
                    if (!shell) shell = CreateChildProc();
                    LaunchShell(shell, "\"C:\\WNU\\WNU OS\\wnuos.exe\"");
                    term_workspace = workspace; /* assign terminal to current workspace when launched from icon */
                    lineCount = 0;
                    inputLen = 0;
                    inputLine[0] = '\0';
                }
            }
        }

        // Handle clicks on additional minimized icons (xclock/utilities) placed next to terminal icon
        {
            int minIconX = icon_x + icon_w + 24;
            int minIconY = icon_y;
            int minIconW = (int)(xclocklogo.width * iconScale);
            int minIconH = (int)(xclocklogo.height * iconScale);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mm = GetMousePosition();
                if (xclock_minimized) {
                    Rectangle r = { (float)minIconX, (float)minIconY, (float)minIconW, (float)minIconH };
                    if (CheckCollisionPointRec(mm, r)) { xclock_minimized = 0; xclock_open = 1; xclock_workspace = workspace; focused_window = WIN_XCLOCK; last_clicked = WIN_XCLOCK; }
                    minIconX += minIconW + 12;
                }
                if (utilities_minimized) {
                    Rectangle r2 = { (float)minIconX, (float)minIconY, (float)minIconW, (float)minIconH };
                    if (CheckCollisionPointRec(mm, r2)) { utilities_minimized = 0; utilities_open = 1; utilities_workspace = workspace; focused_window = WIN_UTILS; last_clicked = WIN_UTILS; }
                }
            }
        }

        // Terminal window dragging, close and minimize button handling
        if (terminal_open) {
            Vector2 m = GetMousePosition();
            int border = (int)(screenWidth * 0.002f);
            int titleH = (int)(termWin.height * 0.07f);
            int closeBtnSz = titleH - 4;
            int closeBtnX = (int)(termWin.x + termWin.width - closeBtnSz - border);
            int closeBtnY = (int)(termWin.y + border + 2);
            Rectangle closeBtn = { (float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz };
            int minBtnX = closeBtnX - closeBtnSz - 4;
            if (minBtnX < termWin.x + border) minBtnX = (int)(termWin.x + border);
            Rectangle minBtn = { (float)minBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz };
            close_hover = CheckCollisionPointRec(m, closeBtn);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(m, closeBtn)) {
                    terminal_open = 0;
                    if (shell) { CloseShell(shell); DestroyChildProc(shell); shell = NULL; }
                    if (last_clicked == WIN_TERM) last_clicked = -1;
                    if (focused_window == WIN_TERM) focused_window = -1;
                } else if (CheckCollisionPointRec(m, minBtn)) {
                    terminal_minimized = 1;
                } else if (CheckCollisionPointRec(m, (Rectangle){ termWin.x, termWin.y, termWin.width, (float)titleH })) {
                    dragging = 1;
                    dragOffset.x = m.x - termWin.x;
                    dragOffset.y = m.y - termWin.y;
                    last_clicked = WIN_TERM;
                    focused_window = WIN_TERM;
                } else {
                    Rectangle resizeHandle = { termWin.x + termWin.width - 18, termWin.y + termWin.height - 18, 18, 18 };
                    if (CheckCollisionPointRec(m, resizeHandle)) {
                        term_resizing = 1; last_clicked = WIN_TERM; focused_window = WIN_TERM;
                    }
                }
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = 0;
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) term_resizing = 0;
            if (dragging) {
                termWin.x = m.x - dragOffset.x;
                termWin.y = m.y - dragOffset.y;
            }
            if (term_resizing) {
                Vector2 rm = GetMousePosition();
                float newW = rm.x - termWin.x; if (newW < 160) newW = 160; if (newW > screenWidth - termWin.x) newW = screenWidth - termWin.x - 8;
                float newH = rm.y - termWin.y; if (newH < 120) newH = 120; if (newH > screenHeight - termWin.y) newH = screenHeight - termWin.y - 8;
                termWin.width = newW; termWin.height = newH;
            }
        }

    // Read real shell output and append to terminal buffer
    if (terminal_open && shell) {
            char chunk[READ_CHUNK_SIZE];
            double __r_t0 = GetTime();
            int got = ReadShellOutput(shell, chunk, sizeof(chunk));
            double __r_t1 = GetTime();
            if ((__r_t1 - __r_t0) > 0.05) {
            }
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
                if (shell) {
                    char toSend[TERM_MAX_COLUMNS + 2];
                    int sendLen = snprintf(toSend, sizeof(toSend), "%s\r\n", inputLine);
                    WriteShellInput(shell, toSend, sendLen);
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
        
        double __frame_t0 = GetTime();
        // Poll input/events to keep window responsive
        // glfwPollEvents(); // Not needed with raylib's main loop

    // Workspace indicator (center, dynamic sizing)
    // FVWM-style pager (center) - draw small boxes for each workspace
    int pagerW = workspaceCount * (menuBtnH + 8);
    int pagerX = (screenWidth - pagerW) / 2;
    int pagerY = menuBtnY;
    for (int p = 1; p <= workspaceCount; p++) {
        int bx = pagerX + (p-1) * (menuBtnH + 8);
        int by = pagerY;
        Rectangle brect = { (float)bx, (float)by, (float)menuBtnH, (float)menuBtnH };
        Color fill = (p == workspace) ? (Color){120,160,220,255} : (Color){80,80,120,255};
        DrawRectangleRec(brect, fill);
        DrawRectangleLinesEx(brect, 1, (Color){40,40,60,255});
        char numbuf[4];
        snprintf(numbuf, sizeof(numbuf), "%d", p);
        DrawTextEx(guiFont, numbuf, (Vector2){bx + menuBtnH/4.0f, by + menuBtnH/6.0f}, (float)(menuBtnH/2), 0.0f, x11_white);
        // click to switch workspace
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 _m = GetMousePosition();
            if (CheckCollisionPointRec(_m, brect)) {
                workspace = p;
            }
        }
    }
    // Number-key switching (1..workspaceCount)
    for (int k = 1; k <= workspaceCount; k++) {
        int key = KEY_ONE + (k-1); // KEY_ONE..KEY_FOUR
        if (IsKeyPressed(key)) workspace = k;
    }

    // Alt+Number: move focused window to workspace k
    for (int k = 1; k <= workspaceCount; k++) {
        int key = KEY_ONE + (k-1);
        if ((IsKeyPressed(key)) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT))) {
            if (focused_window == WIN_TERM) { term_workspace = k; }
            else if (focused_window == WIN_XCLOCK) { xclock_workspace = k; }
            else if (focused_window == WIN_UTILS) { utilities_workspace = k; }
        }
    }

    // Ctrl+S toggles sticky flag for focused window (appears on all workspaces)
    if (IsKeyPressed(KEY_S) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        if (focused_window == WIN_TERM) term_sticky = !term_sticky;
        else if (focused_window == WIN_XCLOCK) xclock_sticky = !xclock_sticky;
        else if (focused_window == WIN_UTILS) utilities_sticky = !utilities_sticky;
    }

    // Clock (right, dynamic sizing)
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char clockStr[32];
    strftime(clockStr, sizeof(clockStr), "%H:%M:%S", t);
    int clockFontSz = topBarH/2;
    Vector2 clkSz = MeasureTextEx(guiFont, clockStr, (float)clockFontSz, 0.0f);
    int clockTextW = (int)clkSz.x;
    int clockW = clockTextW + 32;
    int clockH = menuBtnH;
    int clockX = screenWidth - clockW - 16, clockY = menuBtnY;
    DrawRectangle(clockX, clockY, clockW, clockH, (Color){80, 80, 120, 255});
    DrawTextEx(guiFont, clockStr, (Vector2){(float)(clockX + 16), (float)(clockY + (clockH-clockFontSz)/2)}, (float)clockFontSz, 0.0f, x11_white);
    // Click the clock area to toggle xclock window
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 _m = GetMousePosition();
        Rectangle clkRect = { (float)clockX, (float)clockY, (float)clockW, (float)clockH };
        if (CheckCollisionPointRec(_m, clkRect)) {
            xclock_open = !xclock_open;
            if (xclock_open) {
                xclock_workspace = workspace; /* assign xclock to current workspace when opened */
                // Initialize xclock window size and position near the top-right, but below top bar
                xclockWin.width = 220;
                xclockWin.height = 220;
                xclockWin.x = screenWidth - xclockWin.width - 32;
                xclockWin.y = topBarH + 48;
            }
        }
    }
    
    // FVWM logo (left of menu)
    float logoScaleBar = 0.18f;
    int logoBarW = (int)(logo.width * logoScaleBar);
    (void)logoBarW; // placeholder, used in some layouts; silence unused warning
    (void)contextMenuHover; // reserved for future hover state
    int logoBarH = (int)(logo.height * logoScaleBar);
    int logoBarX = menuBtnX + menuBtnW + 8;
    int logoBarY = (topBarH - logoBarH) / 2;
    DrawTextureEx(logo, (Vector2){(float)logoBarX, (float)logoBarY}, 0.0f, logoScaleBar, x11_white);

    // (Global shortcuts removed) — shortcuts are available only inside the Utilities window.

        // Desktop background
    DrawRectangle(0, topBarH, screenWidth, screenHeight - topBarH, x11_gray);
    DrawTextEx(guiFont, "Welcome to X11 Desktop", (Vector2){(float)(screenWidth*0.06f), (float)(screenHeight*0.08f)}, (float)(topBarH*0.7f), 0.0f, x11_title);

        // Terminal icon
    DrawTextureEx(xtermlogo, (Vector2){(float)icon_x, (float)icon_y}, 0.0f, iconScale, x11_white);
    DrawTextEx(guiFont, "xterm", (Vector2){(float)icon_x, (float)(icon_y + icon_h + 8)}, 18.0f, 0.0f, x11_title);

    // Draw xclock window if open AND on current workspace (and not minimized)
    if (xclock_open && !xclock_minimized && (xclock_workspace == workspace || xclock_sticky)) {
        // Window background and border
        int xb = (int)(screenWidth * 0.002f);
        DrawRectangle((int)xclockWin.x, (int)xclockWin.y, (int)xclockWin.width, (int)xclockWin.height, (Color){80,80,120,255});
        DrawRectangle((int)xclockWin.x + xb, (int)xclockWin.y + xb, (int)xclockWin.width - 2*xb, (int)xclockWin.height - 2*xb, x11_white);
        // Title bar
        int xTitleH = 28;
        DrawRectangle((int)xclockWin.x + xb, (int)xclockWin.y + xb, (int)xclockWin.width - 2*xb, xTitleH, (Color){60,60,80,255});
    DrawTextEx(guiFont, "xclock", (Vector2){(float)xclockWin.x + xb + 8, (float)xclockWin.y + xb + 6}, 14.0f, 0.0f, x11_white);
        // Close button
        int xCloseSz = xTitleH - 6;
        int xCloseX = (int)(xclockWin.x + xclockWin.width - xCloseSz - xb - 4);
        int xCloseY = (int)(xclockWin.y + xb + 3);
        xclock_close_hover = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)xCloseX,(float)xCloseY,(float)xCloseSz,(float)xCloseSz});
        DrawRectangle(xCloseX, xCloseY, xCloseSz, xCloseSz, xclock_close_hover ? RED : (Color){120,120,120,255});
    DrawTextEx(guiFont, "X", (Vector2){(float)(xCloseX + 4), (float)(xCloseY + 2)}, 14.0f, 0.0f, x11_white);

        // Handle dragging and close interactions for xclock window
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 _m = GetMousePosition();
            Rectangle titleRect = {(float)xclockWin.x + xb, (float)xclockWin.y + xb, (float)xclockWin.width - 2*xb, (float)xTitleH};
            if (CheckCollisionPointRec(_m, (Rectangle){(float)xCloseX,(float)xCloseY,(float)xCloseSz,(float)xCloseSz})) {
                xclock_open = 0;
                    if (last_clicked == WIN_XCLOCK) last_clicked = -1;
            } else if (CheckCollisionPointRec(_m, titleRect)) {
                xclock_dragging = 1;
                xclockDragOffset.x = _m.x - xclockWin.x;
                xclockDragOffset.y = _m.y - xclockWin.y;
                    last_clicked = WIN_XCLOCK;
            }
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) xclock_dragging = 0;
        if (xclock_dragging) {
            Vector2 _m = GetMousePosition();
            xclockWin.x = _m.x - xclockDragOffset.x;
            xclockWin.y = _m.y - xclockDragOffset.y;
        }
        // resize handle for xclock
        Rectangle xResize = { xclockWin.x + xclockWin.width - 18, xclockWin.y + xclockWin.height - 18, 18, 18 };
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(GetMousePosition(), xResize)) { xclock_resizing = 1; last_clicked = WIN_XCLOCK; focused_window = WIN_XCLOCK; }
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) xclock_resizing = 0;
        if (xclock_resizing) {
            Vector2 rm = GetMousePosition();
            float newW = rm.x - xclockWin.x; if (newW < 80) newW = 80; if (newW > screenWidth - xclockWin.x) newW = screenWidth - xclockWin.x - 8;
            float newH = rm.y - xclockWin.y; if (newH < 80) newH = 80; if (newH > screenHeight - xclockWin.y) newH = screenHeight - xclockWin.y - 8;
            xclockWin.width = newW; xclockWin.height = newH;
        }

        // Draw analog clock face centered in the client area
        float cx = xclockWin.x + xclockWin.width/2.0f;
        float cy = xclockWin.y + xclockWin.height/2.0f + 8;
        float radius = (xclockWin.width < xclockWin.height ? xclockWin.width : xclockWin.height) * 0.35f;
        DrawCircle((int)cx, (int)cy, radius, (Color){18,18,18,255});
        DrawCircleLines((int)cx, (int)cy, radius, (Color){200,200,200,255});
        // Hands
        time_t nowt = time(NULL);
        struct tm* tmnow = localtime(&nowt);
        float sec = tmnow->tm_sec;
        float min = tmnow->tm_min + sec/60.0f;
        float hr  = (tmnow->tm_hour % 12) + min/60.0f;
        float angSec = (sec / 60.0f) * 2*PI - PI/2;
        float angMin = (min / 60.0f) * 2*PI - PI/2;
        float angHr  = (hr  / 12.0f) * 2*PI - PI/2;
        DrawLine((int)cx, (int)cy, (int)(cx + cosf(angHr)*(radius*0.5f)), (int)(cy + sinf(angHr)*(radius*0.5f)), (Color){220,220,220,255});
        DrawLine((int)cx, (int)cy, (int)(cx + cosf(angMin)*(radius*0.75f)), (int)(cy + sinf(angMin)*(radius*0.75f)), (Color){200,200,200,255});
        DrawLine((int)cx, (int)cy, (int)(cx + cosf(angSec)*(radius*0.9f)), (int)(cy + sinf(angSec)*(radius*0.9f)), (Color){255,0,0,255});
    }
    // Draw xclock minimized icon if iconified
    if (xclock_minimized) {
        int minIconX = icon_x + icon_w + 24;
        int minIconY = icon_y;
        DrawTextureEx(xclocklogo, (Vector2){(float)minIconX, (float)minIconY}, 0.0f, iconScale, x11_white);
        DrawTextEx(guiFont, "xclock", (Vector2){(float)minIconX, (float)(minIconY + (int)(xclocklogo.height*iconScale) + 4)}, 14.0f, 0.0f, x11_title);
    }

    // Draw Utilities window if open AND on current workspace (and not minimized)
    if (utilities_open && !utilities_minimized && (utilities_workspace == workspace || utilities_sticky)) {
        int ub = (int)(screenWidth * 0.002f);
        DrawRectangle((int)utilitiesWin.x, (int)utilitiesWin.y, (int)utilitiesWin.width, (int)utilitiesWin.height, (Color){80,80,120,255});
        DrawRectangle((int)utilitiesWin.x + ub, (int)utilitiesWin.y + ub, (int)utilitiesWin.width - 2*ub, (int)utilitiesWin.height - 2*ub, x11_white);
        int uTitleH = 28;
        DrawRectangle((int)utilitiesWin.x + ub, (int)utilitiesWin.y + ub, (int)utilitiesWin.width - 2*ub, uTitleH, (Color){60,60,80,255});
    DrawTextEx(guiFont, "Utilities", (Vector2){(float)utilitiesWin.x + ub + 8, (float)utilitiesWin.y + ub + 6}, 14.0f, 0.0f, x11_white);
        int uCloseSz = uTitleH - 6;
        int uCloseX = (int)(utilitiesWin.x + utilitiesWin.width - uCloseSz - ub - 4);
        int uCloseY = (int)(utilitiesWin.y + ub + 3);
        utilities_close_hover = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)uCloseX,(float)uCloseY,(float)uCloseSz,(float)uCloseSz});
        DrawRectangle(uCloseX, uCloseY, uCloseSz, uCloseSz, utilities_close_hover ? RED : (Color){120,120,120,255});
    DrawTextEx(guiFont, "X", (Vector2){(float)(uCloseX + 4), (float)(uCloseY + 2)}, 14.0f, 0.0f, x11_white);

        // Desktop-style shortcuts anchored top-left inside Utilities window (only visible here)
        {
            Vector2 _m2 = GetMousePosition();
            // Try to fit all icons on a single row by computing a uniform scale
                const char* labels[3] = { "Clear Terminal", "xclock", "xcalc" };
                Texture2D* icons[3] = { &xtermlogo, &xclocklogo, &xcalclogo };
            int nshort = 3;
            float baseScale = 0.24f; // preferred nice size
            int padding = 12;
            int gap = 20;
            int availableW = (int)(utilitiesWin.width - 2*padding);
            if (availableW < 1) availableW = 1;

            // compute max icon width at baseScale
            int widths[3];
            int heights[3];
            for (int i = 0; i < nshort; i++) {
                widths[i] = (int)(icons[i]->width * baseScale);
                heights[i] = (int)(icons[i]->height * baseScale);
            }
            // total width needed with baseScale
            int totalNeeded = -gap; // compensate first loop
            int maxH = 0;
            for (int i = 0; i < nshort; i++) { totalNeeded += widths[i] + gap; if (heights[i] > maxH) maxH = heights[i]; }

            float uniformScale = baseScale;
            if (totalNeeded > availableW) {
                // shrink uniformly so everything fits one row
                float shrink = (float)availableW / (float)totalNeeded;
                if (shrink < 0.5f) shrink = 0.5f; // avoid too tiny
                uniformScale = baseScale * shrink;
                for (int i = 0; i < nshort; i++) {
                    widths[i] = (int)(icons[i]->width * uniformScale);
                    heights[i] = (int)(icons[i]->height * uniformScale);
                }
                // recompute totalNeeded
                totalNeeded = -gap; maxH = 0;
                for (int i = 0; i < nshort; i++) { totalNeeded += widths[i] + gap; if (heights[i] > maxH) maxH = heights[i]; }
            }

            // If still doesn't fit (very narrow), compute columns instead
            int leftX = (int)(utilitiesWin.x + padding);
            int topY = (int)(utilitiesWin.y + ub + uTitleH + 12);
            Rectangle rects[3];
            if (totalNeeded <= availableW) {
                // left-align the row inside the utilities client area (top-left)
                int x = leftX;
                for (int i = 0; i < nshort; i++) {
                    rects[i] = (Rectangle){ (float)x, (float)topY, (float)widths[i], (float)heights[i] };
                    x += widths[i] + gap;
                }
            } else {
                // place in columns (one per row) with no overlap
                int x = leftX;
                int y = topY;
                for (int i = 0; i < nshort; i++) {
                    rects[i] = (Rectangle){ (float)x, (float)y, (float)widths[i], (float)heights[i] };
                    y += heights[i] + 28; // space for label
                    // clamp vertically to utilities client area
                    int maxY = (int)(utilitiesWin.y + utilitiesWin.height - padding - heights[i] - 8);
                    if (rects[i].y > maxY) rects[i].y = maxY;
                }
            }

            // Draw icons and labels; use uniformScale for texture draw
            for (int i = 0; i < nshort; i++) {
                int hover = CheckCollisionPointRec(_m2, rects[i]);
                if (hover) DrawRectangleRec(rects[i], (Color){40,60,180,255});
                DrawTextureEx(*icons[i], (Vector2){rects[i].x, rects[i].y}, 0.0f, uniformScale, x11_white);
                int labFont = 14;
                int maxLabelWidth = (int)(utilitiesWin.width - 2*padding);
                if (maxLabelWidth < 32) maxLabelWidth = 32;
                Vector2 tempSz = MeasureTextEx(guiFont, labels[i], (float)labFont, 0.0f);
                int w = (int)tempSz.x;
                // Shrink font until it fits or reach a minimum size
                while (w > maxLabelWidth && labFont > 8) {
                    labFont--;
                    tempSz = MeasureTextEx(guiFont, labels[i], (float)labFont, 0.0f);
                    w = (int)tempSz.x;
                }
                // If still too wide, truncate with ellipsis
                char labBuf[128];
                strncpy(labBuf, labels[i], sizeof(labBuf)-1);
                labBuf[sizeof(labBuf)-1] = '\0';
                if (w > maxLabelWidth) {
                    int len = (int)strlen(labBuf);
                    // reserve 3 chars for '...'
                    for (int cut = len; cut > 0; cut--) {
                        labBuf[cut] = '\0';
                        // append ellipsis temporarily
                        if (cut >= 3) {
                            labBuf[cut-3] = '\0';
                            // build truncated string
                            char tmp[128];
                            snprintf(tmp, sizeof(tmp), "%s...", labels[i]);
                            // tmp currently wrong; instead copy first cut-3 chars
                        }
                    }
                    // Simpler truncation: binary shrink by characters
                    int keep = (int)strlen(labels[i]);
                    while (keep > 0) {
                        keep--;
                        char tmp[128];
                        if (keep > 3) {
                            snprintf(tmp, sizeof(tmp), "%.*s...", keep-3, labels[i]);
                        } else {
                            snprintf(tmp, sizeof(tmp), "%.*s", keep, labels[i]);
                        }
                        Vector2 twv = MeasureTextEx(guiFont, tmp, (float)labFont, 0.0f);
                        int tw = (int)twv.x;
                        if (tw <= maxLabelWidth) {
                            strncpy(labBuf, tmp, sizeof(labBuf)-1);
                            labBuf[sizeof(labBuf)-1] = '\0';
                            w = tw;
                            break;
                        }
                    }
                } else {
                    // fits, copy original
                    strncpy(labBuf, labels[i], sizeof(labBuf)-1);
                    labBuf[sizeof(labBuf)-1] = '\0';
                }

                int tx = (int)rects[i].x + ((int)rects[i].width)/2 - w/2;
                // clamp label to utilities client area
                int minTx = (int)(utilitiesWin.x + padding);
                int maxTx = (int)(utilitiesWin.x + utilitiesWin.width - padding - w);
                if (tx < minTx) tx = minTx;
                if (tx > maxTx) tx = maxTx;
                int ty = (int)(rects[i].y + rects[i].height + 8);
                DrawRectangle(tx - 6, ty - 4, w + 12, labFont + 6, (Color){20,20,30,180});
                DrawTextEx(guiFont, labBuf, (Vector2){(float)(tx + 1), (float)(ty + 1)}, (float)labFont, 0.0f, (Color){0,0,0,160});
                DrawTextEx(guiFont, labBuf, (Vector2){(float)tx, (float)ty}, (float)labFont, 0.0f, x11_white);
            }

            // Click handling — hitboxes match icon rects
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mclick = GetMousePosition();
                Rectangle titleRect = {(float)utilitiesWin.x + ub, (float)utilitiesWin.y + ub, (float)utilitiesWin.width - 2*ub, (float)uTitleH};
                if (CheckCollisionPointRec(mclick, (Rectangle){(float)uCloseX,(float)uCloseY,(float)uCloseSz,(float)uCloseSz})) {
                    utilities_open = 0;
                        if (last_clicked == WIN_UTILS) last_clicked = -1;
                } else if (CheckCollisionPointRec(mclick, titleRect)) {
                    utilities_dragging = 1;
                    utilitiesDragOffset.x = mclick.x - utilitiesWin.x;
                    utilitiesDragOffset.y = mclick.y - utilitiesWin.y;
                        last_clicked = WIN_UTILS;
                } else {
                    for (int i = 0; i < nshort; i++) {
                        if (CheckCollisionPointRec(mclick, rects[i])) {
                            if (strcmp(labels[i], "xclock") == 0) {
                                xclock_open = 1;
                                xclock_workspace = workspace;
                                xclockWin.width = 220; xclockWin.height = 220;
                                xclockWin.x = (screenWidth - xclockWin.width) / 2;
                                xclockWin.y = (screenHeight - xclockWin.height) / 2;
                            } else if (strcmp(labels[i], "Clear Terminal") == 0) {
                                lineCount = 0;
                            } else if (strcmp(labels[i], "xcalc") == 0) {
                                // Open a simple xcalc window centered on the screen
                                xcalc_open = 1;
                                xcalc_workspace = workspace; /* assign to current workspace */
                                xcalcWin.width = 300;
                                xcalcWin.height = 300;
                                xcalcWin.x = (screenWidth - xcalcWin.width) / 2;
                                xcalcWin.y = topBarH + 64;
                                // Removed debug print for xcalc open
                            }
                        }
                    }
                }
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) utilities_dragging = 0;
            if (utilities_dragging) {
                Vector2 nm = GetMousePosition();
                utilitiesWin.x = nm.x - utilitiesDragOffset.x;
                utilitiesWin.y = nm.y - utilitiesDragOffset.y;
            }
            // resize handle for utilities
            Rectangle uResize = { utilitiesWin.x + utilitiesWin.width - 18, utilitiesWin.y + utilitiesWin.height - 18, 18, 18 };
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(GetMousePosition(), uResize)) { utilities_resizing = 1; last_clicked = WIN_UTILS; focused_window = WIN_UTILS; }
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) utilities_resizing = 0;
            if (utilities_resizing) {
                Vector2 rm = GetMousePosition();
                float newW = rm.x - utilitiesWin.x; if (newW < 160) newW = 160; if (newW > screenWidth - utilitiesWin.x) newW = screenWidth - utilitiesWin.x - 8;
                float newH = rm.y - utilitiesWin.y; if (newH < 120) newH = 120; if (newH > screenHeight - utilitiesWin.y) newH = screenHeight - utilitiesWin.y - 8;
                utilitiesWin.width = newW; utilitiesWin.height = newH;
            }
        }
    // Draw xcalc (delegated to reusable widget)
    draw_xcalc_widget(&xcalcWin, &xcalc_open, &xcalc_minimized, &xcalc_resizing, &xcalc_close_hover,
                      &xcalc_workspace, &xcalc_sticky, guiFont, xcalclogo,
                      &last_clicked, WIN_XCALC,
                      &focused_window, workspace, screenWidth, screenHeight);
    // Draw utilities minimized icon if iconified
    if (utilities_minimized) {
        int minIconX = icon_x + icon_w + 24;
        int minIconY = icon_y;
        if (xclock_minimized) minIconX += (int)(xclocklogo.width * iconScale) + 12;
        DrawTextureEx(xtermlogo, (Vector2){(float)minIconX, (float)minIconY}, 0.0f, iconScale, x11_white);
        DrawTextEx(guiFont, "Utilities", (Vector2){(float)minIconX, (float)(minIconY + (int)(xtermlogo.height*iconScale) + 4)}, 14.0f, 0.0f, x11_title);
    }

        // Draw context menu if active
        if (showContextMenu) {
            int menuW = 180, menuH = 32;
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            int totalH = menuH * 4;
            if (menuX + menuW > screenWidth) menuX = screenWidth - menuW - 8;
            if (menuY + totalH > screenHeight) menuY = screenHeight - totalH - 8;
            Color menuBg = (Color){80, 80, 120, 255};
            Color menuBorder = (Color){60, 60, 80, 255};
            Color menuHighlight = (Color){40, 60, 180, 255};
            Vector2 mouse = GetMousePosition();
            Rectangle xtermRect = {menuX, menuY, menuW, menuH};
            Rectangle aboutRect = {menuX, menuY + menuH, menuW, menuH};
            Rectangle utilitiesRect = {menuX, menuY + 2*menuH, menuW, menuH};
            Rectangle exitRect  = {menuX, menuY + 3*menuH, menuW, menuH};
            int hoverXterm = CheckCollisionPointRec(mouse, xtermRect);
            int hoverAbout = CheckCollisionPointRec(mouse, aboutRect);
            int hoverUtilities = CheckCollisionPointRec(mouse, utilitiesRect);
            int hoverExit  = CheckCollisionPointRec(mouse, exitRect);
            // Draw XTerm item
            DrawRectangleRec(xtermRect, hoverXterm ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY, menuW, menuH, menuBorder);
            DrawTextEx(guiFont, "XTerm", (Vector2){(float)(menuX + 12), (float)(menuY + 7)}, 18.0f, 0.0f, x11_white);
            // Draw About item
            DrawRectangleRec(aboutRect, hoverAbout ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY + menuH, menuW, menuH, menuBorder);
            DrawTextEx(guiFont, "About X11 Desktop", (Vector2){(float)(menuX + 12), (float)(menuY + menuH + 7)}, 18.0f, 0.0f, x11_white);
            // Draw Utilities item
            DrawRectangleRec(utilitiesRect, hoverUtilities ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY + 2*menuH, menuW, menuH, menuBorder);
            DrawTextEx(guiFont, "Utilities", (Vector2){(float)(menuX + 12), (float)(menuY + 2*menuH + 7)}, 18.0f, 0.0f, x11_white);
            // Draw Exit item
            DrawRectangleRec(exitRect, hoverExit ? menuHighlight : menuBg);
            DrawRectangleLines(menuX, menuY + 3*menuH, menuW, menuH, menuBorder);
            DrawTextEx(guiFont, "Exit X11", (Vector2){(float)(menuX + 12), (float)(menuY + 3*menuH + 7)}, 18.0f, 0.0f, x11_white);
        }

        // Terminal window (show only on its assigned workspace)
        if (terminal_open && !terminal_minimized && term_workspace == workspace) {
            // FVWM-like window decorations
            int border = (int)(screenWidth * 0.002f);
            int titleH = (int)(termWin.height * 0.07f);
            DrawRectangleRec(termWin, (Color){80, 80, 120, 255});
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, (int)termWin.height - 2*border, x11_term_bg);
            // Title bar (flat)
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, titleH, (Color){60, 60, 80, 255});
            DrawTextEx(guiFont, "xterm", (Vector2){(float)((int)termWin.x + 16), (float)((int)termWin.y + border + 8)}, (float)(titleH*0.6f), 0.0f, x11_white);
            // Anchor close and minimize buttons to top-right
            int closeBtnSz = titleH-4;
            int closeBtnX = (int)(termWin.x + termWin.width - closeBtnSz - border);
            int closeBtnY = (int)(termWin.y + border + 2);
            // Ensure buttons stay inside window when resized
            if (closeBtnX + closeBtnSz > termWin.x + termWin.width) closeBtnX = (int)(termWin.x + termWin.width - closeBtnSz - border);
            if (closeBtnY + closeBtnSz > termWin.y + titleH) closeBtnY = (int)(termWin.y + border + 2);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, close_hover ? RED : (Color){120, 120, 120, 255});
            DrawTextEx(guiFont, "X", (Vector2){(float)(closeBtnX + 7), (float)(closeBtnY + 4)}, (float)(titleH*0.5f), 0.0f, x11_white);
            // Minimize button always left of close
            int minBtnSz = closeBtnSz;
            int minBtnX = closeBtnX - minBtnSz - 4;
            int minBtnY = closeBtnY;
            if (minBtnX < termWin.x + border) minBtnX = (int)(termWin.x + border);
            DrawRectangle(minBtnX, minBtnY, minBtnSz, minBtnSz, (Color){120, 120, 120, 255});
            DrawTextEx(guiFont, "_", (Vector2){(float)(minBtnX + 10), (float)(minBtnY + 4)}, (float)(titleH*0.5f), 0.0f, x11_white);
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
            DrawTextEx(guiFont, "> ", (Vector2){(float)x, (float)(maxY + 10)}, (float)(promptH*0.7f), 0.0f, x11_white);
            Vector2 promptSz = MeasureTextEx(guiFont, "> ", (float)(promptH*0.7f), 0.0f);
            DrawTextEx(guiFont, inputLine, (Vector2){(float)(x + (int)promptSz.x), (float)(maxY + 10)}, (float)(promptH*0.7f), 0.0f, x11_white);
            // Draw resize handle for terminal
            DrawRectangle((int)(termWin.x + termWin.width - 18), (int)(termWin.y + termWin.height - 18), 18, 18, (Color){100,100,120,255});
        }

        // Draw the last-clicked window again (minimal, visual-only) so it overlaps others.
        if (last_clicked == WIN_XCLOCK && xclock_open && xclock_workspace == workspace) {
            int xb = (int)(screenWidth * 0.002f);
            DrawRectangle((int)xclockWin.x, (int)xclockWin.y, (int)xclockWin.width, (int)xclockWin.height, (Color){80,80,120,255});
            DrawRectangle((int)xclockWin.x + xb, (int)xclockWin.y + xb, (int)xclockWin.width - 2*xb, (int)xclockWin.height - 2*xb, x11_white);
            int xTitleH = 28;
            DrawRectangle((int)xclockWin.x + xb, (int)xclockWin.y + xb, (int)xclockWin.width - 2*xb, xTitleH, (Color){60,60,80,255});
            int xCloseSz = xTitleH - 6;
            int xCloseX = (int)(xclockWin.x + xclockWin.width - xCloseSz - xb - 4);
            int xCloseY = (int)(xclockWin.y + xb + 3);
            DrawRectangle(xCloseX, xCloseY, xCloseSz, xCloseSz, xclock_close_hover ? RED : (Color){120,120,120,255});
            DrawTextEx(guiFont, "X", (Vector2){(float)(xCloseX + 4), (float)(xCloseY + 2)}, 14.0f, 0.0f, x11_white);
            float cx = xclockWin.x + xclockWin.width/2.0f;
            float cy = xclockWin.y + xclockWin.height/2.0f + 8;
            float radius = (xclockWin.width < xclockWin.height ? xclockWin.width : xclockWin.height) * 0.35f;
            DrawCircle((int)cx, (int)cy, radius, (Color){18,18,18,255});
            DrawCircleLines((int)cx, (int)cy, radius, (Color){200,200,200,255});
            time_t nowt = time(NULL);
            struct tm* tmnow = localtime(&nowt);
            float sec = tmnow->tm_sec;
            float min = tmnow->tm_min + sec/60.0f;
            float hr  = (tmnow->tm_hour % 12) + min/60.0f;
            float angSec = (sec / 60.0f) * 2*PI - PI/2;
            float angMin = (min / 60.0f) * 2*PI - PI/2;
            float angHr  = (hr  / 12.0f) * 2*PI - PI/2;
            DrawLine((int)cx, (int)cy, (int)(cx + cosf(angHr)*(radius*0.5f)), (int)(cy + sinf(angHr)*(radius*0.5f)), (Color){220,220,220,255});
            DrawLine((int)cx, (int)cy, (int)(cx + cosf(angMin)*(radius*0.75f)), (int)(cy + sinf(angMin)*(radius*0.75f)), (Color){200,200,200,255});
            DrawLine((int)cx, (int)cy, (int)(cx + cosf(angSec)*(radius*0.9f)), (int)(cy + sinf(angSec)*(radius*0.9f)), (Color){255,0,0,255});
        }
        if (last_clicked == WIN_UTILS && utilities_open && utilities_workspace == workspace) {
            int ub = (int)(screenWidth * 0.002f);
            DrawRectangle((int)utilitiesWin.x, (int)utilitiesWin.y, (int)utilitiesWin.width, (int)utilitiesWin.height, (Color){80,80,120,255});
            DrawRectangle((int)utilitiesWin.x + ub, (int)utilitiesWin.y + ub, (int)utilitiesWin.width - 2*ub, (int)utilitiesWin.height - 2*ub, x11_white);
            int uTitleH = 28;
            DrawRectangle((int)utilitiesWin.x + ub, (int)utilitiesWin.y + ub, (int)utilitiesWin.width - 2*ub, uTitleH, (Color){60,60,80,255});
            int uCloseSz = uTitleH - 6;
            int uCloseX = (int)(utilitiesWin.x + utilitiesWin.width - uCloseSz - ub - 4);
            int uCloseY = (int)(utilitiesWin.y + ub + 3);
            DrawRectangle(uCloseX, uCloseY, uCloseSz, uCloseSz, utilities_close_hover ? RED : (Color){120,120,120,255});
            DrawTextEx(guiFont, "X", (Vector2){(float)(uCloseX + 4), (float)(uCloseY + 2)}, 14.0f, 0.0f, x11_white);
            // Redraw icons and labels (simpler: reuse client drawing loop quickly)
            {
                const char* labels[2] = { "Clear Terminal", "xclock" };
                Texture2D* icons[2] = { &xtermlogo, &xclocklogo };
                int nshort = 2;
                float baseScale = 0.24f;
                int padding = 12;
                int gap = 20;
                int availableW = (int)(utilitiesWin.width - 2*padding);
                if (availableW < 1) availableW = 1;
                int widths[2]; int heights[2];
                for (int i = 0; i < nshort; i++) { widths[i] = (int)(icons[i]->width * baseScale); heights[i] = (int)(icons[i]->height * baseScale); }
                int totalNeeded = -gap; int maxH = 0; for (int i = 0; i < nshort; i++) { totalNeeded += widths[i] + gap; if (heights[i] > maxH) maxH = heights[i]; }
                float uniformScale = baseScale;
                if (totalNeeded > availableW) { float shrink = (float)availableW / (float)totalNeeded; if (shrink < 0.5f) shrink = 0.5f; uniformScale = baseScale * shrink; for (int i = 0; i < nshort; i++) { widths[i] = (int)(icons[i]->width * uniformScale); heights[i] = (int)(icons[i]->height * uniformScale); } }
                int leftX = (int)(utilitiesWin.x + padding); int topY = (int)(utilitiesWin.y + ub + uTitleH + 12);
                Rectangle rects[2];
                if (totalNeeded <= availableW) { int x = leftX; for (int i = 0; i < nshort; i++) { rects[i] = (Rectangle){ (float)x, (float)topY, (float)widths[i], (float)heights[i] }; x += widths[i] + gap; } } else { int x = leftX; int y = topY; for (int i = 0; i < nshort; i++) { rects[i] = (Rectangle){ (float)x, (float)y, (float)widths[i], (float)heights[i] }; y += heights[i] + 28; int maxY = (int)(utilitiesWin.y + utilitiesWin.height - padding - heights[i] - 8); if (rects[i].y > maxY) rects[i].y = maxY; } }
                for (int i = 0; i < nshort; i++) {
                    DrawTextureEx(*icons[i], (Vector2){rects[i].x, rects[i].y}, 0.0f, uniformScale, x11_white);
                    int labFont = 14;
                    Vector2 tempSz = MeasureTextEx(guiFont, labels[i], (float)labFont, 0.0f);
                    int w = (int)tempSz.x; if (w > (int)(utilitiesWin.width - 2*padding)) w = (int)(utilitiesWin.width - 2*padding);
                    int tx = (int)rects[i].x + ((int)rects[i].width)/2 - w/2; int minTx = (int)(utilitiesWin.x + padding); int maxTx = (int)(utilitiesWin.x + utilitiesWin.width - padding - w); if (tx < minTx) tx = minTx; if (tx > maxTx) tx = maxTx;
                    int ty = (int)(rects[i].y + rects[i].height + 8);
                    DrawRectangle(tx - 6, ty - 4, w + 12, labFont + 6, (Color){20,20,30,180});
                    DrawTextEx(guiFont, labels[i], (Vector2){(float)tx, (float)ty}, (float)labFont, 0.0f, x11_white);
                }
            }
        }
        if (last_clicked == WIN_TERM && terminal_open && !terminal_minimized && term_workspace == workspace) {
            int border = (int)(screenWidth * 0.002f);
            int titleH = (int)(termWin.height * 0.07f);
            DrawRectangleRec(termWin, (Color){80, 80, 120, 255});
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, (int)termWin.height - 2*border, x11_term_bg);
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, (int)termWin.width - 2*border, titleH, (Color){60, 60, 80, 255});
            int closeBtnSz = titleH-4;
            int closeBtnX = (int)(termWin.x + termWin.width - closeBtnSz - border);
            int closeBtnY = (int)(termWin.y + border + 2);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, close_hover ? RED : (Color){120, 120, 120, 255});
            DrawTextEx(guiFont, "X", (Vector2){(float)(closeBtnX + 7), (float)(closeBtnY + 4)}, (float)(titleH*0.5f), 0.0f, x11_white);
            int minBtnSz = closeBtnSz; int minBtnX = closeBtnX - minBtnSz - 4; if (minBtnX < termWin.x + border) minBtnX = (int)(termWin.x + border); int minBtnY = closeBtnY;
            DrawRectangle(minBtnX, minBtnY, minBtnSz, minBtnSz, (Color){120, 120, 120, 255});
            DrawTextEx(guiFont, "_", (Vector2){(float)(minBtnX + 10), (float)(minBtnY + 4)}, (float)(titleH*0.5f), 0.0f, x11_white);
            int y = (int)(termWin.y + titleH + 12);
            int x = (int)(termWin.x + 16);
            int maxY = (int)(termWin.y + termWin.height - 36);
            int first = 0;
            int lineH = (int)(termWin.height * 0.045f);
            int maxLinesOnScreen = (maxY - y) / lineH;
            if (lineCount > maxLinesOnScreen) first = lineCount - maxLinesOnScreen;
            for (int i = first; i < lineCount; i++) {
                DrawTextEx(GetFontDefault(), lines[i], (Vector2){(float)x, (float)y}, (float)lineH, 1.0f, x11_term_fg);
                y += lineH; if (y > maxY) break;
            }
            int promptH = (int)(termWin.height * 0.055f);
            DrawRectangle(x - 4, maxY + 8, (int)termWin.width - 32, promptH, x11_dkgray);
            DrawTextEx(guiFont, "> ", (Vector2){(float)x, (float)(maxY + 10)}, (float)(promptH*0.7f), 0.0f, x11_white);
            Vector2 promptSz = MeasureTextEx(guiFont, "> ", (float)(promptH*0.7f), 0.0f);
            DrawTextEx(guiFont, inputLine, (Vector2){(float)(x + (int)promptSz.x), (float)(maxY + 10)}, (float)(promptH*0.7f), 0.0f, x11_white);
        }
    // Cleanup
        /* Time and check around EndDrawing()/presentation. This helps detect
       stalls that occur during buffer swap/presentation or in the driver. */
       double __end_t0 = GetTime();
        EndDrawing();
        
        double __end_t1 = GetTime();
        double __end_dt = __end_t1 - __end_t0;
        if (__end_dt > 0.02) {
            // Frame took too long - could add debug info here
        }
    } // while (running && !WindowShouldClose())
    
    // Cleanup
    if (shell) { 
        CloseShell(shell); 
        DestroyChildProc(shell); 
        shell = NULL; 
    }
    UnloadTexture(logo);
    UnloadTexture(xtermlogo);
    UnloadTexture(xclocklogo);
    UnloadTexture(xcalclogo);
    UnloadFont(guiFont);
    CloseWindow();
    return 0;
}
}