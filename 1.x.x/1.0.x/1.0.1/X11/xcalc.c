#include <stdio.h>
#include <string.h>
#include <math.h>
#include "raylib.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define XC_MAX_LABEL 256

/* Copied and adapted xcalc widget from x11.c into a standalone app. */
static void draw_xcalc_widget(Rectangle *xcalcWin, int *xcalc_open, int *xcalc_minimized, int *xcalc_resizing, int *xcalc_close_hover,
                              int *xcalc_workspace, int *xcalc_sticky, Font guiFont, Texture2D xcalclogo,
                              int *last_clicked, int WIN_XCALC,
                              int *focused_window, int workspace, int screenWidth, int screenHeight) {
    static int xcalc_dragging = 0;
    static Vector2 xcalcDragOffset = {0};
    // persistent calculator state
    static char calc_display[128] = "0";
    static double calc_pending = 0.0;
    static char calc_op = 0; /* '+','-','*','/' or 0 */
    static int calc_entering = 0; /* 1 when user typing a number */
    static int calc_error = 0;

    if (!xcalcWin || !xcalc_open) return;

    int xb = (int)(screenWidth * 0.002f);
    int xTitleH = 28;

    if (*xcalc_open && !*xcalc_minimized && (*xcalc_workspace == workspace || *xcalc_sticky)) {
        DrawRectangle((int)xcalcWin->x, (int)xcalcWin->y, (int)xcalcWin->width, (int)xcalcWin->height, (Color){80,80,120,255});
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, (int)xcalcWin->height - 2*xb, (Color){255,255,255,255});
        DrawRectangle((int)xcalcWin->x + xb, (int)xcalcWin->y + xb, (int)xcalcWin->width - 2*xb, xTitleH, (Color){60,60,80,255});
        DrawTextEx(guiFont, "xcalc", (Vector2){(float)xcalcWin->x + xb + 8, (float)xcalcWin->y + xb + 6}, 14.0f, 0.0f, (Color){255,255,255,255});
        int xCloseSz = xTitleH - 6;
        int xCloseX = (int)(xcalcWin->x + xcalcWin->width - xCloseSz - xb - 4);
        int xCloseY = (int)(xcalcWin->y + xb + 3);
        *xcalc_close_hover = CheckCollisionPointRec(GetMousePosition(), (Rectangle){(float)xCloseX,(float)xCloseY,(float)xCloseSz,(float)xCloseSz});
        DrawRectangle(xCloseX, xCloseY, xCloseSz, xCloseSz, *xcalc_close_hover ? RED : (Color){120,120,120,255});
        DrawTextEx(guiFont, "X", (Vector2){(float)(xCloseX + 4), (float)(xCloseY + 2)}, 14.0f, 0.0f, (Color){255,255,255,255});

        // Calculator UI: display and buttons
        float clientX = xcalcWin->x + xb + 8;
        float clientY = xcalcWin->y + xb + xTitleH + 8;
        float clientW = xcalcWin->width - 2*(xb + 8);
        float clientH = xcalcWin->height - (xTitleH + 8 + xb + 24);
        Rectangle disp = { clientX, clientY, clientW, 48 };
        DrawRectangleRec(disp, (Color){18,18,18,255});
        DrawRectangleLinesEx(disp, 2, (Color){120,120,160,255});
        float dispFont = 24.0f;
        Vector2 dsz = MeasureTextEx(guiFont, calc_display, dispFont, 0.0f);
        DrawTextEx(guiFont, calc_display, (Vector2){ disp.x + disp.width - dsz.x - 12, disp.y + (disp.height - dsz.y)/2 }, dispFont, 0.0f, (Color){255,255,255,255});

        const char* b_labels[] = { "AC","CE","+/-","%%","7","8","9","/","4","5","6","×s","1","2","3","-","0",".","=","+" };
        int nbuttons = sizeof(b_labels)/sizeof(b_labels[0]);
        int cols = 4; int rows = nbuttons / cols; float gap = 8.0f;
        float btnW = (clientW - (cols-1)*gap) / cols;
        float btnH = (clientH - disp.height - (rows+1)*gap) / rows;
        float bx0 = clientX; float by0 = clientY + disp.height + gap;

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < cols; c++) {
                    int idx = r*cols + c;
                    Rectangle br = { bx0 + c*(btnW+gap), by0 + r*(btnH+gap), btnW, btnH };
                    if (CheckCollisionPointRec(mouse, br)) {
                        const char* lab = b_labels[idx];
                        if (strcmp(lab, "AC") == 0) { strcpy(calc_display, "0"); calc_pending = 0.0; calc_op = 0; calc_entering = 0; calc_error = 0; }
                        else if (strcmp(lab, "CE") == 0) { strcpy(calc_display, "0"); calc_entering = 0; calc_error = 0; }
                        else if (strcmp(lab, "+/-") == 0) { if (calc_display[0] == '-') memmove(calc_display, calc_display+1, strlen(calc_display)); else if (strcmp(calc_display, "0") != 0) { char tmp[XC_MAX_LABEL]; snprintf(tmp, sizeof(tmp), "-%s", calc_display); strncpy(calc_display, tmp, sizeof(calc_display)-1); } }
                        else if (strcmp(lab, "+/-") == 0) { if (calc_display[0] == '-') memmove(calc_display, calc_display+1, strlen(calc_display)); else if (strcmp(calc_display, "0") != 0) { char tmp[XC_MAX_LABEL]; snprintf(tmp, sizeof(tmp) - 1, "-%s", calc_display); tmp[sizeof(tmp)-1] = '\0'; strncpy(calc_display, tmp, sizeof(calc_display)-1); calc_display[sizeof(calc_display)-1] = '\0'; } }
                        else if (strcmp(lab, "%%") == 0) { double v = atof(calc_display); v = v / 100.0; snprintf(calc_display, sizeof(calc_display), "%g", v); calc_entering = 0; }
                        else if (strcmp(lab, "=") == 0) { double cur = atof(calc_display); double res = cur; if (calc_op) { if (calc_op == '+') res = calc_pending + cur; else if (calc_op == '-') res = calc_pending - cur; else if (calc_op == '*') res = calc_pending * cur; else if (calc_op == '/') { if (cur == 0.0) { calc_error = 1; strncpy(calc_display, "ERR", sizeof(calc_display)-1); } else res = calc_pending / cur; } if (!calc_error) snprintf(calc_display, sizeof(calc_display), "%g", res); calc_pending = res; calc_op = 0; calc_entering = 0; } }
                        else if (strchr("+-*/", lab[0]) && lab[1] == '\0') { double cur = atof(calc_display); if (calc_op) { double res = cur; if (calc_op == '+') res = calc_pending + cur; else if (calc_op == '-') res = calc_pending - cur; else if (calc_op == '*') res = calc_pending * cur; else if (calc_op == '/') { if (cur == 0.0) { calc_error = 1; strncpy(calc_display, "ERR", sizeof(calc_display)-1); } else res = calc_pending / cur; } if (!calc_error) snprintf(calc_display, sizeof(calc_display), "%g", res); calc_pending = res; } else { calc_pending = cur; } calc_op = lab[0]; calc_entering = 0; }
                        else if (strcmp(lab, ".") == 0) { if (!strchr(calc_display, '.')) { if (!calc_entering) { strncpy(calc_display, "0", sizeof(calc_display)-1); calc_entering = 1; } strncat(calc_display, ".", sizeof(calc_display)-strlen(calc_display)-1); } }
                        else { if (!calc_entering || (strcmp(calc_display, "0") == 0 && strchr("0123456789", lab[0]))) { snprintf(calc_display, sizeof(calc_display), "%s", lab); calc_entering = 1; } else { strncat(calc_display, lab, sizeof(calc_display)-strlen(calc_display)-1); } }
                    }
                }
            }
        }

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r*cols + c;
                Rectangle br = { bx0 + c*(btnW+gap), by0 + r*(btnH+gap), btnW, btnH };
                int hov = CheckCollisionPointRec(mouse, br);
                DrawRectangleRec(br, hov ? (Color){60,80,140,255} : (Color){220,220,230,255});
                DrawRectangleLinesEx(br, 1, (Color){80,80,100,255});
                Vector2 lsz = MeasureTextEx(guiFont, b_labels[idx], 18.0f, 0.0f);
                DrawTextEx(guiFont, b_labels[idx], (Vector2){ br.x + (br.width - lsz.x)/2, br.y + (br.height - lsz.y)/2 }, 18.0f, 0.0f, (Color){30,30,40,255});
            }
        }

        // Window interactions (close/drag/resize)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 _m = GetMousePosition();
            Rectangle titleRect = {(float)xcalcWin->x + xb, (float)xcalcWin->y + xb, (float)xcalcWin->width - 2*xb, (float)xTitleH};
            if (CheckCollisionPointRec(_m, (Rectangle){(float)xCloseX,(float)xCloseY,(float)xCloseSz,(float)xCloseSz})) {
                *xcalc_open = 0; if (*last_clicked == WIN_XCALC) *last_clicked = -1;
            } else if (CheckCollisionPointRec(_m, titleRect)) {
                xcalc_dragging = 1; xcalcDragOffset.x = _m.x - xcalcWin->x; xcalcDragOffset.y = _m.y - xcalcWin->y; *last_clicked = WIN_XCALC;
            } else {
                Rectangle xResize = { xcalcWin->x + xcalcWin->width - 18, xcalcWin->y + xcalcWin->height - 18, 18, 18 };
                if (CheckCollisionPointRec(_m, xResize)) { *xcalc_resizing = 1; *last_clicked = WIN_XCALC; *focused_window = WIN_XCALC; }
            }
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) { xcalc_dragging = 0; *xcalc_resizing = 0; }
    if (xcalc_dragging && *last_clicked == WIN_XCALC) { Vector2 _m = GetMousePosition(); xcalcWin->x = _m.x - xcalcDragOffset.x; xcalcWin->y = _m.y - xcalcDragOffset.y; }
        if (*xcalc_resizing) { Vector2 rm = GetMousePosition(); float newW = rm.x - xcalcWin->x; if (newW < 220) newW = 220; if (newW > screenWidth - xcalcWin->x) newW = screenWidth - xcalcWin->x - 8; float newH = rm.y - xcalcWin->y; if (newH < 240) newH = 240; if (newH > screenHeight - xcalcWin->y) newH = screenHeight - xcalcWin->y - 8; xcalcWin->width = newW; xcalcWin->height = newH; }
    }

    // Minimized icon
    if (*xcalc_minimized) {
        int icon_x = 32, icon_y = (int)(48 + 32); // approximate positions used in main
        int minIconX = icon_x + (int)(32 * 0.18f) + 24;
        int minIconY = icon_y;
        DrawTextureEx(xcalclogo, (Vector2){(float)minIconX, (float)minIconY}, 0.0f, 0.18f, (Color){255,255,255,255});
        DrawTextEx(guiFont, "xcalc", (Vector2){(float)minIconX, (float)(minIconY + (int)(xcalclogo.height*0.18f) + 4)}, 14.0f, 0.0f, (Color){30,30,40,255});
    }
}

