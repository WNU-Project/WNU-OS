#include <stdio.h>
#include <math.h>
#include "raylib.h"
#include "xeyes.h"

// Classic xeyes implementation with FWVM 3.x styling
void draw_xeyes_window(Rectangle *xeyesWin, int *xeyes_open, int *xeyes_minimized, 
                      int *xeyes_dragging, Vector2 *xeyes_drag_offset,
                      int *last_clicked, int WIN_XEYES, Font guiFont,
                      int screenWidth, int screenHeight) {
    
    if (!xeyesWin || !xeyes_open) return;
    
    // FWVM 3.x Color Scheme
    Color fwvm_bg = {45, 45, 48, 255};
    Color fwvm_white = {255, 255, 255, 255};
    Color fwvm_light_gray = {240, 240, 240, 255};
    Color fwvm_dark_gray = {32, 32, 32, 255};
    Color fwvm_taskbar = {25, 25, 25, 255};
    Color fwvm_border = {76, 76, 76, 255};
    Color fwvm_accent = {0, 120, 215, 255};
    Color eye_bg = {248, 248, 255, 255};      // Very light blue for eye background
    Color pupil_color = {20, 20, 20, 255};    // Dark pupil
    Color iris_color = {70, 130, 180, 255};   // Steel blue iris
    
    Vector2 mouse = GetMousePosition();
    
    if (*xeyes_open && !*xeyes_minimized) {
        // Set minimum size for xeyes window
        if (xeyesWin->width < 200) xeyesWin->width = 200;
        if (xeyesWin->height < 120) xeyesWin->height = 120;
        
        int border = 1;
        int titleH = 32;
        
        // Modern flat window frame with shadow
        DrawRectangle((int)xeyesWin->x + 2, (int)xeyesWin->y + 2, (int)xeyesWin->width, (int)xeyesWin->height, (Color){0, 0, 0, 60}); // Shadow
        DrawRectangleRec(*xeyesWin, fwvm_border);
        DrawRectangle((int)xeyesWin->x + border, (int)xeyesWin->y + border, 
                     (int)xeyesWin->width - 2*border, (int)xeyesWin->height - 2*border, fwvm_light_gray);
        
        // Modern flat title bar
        DrawRectangle((int)xeyesWin->x + border, (int)xeyesWin->y + border, 
                     (int)xeyesWin->width - 2*border, titleH, fwvm_taskbar);
        DrawTextEx(guiFont, "xeyes", (Vector2){xeyesWin->x + border + 12, xeyesWin->y + border + 8}, 
                  16, 0.0f, fwvm_white);
        
        // Modern flat close button
        int closeBtnSz = titleH - 8;
        int closeBtnX = (int)(xeyesWin->x + xeyesWin->width - closeBtnSz - border - 8);
        int closeBtnY = (int)(xeyesWin->y + border + 4);
        DrawRectangle(closeBtnX, closeBtnY, closeBtnSz, closeBtnSz, (Color){232, 17, 35, 255}); // Windows red
        DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 
                  14, 0.0f, fwvm_white);
        
        // Eyes area - calculate eye positions and sizes
        float eyeAreaY = xeyesWin->y + titleH + border + 10;
        float eyeAreaH = xeyesWin->height - titleH - border - 20;
        float eyeAreaW = xeyesWin->width - 2*border - 20;
        
        // Calculate eye dimensions (two eyes side by side)
        float eyeRadius = fminf(eyeAreaH * 0.35f, eyeAreaW * 0.2f);
        if (eyeRadius < 15) eyeRadius = 15; // Minimum eye size
        
        // Eye positions
        float leftEyeX = xeyesWin->x + (xeyesWin->width * 0.3f);
        float rightEyeX = xeyesWin->x + (xeyesWin->width * 0.7f);
        float eyeY = eyeAreaY + (eyeAreaH * 0.5f);
        
        // Draw left eye
        DrawCircle((int)leftEyeX, (int)eyeY, eyeRadius, eye_bg);
        DrawCircleLines((int)leftEyeX, (int)eyeY, eyeRadius, fwvm_dark_gray);
        
        // Calculate pupil position for left eye (following mouse)
        float leftDx = mouse.x - leftEyeX;
        float leftDy = mouse.y - eyeY;
        float leftDist = sqrtf(leftDx*leftDx + leftDy*leftDy);
        float maxPupilDist = eyeRadius * 0.6f; // Pupil can move within 60% of eye radius
        
        float leftPupilX = leftEyeX;
        float leftPupilY = eyeY;
        if (leftDist > 0) {
            float factor = fminf(maxPupilDist / leftDist, 1.0f);
            leftPupilX += leftDx * factor;
            leftPupilY += leftDy * factor;
        }
        
        // Draw left pupil and iris
        float irisRadius = eyeRadius * 0.4f;
        float pupilRadius = eyeRadius * 0.25f;
        DrawCircle((int)leftPupilX, (int)leftPupilY, irisRadius, iris_color);
        DrawCircle((int)leftPupilX, (int)leftPupilY, pupilRadius, pupil_color);
        
        // Add highlight to left eye
        DrawCircle((int)(leftPupilX - pupilRadius*0.3f), (int)(leftPupilY - pupilRadius*0.3f), pupilRadius*0.2f, fwvm_white);
        
        // Draw right eye
        DrawCircle((int)rightEyeX, (int)eyeY, eyeRadius, eye_bg);
        DrawCircleLines((int)rightEyeX, (int)eyeY, eyeRadius, fwvm_dark_gray);
        
        // Calculate pupil position for right eye (following mouse)
        float rightDx = mouse.x - rightEyeX;
        float rightDy = mouse.y - eyeY;
        float rightDist = sqrtf(rightDx*rightDx + rightDy*rightDy);
        
        float rightPupilX = rightEyeX;
        float rightPupilY = eyeY;
        if (rightDist > 0) {
            float factor = fminf(maxPupilDist / rightDist, 1.0f);
            rightPupilX += rightDx * factor;
            rightPupilY += rightDy * factor;
        }
        
        // Draw right pupil and iris
        DrawCircle((int)rightPupilX, (int)rightPupilY, irisRadius, iris_color);
        DrawCircle((int)rightPupilX, (int)rightPupilY, pupilRadius, pupil_color);
        
        // Add highlight to right eye
        DrawCircle((int)(rightPupilX - pupilRadius*0.3f), (int)(rightPupilY - pupilRadius*0.3f), pupilRadius*0.2f, fwvm_white);
        
        // Handle close button
        Rectangle closeBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, closeBtnRect)) {
            *xeyes_open = 0;
            if (*last_clicked == WIN_XEYES) *last_clicked = -1;
        }
        
        // Handle window dragging
        Rectangle titleBarRect = {xeyesWin->x + border, xeyesWin->y + border, xeyesWin->width - 2*border - closeBtnSz - 8, titleH};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, titleBarRect)) {
            *xeyes_dragging = 1;
            xeyes_drag_offset->x = mouse.x - xeyesWin->x;
            xeyes_drag_offset->y = mouse.y - xeyesWin->y;
            *last_clicked = WIN_XEYES;
        }
        
        // Handle drag movement
        if (*xeyes_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            xeyesWin->x = mousePos.x - xeyes_drag_offset->x;
            xeyesWin->y = mousePos.y - xeyes_drag_offset->y;
            
            // Keep window on screen
            if (xeyesWin->x < 0) xeyesWin->x = 0;
            if (xeyesWin->y < 48) xeyesWin->y = 48; // Below top bar
            if (xeyesWin->x + xeyesWin->width > screenWidth) xeyesWin->x = screenWidth - xeyesWin->width;
            if (xeyesWin->y + xeyesWin->height > screenHeight - 48) xeyesWin->y = screenHeight - 48 - xeyesWin->height; // Above taskbar
        }
        
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *xeyes_dragging = 0;
        }
    }
}