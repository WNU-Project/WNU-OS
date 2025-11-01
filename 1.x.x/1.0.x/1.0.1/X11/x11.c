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
#include "xeyes.h"
#include "shell_win.h"
#include "xeyes_logo.h"
#include "xeyes.h"
#include "xlogo.h"
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

        // FWVM 3.x flat window frame with shadow
        DrawRectangle((int)xcalcWin->x + 2, (int)xcalcWin->y + 2, (int)xcalcWin->width, (int)xcalcWin->height, (Color){0,0,0,60}); // Shadow
        DrawRectangle((int)xcalcWin->x, (int)xcalcWin->y, (int)xcalcWin->width, (int)xcalcWin->height, (Color){76,76,76,255}); // Border
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, (int)xcalcWin->height - 2*xb, (Color){240,240,240,255}); // Light bg
        // Modern flat title bar
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, titleH, (Color){25,25,25,255}); // Dark titlebar
        DrawTextEx(guiFont, "Calculator", (Vector2){xcalcWin->x + xb + 8, xcalcWin->y + xb + 8}, 14.0f, 0.0f, (Color){255,255,255,255});

        // Modern flat close button (Windows style)
        int closeSz = titleH - 8;
        int closeX = (int)(xcalcWin->x + xcalcWin->width - closeSz - xb - 6);
        int closeY = (int)(xcalcWin->y + xb + 4);
        Vector2 mpos = GetMousePosition();
        *xcalc_close_hover = CheckCollisionPointRec(mpos, (Rectangle){(float)closeX,(float)closeY,(float)closeSz,(float)closeSz});
        DrawRectangle(closeX, closeY, closeSz, closeSz, *xcalc_close_hover ? (Color){232,17,35,255} : (Color){232,17,35,255}); // Windows red
        DrawTextEx(guiFont, "×", (Vector2){(float)(closeX+4),(float)(closeY+2)}, 14.0f, 0.0f, (Color){255,255,255,255});

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
                    else if (strcmp(lab,"+/-")==0) { if (display[0]=='-') memmove(display, display+1, strlen(display)); else if (strcmp(display,"0")!=0) { char t[128]; snprintf(t,sizeof(t),"-%s",display); strncpy(display,t,sizeof(display)-1); display[sizeof(display)-1]='\0';} }
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
        if (drag && *last_clicked == WIN_XCALC) { 
            Vector2 mm = GetMousePosition(); 
            xcalcWin->x = mm.x - dragOffset.x; 
            xcalcWin->y = mm.y - dragOffset.y; 
            // Keep window on screen
            if (xcalcWin->x < 0) xcalcWin->x = 0;
            if (xcalcWin->y < 48) xcalcWin->y = 48; // Below top bar
            if (xcalcWin->x + xcalcWin->width > screenWidth) xcalcWin->x = screenWidth - xcalcWin->width;
            if (xcalcWin->y + xcalcWin->height > screenHeight - 48) xcalcWin->y = screenHeight - 48 - xcalcWin->height; // Above taskbar
        }
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
bool isFullscreen = false;