int xcalc(void) {
    const int screenW = 480;
    const int screenH = 640;
    InitWindow(screenW, screenH, "xcalc - standalone");
    SetTargetFPS(60);

    Font guiFont = LoadFontEx("C:\\Windows\\Fonts\\arial.ttf", 24, NULL, 0);
    Image img = GenImageColor(128,128, (Color){200,200,200,255});
    Texture2D xcalclogo = LoadTextureFromImage(img);
    UnloadImage(img);

    Rectangle xcalcWin = { 40, 40, 400, 480 };
    int xcalc_open = 1;
    int xcalc_minimized = 0;
    int xcalc_resizing = 0;
    int xcalc_close_hover = 0;
    int xcalc_workspace = 1;
    int xcalc_sticky = 0;
    int last_clicked = -1;
    int focused_window = -1;
    int workspace = 1;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){100,100,120,255});
        draw_xcalc_widget(&xcalcWin, &xcalc_open, &xcalc_minimized, &xcalc_resizing, &xcalc_close_hover,
                          &xcalc_workspace, &xcalc_sticky, guiFont, xcalclogo,
                          &last_clicked, 0, &focused_window, workspace, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
    }

    UnloadTexture(xcalclogo);
    UnloadFont(guiFont);
    CloseWindow();
    return 0;
}
