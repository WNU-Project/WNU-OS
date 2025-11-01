#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "x11.h"

// Draw the classic X Window System logo window
void draw_xlogo_window(Rectangle *xlogoWin, int *xlogo_open, int *xlogo_minimized, 
                       int *xlogo_dragging, Vector2 *xlogo_drag_offset, 
                       int *focused_window, Font guiFont, Texture2D x11Logo,
                       Color fwvm_bg, Color fwvm_white, Color fwvm_dark_gray, 
                       Color fwvm_accent, int WIN_XLOGO, int *last_clicked, 
                       int screenWidth, int screenHeight) {
    if (!*xlogo_open || *xlogo_minimized) return;

    Vector2 mousePos = GetMousePosition();
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool mouseReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    // Handle window dragging
    Rectangle titleBar = {xlogoWin->x, xlogoWin->y, xlogoWin->width, 30};
    if (mousePressed && CheckCollisionPointRec(mousePos, titleBar)) {
        *xlogo_dragging = 1;
        *focused_window = WIN_XLOGO;
        *last_clicked = WIN_XLOGO;
        xlogo_drag_offset->x = mousePos.x - xlogoWin->x;
        xlogo_drag_offset->y = mousePos.y - xlogoWin->y;
    }

    if (*xlogo_dragging && mouseDown) {
        xlogoWin->x = mousePos.x - xlogo_drag_offset->x;
        xlogoWin->y = mousePos.y - xlogo_drag_offset->y;
        
        // Boundary constraints
        if (xlogoWin->x < 0) xlogoWin->x = 0;
        if (xlogoWin->y < 0) xlogoWin->y = 0;
        if (xlogoWin->x + xlogoWin->width > screenWidth) xlogoWin->x = screenWidth - xlogoWin->width;
        if (xlogoWin->y + xlogoWin->height > screenHeight) xlogoWin->y = screenHeight - xlogoWin->height;
    }

    if (mouseReleased) {
        *xlogo_dragging = 0;
    }

    // Draw main window
    DrawRectangleRec(*xlogoWin, fwvm_bg);
    DrawRectangleLinesEx(*xlogoWin, 1, fwvm_dark_gray);

    // Draw title bar
    DrawRectangle((int)xlogoWin->x, (int)xlogoWin->y, (int)xlogoWin->width, 30, fwvm_dark_gray);
    DrawTextEx(guiFont, "X Logo", (Vector2){xlogoWin->x + 8, xlogoWin->y + 8}, 14, 0.0f, fwvm_white);

    // Close button
    Rectangle closeBtn = {xlogoWin->x + xlogoWin->width - 25, xlogoWin->y + 5, 20, 20};
    Color closeBtnColor = CheckCollisionPointRec(mousePos, closeBtn) ? 
                         (Color){220, 50, 50, 255} : fwvm_accent;
    DrawRectangleRec(closeBtn, closeBtnColor);
    DrawTextEx(guiFont, "×", (Vector2){closeBtn.x + 6, closeBtn.y + 2}, 14, 0.0f, fwvm_white);

    if (mousePressed && CheckCollisionPointRec(mousePos, closeBtn)) {
        *xlogo_open = 0;
        return;
    }

    // Display the X11 logo (passed as parameter)
    // Calculate center position for the logo
    float centerX = xlogoWin->x + xlogoWin->width / 2;
    float centerY = xlogoWin->y + 60 + (xlogoWin->height - 90) / 2;
    
    // Calculate logo scale to fit nicely in the window
    float logoScale = 0.8f; // Adjust this to make logo bigger/smaller
    float logoWidth = x11Logo.width * logoScale;
    float logoHeight = x11Logo.height * logoScale;
    
    // Draw background rectangle for the logo
    float bgPadding = 20.0f;
    Rectangle logoBg = {
        centerX - logoWidth/2 - bgPadding/2,
        centerY - logoHeight/2 - bgPadding/2,
        logoWidth + bgPadding,
        logoHeight + bgPadding
    };
    DrawRectangleRec(logoBg, fwvm_dark_gray);
    DrawRectangleRec((Rectangle){logoBg.x + 2, logoBg.y + 2, logoBg.width - 4, logoBg.height - 4}, fwvm_bg);
    
    // Draw the X11 logo centered in the window
    Vector2 logoPos = {centerX - logoWidth/2, centerY - logoHeight/2};
    DrawTextureEx(x11Logo, logoPos, 0.0f, logoScale, fwvm_white);

    // Draw version text at the bottom
    const char* versionText = "X Window System";
    Vector2 textSize = MeasureTextEx(guiFont, versionText, 12, 0.0f);
    DrawTextEx(guiFont, versionText, 
               (Vector2){centerX - textSize.x/2, xlogoWin->y + xlogoWin->height - 25}, 
               12, 0.0f, fwvm_white);

    // Add copyright info
    const char* copyrightText = "© 1984-2024 X Consortium";
    Vector2 copyrightSize = MeasureTextEx(guiFont, copyrightText, 10, 0.0f);
    DrawTextEx(guiFont, copyrightText,
               (Vector2){centerX - copyrightSize.x/2, xlogoWin->y + xlogoWin->height - 12},
               10, 0.0f, fwvm_dark_gray);
}
