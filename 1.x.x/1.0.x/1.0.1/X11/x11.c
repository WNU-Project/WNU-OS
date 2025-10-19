#include "raylib.h"
#include "x11_logo.h"   // generated from X11.png
#include <stdio.h>
#include <string.h>

int x11() {
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "WNU OS X11");
    SetTargetFPS(60);

    // Load embedded logo
    Image logoImage = LoadImageFromMemory(".png", X11_png, X11_png_len);
    Texture2D icon = LoadTextureFromImage(logoImage);
    UnloadImage(logoImage);

    int icon_x = 32;
    int icon_y = 80;
    int icon_w = icon.width/2;
    int icon_h = icon.height/2;
    int terminal_launched = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, screenWidth, 32, DARKGRAY);
        DrawTextEx(GetFontDefault(), "WNU OS X11 GUI", (Vector2){10, 8}, 16, 1, RAYWHITE);

        DrawRectangle(0, 32, screenWidth, screenHeight - 32, LIGHTGRAY);
        DrawText("Welcome to WNU OS X11 GUI (raylib)", 40, 60, 20, DARKGRAY);

        // Draw terminal icon
        DrawTextureEx(icon, (Vector2){icon_x, icon_y}, 0.0f, 0.5f, WHITE);
        DrawText("Terminal", icon_x, icon_y + icon_h + 2, 12, DARKGRAY);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            if (mouse.x >= icon_x && mouse.x <= icon_x + icon_w &&
                mouse.y >= icon_y && mouse.y <= icon_y + icon_h) {
                terminal_launched = 1;
            }
        }

        if (terminal_launched) {
            DrawRectangle(300, 250, 250, 120, DARKGRAY);
            DrawText("[Terminal]", 340, 300, 18, RAYWHITE);
        }

        EndDrawing();
    }

    UnloadTexture(icon);
    CloseWindow();
    return 0;
}