int x11(void) {
    // Print OS it was build for and get the OS version
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);
    GetSystemInfo(&si);
    printf("Build Operating System: Windows 10.0.26200\n");
    printf("Current Operating System: Windows %lu.%lu.%lu\n", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    // Initialize raylib with FWVM 3.x branding
    int screenWidth = 1024;
    int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "X11 Desktop");
    SetTargetFPS(60);

    // Load textures from memory and fonts
    Image logoImg = LoadImageFromMemory(".png", X11_png, X11_png_len);
    Texture2D logo = LoadTextureFromImage(logoImg);
    UnloadImage(logoImg);
    
    Image xtermImg = LoadImageFromMemory(".png", xterm_png, xterm_png_len);
    Texture2D xtermlogo = LoadTextureFromImage(xtermImg);
    UnloadImage(xtermImg);
    
    Image xclockImg = LoadImageFromMemory(".png", xclock_png, xclock_png_len);
    Texture2D xclocklogo = LoadTextureFromImage(xclockImg);
    UnloadImage(xclockImg);
    
    Image xcalcImg = LoadImageFromMemory(".png", xcalc_png, xcalc_png_len);
    Texture2D xcalclogo = LoadTextureFromImage(xcalcImg);
    UnloadImage(xcalcImg);
    Image xeyesImg = LoadImageFromMemory(".png", xeyes_png, xeyes_png_len);
    Texture2D xeyeslogo = LoadTextureFromImage(xeyesImg);
    UnloadImage(xeyesImg);

    Font guiFont = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, 0, 95);

    // FWVM 3.x Color Scheme - Modern Flat Windows Style
    Color fwvm_bg = {45, 45, 48, 255};           // Dark charcoal background
    Color fwvm_accent = {0, 120, 215, 255};      // Windows 10 blue accent
    Color fwvm_white = {255, 255, 255, 255};     // Pure white
    Color fwvm_light_gray = {240, 240, 240, 255}; // Light gray for panels
    Color fwvm_dark_gray = {32, 32, 32, 255};    // Dark panel color
    Color fwvm_taskbar = {25, 25, 25, 255};      // Almost black taskbar
    Color fwvm_border = {76, 76, 76, 255};       // Subtle border color
    Color fwvm_hover = {70, 130, 180, 255};      // Hover state blue
    Color fwvm_term_bg = {12, 12, 12, 255};      // Terminal background
    Color fwvm_term_fg = {204, 204, 204, 255};   // Terminal text

    // State variables
    int running = 1;
    int frameCount = 0;
    int showContextMenu = 0;
    Vector2 contextMenuPos = {0, 0};
    int contextMenuSetFrame = 0;
    
    // Terminal state
    int terminal_open = 0;
    int terminal_minimized = 0;
    int terminal_dragging = 0;
    Vector2 terminal_drag_offset = {0, 0};
    Rectangle termWin = {screenWidth * 0.31f, screenHeight * 0.23f, screenWidth * 0.62f, screenHeight * 0.55f};
    char inputLine[256] = {0};
    int inputPos = 0;
    int terminal_focused = 0;
    int clear_terminal_requested = 0;
    
    // xcalc state
    int xcalc_open = 0;
    int xcalc_minimized = 0;
    int xcalc_resizing = 0;
    int xcalc_close_hover = 0;
    int xcalc_workspace = 0;
    int xcalc_sticky = 0;
    Rectangle xcalcWin = {400, 200, 220, 300};
    
    // xclock state
    int xclock_open = 0;
    int xclock_minimized = 0;
    int xclock_dragging = 0;
    Vector2 xclock_drag_offset = {0, 0};
    Rectangle xclockWin = {600, 150, 200, 200};
    
    // Utilities window state
    int utilities_open = 0;
    int utilities_minimized = 0;
    int utilities_dragging = 0;
    Vector2 utilities_drag_offset = {0, 0};
    Rectangle utilitiesWin = {300, 200, 280, 200};
    
    // About dialog state
    int about_open = 0;
    int about_dragging = 0;
    Vector2 about_drag_offset = {0, 0};
    Rectangle aboutWin = {350, 250, 400, 200};
    
    // XEyes state
    int xeyes_open = 0;
    int xeyes_minimized = 0;
    int xeyes_dragging = 0;
    Vector2 xeyes_drag_offset = {0, 0};
    Rectangle xeyesWin = {500, 300, 240, 150};
    
    // XLogo state
    int xlogo_open = 0;
    int xlogo_minimized = 0;
    int xlogo_dragging = 0;
    Vector2 xlogo_drag_offset = {0, 0};
    Rectangle xlogoWin = {450, 250, 200, 200};
    // Window management
    int last_clicked = -1;

    int focused_window = -1;
    int workspace = 0;
    const int WIN_TERM = 1;
    const int WIN_XCALC = 2;
    const int WIN_XCLOCK = 3;
    const int WIN_UTILITIES = 4;
    const int WIN_XEYES = 5;
    const int WIN_XLOGO = 6;
    
    // Shell process
    ChildProc* shell = NULL;

    printf("DEBUG: Starting main loop, running=%d, WindowShouldClose()=%d\n", running, WindowShouldClose());
    fflush(stdout);

    // Main loop
    while (running && !WindowShouldClose()) {
        frameCount++;

        // Handle fullscreen toggle (F11 key)
        if (IsKeyPressed(KEY_F11)) {
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                // Get monitor size and switch to fullscreen
                int monitor = GetCurrentMonitor();
                screenWidth = GetMonitorWidth(monitor);
                screenHeight = GetMonitorHeight(monitor);
                SetWindowSize(screenWidth, screenHeight);
                ToggleFullscreen();
            } else {
                // Switch back to windowed mode
                ToggleFullscreen();
                screenWidth = 1024;
                screenHeight = 768;
                SetWindowSize(screenWidth, screenHeight);
            }
        }

        // Begin drawing
        BeginDrawing();
        
        // Clear background - FWVM 3.x flat style
        ClearBackground(fwvm_bg);
        
        // Draw modern flat top bar
        int topBarH = 48;
        DrawRectangle(0, 0, screenWidth, topBarH, fwvm_taskbar);
        DrawRectangle(0, topBarH-1, screenWidth, 1, fwvm_border); // Subtle border
        DrawTextEx(guiFont, "X11 Desktop", (Vector2){10, 12}, 20, 0.0f, fwvm_white);
        
        // Show fullscreen toggle hint
        const char* fullscreenHint = isFullscreen ? "F11: Exit Fullscreen" : "F11: Fullscreen";
        Vector2 hintSize = MeasureTextEx(guiFont, fullscreenHint, 14, 0.0f);
        DrawTextEx(guiFont, fullscreenHint, (Vector2){screenWidth - hintSize.x - 10, 16}, 14, 0.0f, fwvm_white);
        
        // Draw desktop icons
        float iconScale = 0.18f;
        int icon_x = 32;
        int icon_y = topBarH + 32;
        int icon_w = (int)(logo.width * iconScale);
        int icon_h = (int)(logo.height * iconScale);
        
        // FWVM desktop icons with flat style
        DrawTextureEx(logo, (Vector2){(float)icon_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "X11", (Vector2){(float)icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);
        
        // Terminal icon
        int term_icon_x = icon_x + icon_w + 32;
        DrawTextureEx(xtermlogo, (Vector2){(float)term_icon_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "Terminal", (Vector2){(float)term_icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);
        
        // Calculator icon  
        int calc_icon_x = term_icon_x + icon_w + 32;
        DrawTextureEx(xcalclogo, (Vector2){(float)calc_icon_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "Calculator", (Vector2){(float)calc_icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);
        
        // Clock icon
        int clock_icon_x = calc_icon_x + icon_w + 32;
        DrawTextureEx(xclocklogo, (Vector2){(float)clock_icon_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "Clock", (Vector2){(float)clock_icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);
        
        // Utilities icon (using xcalc logo as placeholder)
        int utils_icon_x = clock_icon_x + icon_w + 32;
        DrawTextureEx(xcalclogo, (Vector2){(float)utils_icon_x, (float)icon_y}, 0.0f, iconScale, (Color){150, 200, 255, 255}); // Light blue tint
        DrawTextEx(guiFont, "Utilities", (Vector2){(float)utils_icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);
        
        // XEyes icon (using xeyes logo)
        int xeyes_icon_x = utils_icon_x + icon_w + 32;
        DrawTextureEx(xeyeslogo, (Vector2){(float)xeyes_icon_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "XEyes", (Vector2){(float)xeyes_icon_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);

        int xlogo_x = xeyes_icon_x + icon_w + 32;
        DrawTextureEx(logo, (Vector2){(float)xlogo_x, (float)icon_y}, 0.0f, iconScale, fwvm_white);
        DrawTextEx(guiFont, "XLogo", (Vector2){(float)xlogo_x, (float)(icon_y + icon_h + 4)}, 14, 0.0f, fwvm_white);


        // Handle right-click context menu
        if (frameCount > 60 && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            showContextMenu = 1;
            contextMenuPos = mouse;
            contextMenuSetFrame = frameCount;
        }

        // Handle left-click on context menu
        if (showContextMenu && frameCount > 90 && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && frameCount > (contextMenuSetFrame + 10)) {
            Vector2 mouse = GetMousePosition();
            int menuW = 180, menuH = 32;
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            
            // Clamp menu to screen
            if (menuX + menuW > screenWidth) menuX = screenWidth - menuW - 8;
            if (menuY + menuH * 7 > screenHeight) menuY = screenHeight - menuH * 7 - 8;
            
            Rectangle xtermRect = {menuX, menuY, menuW, menuH};
            Rectangle xcalcRect = {menuX, menuY + menuH, menuW, menuH};
            Rectangle xclockRect = {menuX, menuY + 2*menuH, menuW, menuH};
            Rectangle utilitiesRect = {menuX, menuY + 3*menuH, menuW, menuH};
            Rectangle xeyesRect = {menuX, menuY + 4*menuH, menuW, menuH};
            Rectangle xlogoRect = {menuX, menuY + 5*menuH, menuW, menuH};
            Rectangle aboutRect = {menuX, menuY + 6*menuH, menuW, menuH};
            Rectangle exitRect = {menuX, menuY + 7*menuH, menuW, menuH};
            
            if (CheckCollisionPointRec(mouse, xtermRect)) {
                if (!terminal_open) {
                    terminal_open = 1;
                    if (!shell) {
                        shell = CreateChildProc();
                        LaunchShell(shell, "C:\\WNU\\WNU OS\\wnuos.exe");
                    }
                    last_clicked = WIN_TERM;
                }
            } else if (CheckCollisionPointRec(mouse, xcalcRect)) {
                if (!xcalc_open) {
                    xcalc_open = 1;
                    last_clicked = WIN_XCALC;
                }
            } else if (CheckCollisionPointRec(mouse, xclockRect)) {
                if (!xclock_open) {
                    xclock_open = 1;
                    last_clicked = WIN_XCLOCK;
                }
            } else if (CheckCollisionPointRec(mouse, utilitiesRect)) {
                if (!utilities_open) {
                    utilities_open = 1;
                    last_clicked = WIN_UTILITIES;
                }
            } else if (CheckCollisionPointRec(mouse, xeyesRect)) {
                if (!xeyes_open) {
                    xeyes_open = 1;
                    last_clicked = WIN_XEYES;
                }
            } else if (CheckCollisionPointRec(mouse, xlogoRect)) {
                if (!xlogo_open) {
                    xlogo_open = 1;
                    last_clicked = WIN_XLOGO;
                }
            } else if (CheckCollisionPointRec(mouse, aboutRect)) {
                // Print to console and open about window
                printf("About X11: WNUOS GUI X11 WM: FVWM 3.x Made In: The C Programming Language Made With: raylib 5.5\n");
                fflush(stdout);
                if (!about_open) {
                    about_open = 1;
                }
            } else if (CheckCollisionPointRec(mouse, exitRect)) {
                running = 0;
            }
            showContextMenu = 0;
        }

        // Handle icon clicks
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Terminal icon
            Rectangle termIconRect = {(float)term_icon_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, termIconRect)) {
                if (!terminal_open) {
                    terminal_open = 1;
                    if (!shell) {
                        shell = CreateChildProc();
                        LaunchShell(shell, "C:\\WNU\\WNU OS\\wnuos.exe");
                    }
                    last_clicked = WIN_TERM;
                }
            }
            
            // Calculator icon
            Rectangle calcIconRect = {(float)calc_icon_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, calcIconRect)) {
                if (!xcalc_open) {
                    xcalc_open = 1;
                    last_clicked = WIN_XCALC;
                }
            }
            
            // Clock icon
            Rectangle clockIconRect = {(float)clock_icon_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, clockIconRect)) {
                if (!xclock_open) {
                    xclock_open = 1;
                    last_clicked = WIN_XCLOCK;
                }
            }
            
            // Utilities icon
            Rectangle utilitiesIconRect = {(float)utils_icon_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, utilitiesIconRect)) {
                if (!utilities_open) {
                    utilities_open = 1;
                    last_clicked = WIN_UTILITIES;
                }
            }
            
            // XEyes icon
            Rectangle xeyesIconRect = {(float)xeyes_icon_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, xeyesIconRect)) {
                if (!xeyes_open) {
                    xeyes_open = 1;
                    last_clicked = WIN_XEYES;
                }
            }

            // XLogo icon
            Rectangle xlogoIconRect = {(float)xlogo_x, (float)icon_y, (float)icon_w, (float)icon_h};
            if (CheckCollisionPointRec(mouse, xlogoIconRect)) {
                if (!xlogo_open) {
                    xlogo_open = 1;
                    last_clicked = WIN_XLOGO;
                }
            }
        }

        // Draw FWVM 3.x flat context menu
        if (showContextMenu) {
            int menuW = 180, menuH = 32;
            int menuX = (int)contextMenuPos.x;
            int menuY = (int)contextMenuPos.y;
            
            // Clamp menu to screen
            if (menuX + menuW > screenWidth) menuX = screenWidth - menuW - 8;
            if (menuY + menuH * 8 > screenHeight) menuY = screenHeight - menuH * 8 - 8;
            
            // Modern flat menu styling with shadow
            DrawRectangle(menuX + 2, menuY + 2, menuW, menuH * 8, (Color){0, 0, 0, 50});
            DrawRectangle(menuX, menuY, menuW, menuH * 8, fwvm_light_gray);
            DrawRectangleLines(menuX, menuY, menuW, menuH * 8, fwvm_border);
            
            DrawTextEx(guiFont, "Terminal", (Vector2){(float)(menuX + 8), (float)(menuY + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Calculator", (Vector2){(float)(menuX + 8), (float)(menuY + menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Clock", (Vector2){(float)(menuX + 8), (float)(menuY + 2*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Utilities", (Vector2){(float)(menuX + 8), (float)(menuY + 3*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "XEyes", (Vector2){(float)(menuX + 8), (float)(menuY + 4*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "XLogo", (Vector2){(float)(menuX + 8), (float)(menuY + 5*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "About FWVM 3.x", (Vector2){(float)(menuX + 8), (float)(menuY + 6*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Exit", (Vector2){(float)(menuX + 8), (float)(menuY + 7*menuH + 8)}, 16, 0.0f, fwvm_dark_gray);
        }

        // Draw FWVM 3.x flat terminal window
        if (terminal_open && !terminal_minimized) {
            int border = 1;
            int titleH = 32;
            
            // Modern flat window frame with subtle shadow
            DrawRectangle((int)termWin.x + 2, (int)termWin.y + 2, (int)termWin.width, (int)termWin.height, (Color){0, 0, 0, 60}); // Shadow
            DrawRectangleRec(termWin, fwvm_border);
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, 
                         (int)termWin.width - 2*border, (int)termWin.height - 2*border, fwvm_term_bg);
            
            // Modern flat title bar
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border, 
                         (int)termWin.width - 2*border, titleH, fwvm_taskbar);
            DrawTextEx(guiFont, "Terminal", (Vector2){termWin.x + border + 12, termWin.y + border + 8}, 
                      16, 0.0f, fwvm_white);
            
            // Modern flat close button
            int closeBtnSz = titleH - 8;
            int closeBtnX = (int)(termWin.x + termWin.width - closeBtnSz - border - 8);
            int closeBtnY = (int)(termWin.y + border + 4);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, (Color){232, 17, 35, 255}); // Windows red
            DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 
                      14, 0.0f, fwvm_white);
            
            // Terminal content area
            DrawRectangle((int)termWin.x + border, (int)termWin.y + border + titleH, 
                         (int)termWin.width - 2*border, (int)termWin.height - 2*border - titleH, fwvm_term_bg);
            
            // Terminal text with FWVM branding and WNU OS executable info (much bigger text)
            DrawTextEx(guiFont, "Terminal", 
                      (Vector2){termWin.x + 16, termWin.y + titleH + 16}, 24, 0.0f, fwvm_term_fg);
            DrawTextEx(guiFont, "Running: C:\\WNU\\WNU OS\\wnuos.exe", 
                      (Vector2){termWin.x + 16, termWin.y + titleH + 72}, 18, 0.0f, (Color){100, 200, 100, 255});
            
            // Read and display shell output if available (much bigger text with auto-clear)
            static char terminalBuffer[4096] = {0};
            static int bufferPos = 0;
            static int lineCount = 0;
            
            // Handle clear terminal request
            if (clear_terminal_requested) {
                memset(terminalBuffer, 0, sizeof(terminalBuffer));
                bufferPos = 0;
                lineCount = 0;
                clear_terminal_requested = 0;
            }
            
            if (shell && ChildProcessAlive(shell)) {
                char readBuf[256];
                int bytesRead = ReadShellOutput(shell, readBuf, sizeof(readBuf) - 1);
                if (bytesRead > 0) {
                    readBuf[bytesRead] = '\0';
                    // Append to buffer
                    if (bufferPos + bytesRead < sizeof(terminalBuffer) - 1) {
                        strcat(terminalBuffer + bufferPos, readBuf);
                        bufferPos += bytesRead;
                        
                        // Count newlines to track lines
                        for (int i = 0; i < bytesRead; i++) {
                            if (readBuf[i] == '\n') lineCount++;
                        }
                    }
                }
                
                // Calculate available space for text (bigger line height for bigger text)
                float availableHeight = termWin.height - titleH - 120; // Space below header info
                int maxLines = (int)(availableHeight / 22); // 22px line height for 20px text
                
                // Auto-clear if we have too many lines
                if (lineCount > maxLines) {
                    memset(terminalBuffer, 0, sizeof(terminalBuffer));
                    bufferPos = 0;
                    lineCount = 0;
                    strcpy(terminalBuffer, "[Terminal cleared - too many lines]\n");
                    bufferPos = strlen(terminalBuffer);
                    lineCount = 1;
                }
                
                // Display terminal output with much bigger text
                if (strlen(terminalBuffer) > 0) {
                    DrawTextEx(guiFont, terminalBuffer, 
                              (Vector2){termWin.x + 16, termWin.y + titleH + 98}, 20, 0.0f, fwvm_term_fg);
                }
            } else {
                DrawTextEx(guiFont, "> _", 
                          (Vector2){termWin.x + 16, termWin.y + titleH + 98}, 22, 0.0f, fwvm_term_fg);
            }
            
            // Draw current input line
            char promptLine[512];
            snprintf(promptLine, sizeof(promptLine), "> %s%s", inputLine, terminal_focused ? "_" : "");
            float inputY = termWin.y + termWin.height - 50; // Near bottom of terminal
            DrawTextEx(guiFont, promptLine, (Vector2){termWin.x + 16, inputY}, 18, 0.0f, 
                      terminal_focused ? fwvm_accent : fwvm_term_fg);
            
            // Handle terminal close button
            Rectangle closeBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, closeBtnRect)) {
                terminal_open = 0;
                if (shell) {
                    CloseShell(shell);
                    DestroyChildProc(shell);
                    shell = NULL;
                }
            }
            
            // Handle terminal window dragging
            Rectangle titleBarRect = {termWin.x + border, termWin.y + border, termWin.width - 2*border - closeBtnSz - 8, titleH};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, titleBarRect)) {
                terminal_dragging = 1;
                terminal_drag_offset.x = mouse.x - termWin.x;
                terminal_drag_offset.y = mouse.y - termWin.y;
                last_clicked = WIN_TERM;
            }
        }
        
        // Handle terminal keyboard input
        if (terminal_open && terminal_focused) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key < 127 && inputPos < sizeof(inputLine) - 1) {
                    inputLine[inputPos] = (char)key;
                    inputPos++;
                    inputLine[inputPos] = '\0';
                }
                key = GetCharPressed();
            }
            
            // Handle special keys
            if (IsKeyPressed(KEY_ENTER)) {
                if (shell && strlen(inputLine) > 0) {
                    strcat(inputLine, "\n");
                    WriteShellInput(shell, inputLine, strlen(inputLine));
                }
                memset(inputLine, 0, sizeof(inputLine));
                inputPos = 0;
            }
            
            if (IsKeyPressed(KEY_BACKSPACE) && inputPos > 0) {
                inputPos--;
                inputLine[inputPos] = '\0';
            }
        }
        
        // Set terminal focus when clicked
        if (terminal_open && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            Rectangle termContentRect = {termWin.x, termWin.y + 32, termWin.width, termWin.height - 32};
            terminal_focused = CheckCollisionPointRec(mouse, termContentRect) ? 1 : 0;
        }
        
        // Handle terminal drag movement (outside the drawing block to ensure it works)
        if (terminal_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            termWin.x = mousePos.x - terminal_drag_offset.x;
            termWin.y = mousePos.y - terminal_drag_offset.y;
            
            // Keep window on screen
            if (termWin.x < 0) termWin.x = 0;
            if (termWin.y < 48) termWin.y = 48; // Below top bar
            if (termWin.x + termWin.width > screenWidth) termWin.x = screenWidth - termWin.width;
            if (termWin.y + termWin.height > screenHeight - 48) termWin.y = screenHeight - 48 - termWin.height; // Above taskbar
        }
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            terminal_dragging = 0;
            xclock_dragging = 0;
            utilities_dragging = 0;
        }

        // Draw FWVM 3.x xclock window
        if (xclock_open && !xclock_minimized) {
            int border = 1;
            int titleH = 32;
            
            // Modern flat clock window with shadow
            DrawRectangle((int)xclockWin.x + 2, (int)xclockWin.y + 2, (int)xclockWin.width, (int)xclockWin.height, (Color){0, 0, 0, 60}); // Shadow
            DrawRectangleRec(xclockWin, fwvm_border);
            DrawRectangle((int)xclockWin.x + border, (int)xclockWin.y + border, 
                         (int)xclockWin.width - 2*border, (int)xclockWin.height - 2*border, fwvm_light_gray);
            
            // Modern flat title bar
            DrawRectangle((int)xclockWin.x + border, (int)xclockWin.y + border, 
                         (int)xclockWin.width - 2*border, titleH, fwvm_taskbar);
            DrawTextEx(guiFont, "Clock", (Vector2){xclockWin.x + border + 12, xclockWin.y + border + 8}, 
                      16, 0.0f, fwvm_white);
            
            // Modern flat close button
            int closeBtnSz = titleH - 8;
            int closeBtnX = (int)(xclockWin.x + xclockWin.width - closeBtnSz - border - 8);
            int closeBtnY = (int)(xclockWin.y + border + 4);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, (Color){232, 17, 35, 255}); // Windows red
            DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 
                      14, 0.0f, fwvm_white);
            
            // Clock face area
            float clockCenterX = xclockWin.x + xclockWin.width / 2;
            float clockCenterY = xclockWin.y + titleH + (xclockWin.height - titleH) / 2 + 10;
            float clockRadius = (xclockWin.width < xclockWin.height - titleH ? xclockWin.width : xclockWin.height - titleH) / 2 - 20;
            
            // Draw clock face
            DrawCircle((int)clockCenterX, (int)clockCenterY, clockRadius, fwvm_white);
            DrawCircleLines((int)clockCenterX, (int)clockCenterY, clockRadius, fwvm_dark_gray);
            
            // Get current time for clock hands
            time_t now = time(NULL);
            struct tm* tm_info = localtime(&now);
            
            // Draw hour markers
            for (int i = 0; i < 12; i++) {
                float angle = (i * 30 - 90) * DEG2RAD;
                float x1 = clockCenterX + (clockRadius - 15) * cosf(angle);
                float y1 = clockCenterY + (clockRadius - 15) * sinf(angle);
                float x2 = clockCenterX + (clockRadius - 5) * cosf(angle);
                float y2 = clockCenterY + (clockRadius - 5) * sinf(angle);
                DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 3, fwvm_dark_gray);
            }
            
            // Draw hour hand
            float hourAngle = ((tm_info->tm_hour % 12) * 30 + tm_info->tm_min * 0.5f - 90) * DEG2RAD;
            float hourX = clockCenterX + (clockRadius * 0.5f) * cosf(hourAngle);
            float hourY = clockCenterY + (clockRadius * 0.5f) * sinf(hourAngle);
            DrawLineEx((Vector2){clockCenterX, clockCenterY}, (Vector2){hourX, hourY}, 6, fwvm_dark_gray);
            
            // Draw minute hand
            float minuteAngle = (tm_info->tm_min * 6 - 90) * DEG2RAD;
            float minuteX = clockCenterX + (clockRadius * 0.7f) * cosf(minuteAngle);
            float minuteY = clockCenterY + (clockRadius * 0.7f) * sinf(minuteAngle);
            DrawLineEx((Vector2){clockCenterX, clockCenterY}, (Vector2){minuteX, minuteY}, 4, fwvm_accent);
            
            // Draw second hand
            float secondAngle = (tm_info->tm_sec * 6 - 90) * DEG2RAD;
            float secondX = clockCenterX + (clockRadius * 0.8f) * cosf(secondAngle);
            float secondY = clockCenterY + (clockRadius * 0.8f) * sinf(secondAngle);
            DrawLineEx((Vector2){clockCenterX, clockCenterY}, (Vector2){secondX, secondY}, 2, (Color){232, 17, 35, 255});
            
            // Center dot
            DrawCircle((int)clockCenterX, (int)clockCenterY, 4, fwvm_dark_gray);
            
            // Digital time display
            char digitalTime[32];
            strftime(digitalTime, sizeof(digitalTime), "%H:%M:%S", tm_info);
            Vector2 timeSize = MeasureTextEx(guiFont, digitalTime, 16, 0.0f);
            DrawTextEx(guiFont, digitalTime, 
                      (Vector2){clockCenterX - timeSize.x/2, xclockWin.y + xclockWin.height - 30}, 
                      16, 0.0f, fwvm_dark_gray);
            
            // Handle clock close button
            Rectangle clockCloseBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, clockCloseBtnRect)) {
                xclock_open = 0;
                if (last_clicked == WIN_XCLOCK) last_clicked = -1;
            }
            
            // Handle xclock window dragging
            Rectangle clockTitleBarRect = {xclockWin.x + border, xclockWin.y + border, xclockWin.width - 2*border - closeBtnSz - 8, titleH};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, clockTitleBarRect)) {
                xclock_dragging = 1;
                xclock_drag_offset.x = mouse.x - xclockWin.x;
                xclock_drag_offset.y = mouse.y - xclockWin.y;
                last_clicked = WIN_XCLOCK;
            }
        }
        
        // Handle xclock drag movement
        if (xclock_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            xclockWin.x = mousePos.x - xclock_drag_offset.x;
            xclockWin.y = mousePos.y - xclock_drag_offset.y;
            
            // Keep window on screen
            if (xclockWin.x < 0) xclockWin.x = 0;
            if (xclockWin.y < 48) xclockWin.y = 48; // Below top bar
            if (xclockWin.x + xclockWin.width > screenWidth) xclockWin.x = screenWidth - xclockWin.width;
            if (xclockWin.y + xclockWin.height > screenHeight - 48) xclockWin.y = screenHeight - 48 - xclockWin.height; // Above taskbar
        }

        // Draw xcalc window using the widget function
        draw_xcalc_widget(&xcalcWin, &xcalc_open, &xcalc_minimized, &xcalc_resizing, &xcalc_close_hover,
                         &xcalc_workspace, &xcalc_sticky, guiFont, xcalclogo,
                         &last_clicked, WIN_XCALC, &focused_window, workspace, screenWidth, screenHeight);

        // Draw XEyes window
        draw_xeyes_window(&xeyesWin, &xeyes_open, &xeyes_minimized, &xeyes_dragging, &xeyes_drag_offset,
                         &last_clicked, WIN_XEYES, guiFont, screenWidth, screenHeight);

        // Draw XLogo window
        draw_xlogo_window(&xlogoWin, &xlogo_open, &xlogo_minimized, &xlogo_dragging, &xlogo_drag_offset,
                         &focused_window, guiFont, logo, fwvm_bg, fwvm_white, fwvm_dark_gray,
                         fwvm_accent, WIN_XLOGO, &last_clicked, screenWidth, screenHeight);

        // Draw FWVM 3.x Utilities window
        if (utilities_open && !utilities_minimized) {
            int border = 1;
            int titleH = 32;
            
            // Modern flat utilities window with shadow
            DrawRectangle((int)utilitiesWin.x + 2, (int)utilitiesWin.y + 2, (int)utilitiesWin.width, (int)utilitiesWin.height, (Color){0, 0, 0, 60}); // Shadow
            DrawRectangleRec(utilitiesWin, fwvm_border);
            DrawRectangle((int)utilitiesWin.x + border, (int)utilitiesWin.y + border, 
                         (int)utilitiesWin.width - 2*border, (int)utilitiesWin.height - 2*border, fwvm_light_gray);
            
            // Modern flat title bar
            DrawRectangle((int)utilitiesWin.x + border, (int)utilitiesWin.y + border, 
                         (int)utilitiesWin.width - 2*border, titleH, fwvm_taskbar);
            DrawTextEx(guiFont, "Utilities", (Vector2){utilitiesWin.x + border + 12, utilitiesWin.y + border + 8}, 
                      16, 0.0f, fwvm_white);
            
            // Modern flat close button
            int closeBtnSz = titleH - 8;
            int closeBtnX = (int)(utilitiesWin.x + utilitiesWin.width - closeBtnSz - border - 8);
            int closeBtnY = (int)(utilitiesWin.y + border + 4);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, (Color){232, 17, 35, 255}); // Windows red
            DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 
                      14, 0.0f, fwvm_white);
            
            // Utility buttons
            int btnW = 200, btnH = 30;
            int btnX = (int)utilitiesWin.x + 40;
            int btnY = (int)utilitiesWin.y + titleH + 20;
            
            // Clear Terminal button
            Rectangle clearTermBtn = {btnX, btnY, btnW, btnH};
            int clearHover = CheckCollisionPointRec(mouse, clearTermBtn);
            DrawRectangleRec(clearTermBtn, clearHover ? fwvm_hover : fwvm_accent);
            DrawRectangleLinesEx(clearTermBtn, 1, fwvm_border);
            DrawTextEx(guiFont, "Clear Terminal", (Vector2){btnX + 60, btnY + 6}, 16, 0.0f, fwvm_white);
            
            // Open Clock button
            Rectangle openClockBtn = {btnX, btnY + 40, btnW, btnH};
            int clockHover = CheckCollisionPointRec(mouse, openClockBtn);
            DrawRectangleRec(openClockBtn, clockHover ? fwvm_hover : fwvm_accent);
            DrawRectangleLinesEx(openClockBtn, 1, fwvm_border);
            DrawTextEx(guiFont, "Open Clock", (Vector2){btnX + 70, btnY + 46}, 16, 0.0f, fwvm_white);
            
            // Open Calculator button
            Rectangle openCalcBtn = {btnX, btnY + 80, btnW, btnH};
            int calcHover = CheckCollisionPointRec(mouse, openCalcBtn);
            DrawRectangleRec(openCalcBtn, calcHover ? fwvm_hover : fwvm_accent);
            DrawRectangleLinesEx(openCalcBtn, 1, fwvm_border);
            DrawTextEx(guiFont, "Open Calculator", (Vector2){btnX + 55, btnY + 86}, 16, 0.0f, fwvm_white);
            
            // Handle button clicks
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, clearTermBtn)) {
                    // Request terminal clear
                    clear_terminal_requested = 1;
                } else if (CheckCollisionPointRec(mouse, openClockBtn)) {
                    if (!xclock_open) {
                        xclock_open = 1;
                        last_clicked = WIN_XCLOCK;
                    }
                } else if (CheckCollisionPointRec(mouse, openCalcBtn)) {
                    if (!xcalc_open) {
                        xcalc_open = 1;
                        last_clicked = WIN_XCALC;
                    }
                }
            }
            
            // Handle utilities close button
            Rectangle utilitiesCloseBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, utilitiesCloseBtnRect)) {
                utilities_open = 0;
                if (last_clicked == WIN_UTILITIES) last_clicked = -1;
            }
            
            // Handle utilities window dragging
            Rectangle utilitiesTitleBarRect = {utilitiesWin.x + border, utilitiesWin.y + border, utilitiesWin.width - 2*border - closeBtnSz - 8, titleH};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, utilitiesTitleBarRect)) {
                utilities_dragging = 1;
                utilities_drag_offset.x = mouse.x - utilitiesWin.x;
                utilities_drag_offset.y = mouse.y - utilitiesWin.y;
                last_clicked = WIN_UTILITIES;
            }
        }
        
        // Handle utilities drag movement
        if (utilities_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            utilitiesWin.x = mousePos.x - utilities_drag_offset.x;
            utilitiesWin.y = mousePos.y - utilities_drag_offset.y;
            
            // Keep window on screen
            if (utilitiesWin.x < 0) utilitiesWin.x = 0;
            if (utilitiesWin.y < 48) utilitiesWin.y = 48; // Below top bar
            if (utilitiesWin.x + utilitiesWin.width > screenWidth) utilitiesWin.x = screenWidth - utilitiesWin.width;
            if (utilitiesWin.y + utilitiesWin.height > screenHeight - 48) utilitiesWin.y = screenHeight - 48 - utilitiesWin.height; // Above taskbar
        }

        // Draw FWVM 3.x About Dialog
        if (about_open) {
            int border = 1;
            int titleH = 32;
            
            // Modern flat about dialog with shadow
            DrawRectangle((int)aboutWin.x + 2, (int)aboutWin.y + 2, (int)aboutWin.width, (int)aboutWin.height, (Color){0, 0, 0, 60}); // Shadow
            DrawRectangleRec(aboutWin, fwvm_border);
            DrawRectangle((int)aboutWin.x + border, (int)aboutWin.y + border, 
                         (int)aboutWin.width - 2*border, (int)aboutWin.height - 2*border, fwvm_light_gray);
            
            // Modern flat title bar
            DrawRectangle((int)aboutWin.x + border, (int)aboutWin.y + border, 
                         (int)aboutWin.width - 2*border, titleH, fwvm_taskbar);
            DrawTextEx(guiFont, "About X11", (Vector2){aboutWin.x + border + 12, aboutWin.y + border + 8}, 
                      16, 0.0f, fwvm_white);
            
            // Modern flat close button
            int closeBtnSz = titleH - 8;
            int closeBtnX = (int)(aboutWin.x + aboutWin.width - closeBtnSz - border - 8);
            int closeBtnY = (int)(aboutWin.y + border + 4);
            DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, (Color){232, 17, 35, 255}); // Windows red
            DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 
                      14, 0.0f, fwvm_white);
            
            // About dialog content
            float contentY = aboutWin.y + titleH + 20;
            DrawTextEx(guiFont, "WNUOS GUI X11 WM: FVWM 3.x", 
                      (Vector2){aboutWin.x + 20, contentY}, 18, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Flat Windows Virtual Machine Desktop Environment", 
                      (Vector2){aboutWin.x + 20, contentY + 30}, 14, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Built with raylib graphics library", 
                      (Vector2){aboutWin.x + 20, contentY + 50}, 14, 0.0f, fwvm_dark_gray);
            DrawTextEx(guiFont, "Copyright (c) WNU Project 2025", 
                      (Vector2){aboutWin.x + 20, contentY + 80}, 14, 0.0f, fwvm_dark_gray);
            
            // Handle about close button
            Rectangle aboutCloseBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, aboutCloseBtnRect)) {
                about_open = 0;
            }
            
            // Handle about window dragging
            Rectangle aboutTitleBarRect = {aboutWin.x + border, aboutWin.y + border, aboutWin.width - 2*border - closeBtnSz - 8, titleH};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, aboutTitleBarRect)) {
                about_dragging = 1;
                about_drag_offset.x = mouse.x - aboutWin.x;
                about_drag_offset.y = mouse.y - aboutWin.y;
            }
        }
        
        // Handle about drag movement
        if (about_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            aboutWin.x = mousePos.x - about_drag_offset.x;
            aboutWin.y = mousePos.y - about_drag_offset.y;
            
            // Keep window on screen
            if (aboutWin.x < 0) aboutWin.x = 0;
            if (aboutWin.y < 48) aboutWin.y = 48; // Below top bar
            if (aboutWin.x + aboutWin.width > screenWidth) aboutWin.x = screenWidth - aboutWin.width;
            if (aboutWin.y + aboutWin.height > screenHeight - 48) aboutWin.y = screenHeight - 48 - aboutWin.height; // Above taskbar
        }
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            terminal_dragging = 0;
            xclock_dragging = 0;
            utilities_dragging = 0;
            about_dragging = 0;
            xlogo_dragging = 0;
        }

        // Draw FWVM 3.x modern taskbar
        int taskbarH = 48;
        int taskbarY = screenHeight - taskbarH;
        DrawRectangle(0, taskbarY, screenWidth, taskbarH, fwvm_taskbar);
        DrawRectangle(0, taskbarY, screenWidth, 1, fwvm_border); // Top border
        
        // FWVM start button area
        int startBtnW = 80;
        DrawRectangle(8, taskbarY + 6, startBtnW, taskbarH - 12, fwvm_accent);
        DrawTextEx(guiFont, "X11", (Vector2){18, (float)(taskbarY + 14)}, 18, 0.0f, fwvm_white);
        
        // Current time with modern styling
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char timeStr[64];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);
        Vector2 timeSz = MeasureTextEx(guiFont, timeStr, 16, 0.0f);
        
        // Time background panel
        int timePanelW = (int)timeSz.x + 20;
        DrawRectangle(screenWidth - timePanelW - 8, taskbarY + 6, timePanelW, taskbarH - 12, fwvm_dark_gray);
        DrawTextEx(guiFont, timeStr, (Vector2){screenWidth - timeSz.x - 18, (float)(taskbarY + 16)}, 
                  16, 0.0f, fwvm_white);

        EndDrawing();
    }

    // Cleanup
    if (shell) {
        CloseShell(shell);
        DestroyChildProc(shell);
        shell = NULL;
    }

    printf("waiting for X server to shut down .\n");

    UnloadTexture(logo);
    UnloadTexture(xtermlogo);
    UnloadTexture(xclocklogo);
    UnloadTexture(xcalclogo);
    UnloadFont(guiFont);
    CloseWindow();
    printf("INFO: Server terminated successfully\n");
    
    return 0;
}