#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include "xpong.h"

// Game structures
typedef struct {
    Vector2 position;
    Vector2 size;
    float speed;
} Paddle;

typedef struct {
    Vector2 position;
    Vector2 speed;
    float radius;
} Ball;

// XPong window implementation - runs as a window within the desktop
int draw_xpong_window(Rectangle *xpongWin, int *xpong_open, int *xpong_minimized, 
                      int *xpong_dragging, Vector2 *xpong_drag_offset,
                      int *last_clicked, int WIN_XPONG, Font guiFont, 
                      int screenWidth, int screenHeight) {
    
    if (!xpong_open || !*xpong_open || *xpong_minimized) return 0;

    // Persistent game state (static variables to maintain state between calls)
    static Paddle leftPaddle = { {50, 250}, {15, 80}, 250 };
    static Paddle rightPaddle = { {735, 250}, {15, 80}, 250 };
    static Ball ball = { {400, 300}, {200, 150}, 8 };
    static int leftScore = 0;
    static int rightScore = 0;
    static bool gameStarted = false;
    static bool gameInitialized = false;
    static bool gameWon = false;
    static float winTimer = 0.0f;

    Vector2 mouse = GetMousePosition();
    int border = 2;
    int titleH = 32;

    // Initialize game when window first opens
    if (!gameInitialized) {
        // Reset game to initial state
        leftPaddle.position.x = 50;
        leftPaddle.position.y = xpongWin->height / 2 - 40;
        rightPaddle.position.x = xpongWin->width - 65;
        rightPaddle.position.y = xpongWin->height / 2 - 40;
        ball.position.x = xpongWin->width / 2;
        ball.position.y = xpongWin->height / 2;
        ball.speed.x = (GetRandomValue(0, 1) == 0) ? 200 : -200;
        ball.speed.y = GetRandomValue(-150, 150);
        leftScore = 0;
        rightScore = 0;
        gameStarted = false;
        gameInitialized = true;
        gameWon = false;
        winTimer = 0.0f;
    }

    // Ensure minimum window size
    if (xpongWin->width < 600) xpongWin->width = 600;
    if (xpongWin->height < 400) xpongWin->height = 400;

    // Draw window frame with shadow (FWVM 3.x style)
    Color fwvm_bg = {45, 45, 48, 255};
    Color fwvm_light_gray = {240, 240, 240, 255};
    Color fwvm_taskbar = {25, 25, 25, 255};
    Color fwvm_border = {76, 76, 76, 255};
    Color fwvm_white = {255, 255, 255, 255};

    DrawRectangle((int)xpongWin->x + 2, (int)xpongWin->y + 2, (int)xpongWin->width, (int)xpongWin->height, (Color){0,0,0,60}); // Shadow
    DrawRectangleRec(*xpongWin, fwvm_border);
    DrawRectangle((int)xpongWin->x + border, (int)xpongWin->y + border, 
                 (int)xpongWin->width - 2*border, (int)xpongWin->height - 2*border, (Color){12, 12, 12, 255}); // Game bg

    // Modern flat title bar
    DrawRectangle((int)xpongWin->x + border, (int)xpongWin->y + border, 
                 (int)xpongWin->width - 2*border, titleH, fwvm_taskbar);
    DrawTextEx(guiFont, "XPong - Classic Pong Game", 
              (Vector2){xpongWin->x + border + 12, xpongWin->y + border + 8}, 
              16, 0.0f, fwvm_white);

    // Modern flat close button
    int closeBtnSz = titleH - 8;
    int closeBtnX = (int)(xpongWin->x + xpongWin->width - closeBtnSz - border - 8);
    int closeBtnY = (int)(xpongWin->y + border + 4);
    Rectangle closeBtnRect = {(float)closeBtnX, (float)closeBtnY, (float)closeBtnSz, (float)closeBtnSz};
    
    int closeHover = CheckCollisionPointRec(mouse, closeBtnRect);
    DrawRectangleRec(closeBtnRect, closeHover ? (Color){232, 17, 35, 255} : (Color){63, 63, 70, 255});
    DrawTextEx(guiFont, "×", (Vector2){(float)(closeBtnX + 4), (float)(closeBtnY + 2)}, 14, 0.0f, fwvm_white);
    
    // Handle close button
    if (closeHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *xpong_open = 0;
        gameInitialized = false; // Reset for next time
        if (*last_clicked == WIN_XPONG) *last_clicked = -1;
        return 0;
    }

    // Game area (below title bar)
    float gameAreaX = xpongWin->x + border;
    float gameAreaY = xpongWin->y + border + titleH;
    float gameAreaW = xpongWin->width - 2*border;
    float gameAreaH = xpongWin->height - 2*border - titleH;

    // Check if mouse is in game area for input focus
    Rectangle gameArea = {gameAreaX, gameAreaY, gameAreaW, gameAreaH};
    bool gameHasFocus = CheckCollisionPointRec(mouse, gameArea);

    // Game logic update
    float deltaTime = GetFrameTime();
    
    // Check for win condition and handle win timer
    if (!gameWon && (leftScore >= 5 || rightScore >= 5)) {
        gameWon = true;
        winTimer = 0.0f;
    }
    
    if (gameWon) {
        winTimer += deltaTime;
        if (winTimer >= 3.0f) { // Show win message for 3 seconds, then close
            *xpong_open = 0;
            gameInitialized = false; // Reset for next time
            if (*last_clicked == WIN_XPONG) *last_clicked = -1;
            return 0;
        }
    }
    
    if (gameHasFocus && !gameWon) {
        if (!gameStarted) {
            // Start game when space is pressed (only if game window has focus)
            if (IsKeyPressed(KEY_SPACE)) {
                gameStarted = true;
                // Reset ball position relative to window
                ball.position.x = gameAreaW / 2;
                ball.position.y = gameAreaH / 2;
                ball.speed.x = (GetRandomValue(0, 1) == 0) ? 200 : -200;
                ball.speed.y = GetRandomValue(-150, 150);
            }
        } else {
            // Player controls (only when game has focus)
            // Player 1 controls (W/S keys)
            if (IsKeyDown(KEY_W) && leftPaddle.position.y > 0) {
                leftPaddle.position.y -= leftPaddle.speed * deltaTime;
            }
            if (IsKeyDown(KEY_S) && leftPaddle.position.y < gameAreaH - leftPaddle.size.y) {
                leftPaddle.position.y += leftPaddle.speed * deltaTime;
            }
            
            // Player 2 controls (UP/DOWN arrows)
            if (IsKeyDown(KEY_UP) && rightPaddle.position.y > 0) {
                rightPaddle.position.y -= rightPaddle.speed * deltaTime;
            }
            if (IsKeyDown(KEY_DOWN) && rightPaddle.position.y < gameAreaH - rightPaddle.size.y) {
                rightPaddle.position.y += rightPaddle.speed * deltaTime;
            }
            
            // Ball movement
            ball.position.x += ball.speed.x * deltaTime;
            ball.position.y += ball.speed.y * deltaTime;
            
            // Ball collision with top and bottom walls
            if (ball.position.y <= ball.radius || ball.position.y >= gameAreaH - ball.radius) {
                ball.speed.y *= -1;
            }
            
            // Ball collision with left paddle
            if (ball.position.x - ball.radius <= leftPaddle.position.x + leftPaddle.size.x &&
                ball.position.y >= leftPaddle.position.y &&
                ball.position.y <= leftPaddle.position.y + leftPaddle.size.y &&
                ball.speed.x < 0) {
                
                ball.speed.x *= -1;
                // Add some spin based on where ball hits paddle
                float hitPos = (ball.position.y - leftPaddle.position.y) / leftPaddle.size.y;
                ball.speed.y = (hitPos - 0.5f) * 300;
            }
            
            // Ball collision with right paddle
            if (ball.position.x + ball.radius >= rightPaddle.position.x &&
                ball.position.y >= rightPaddle.position.y &&
                ball.position.y <= rightPaddle.position.y + rightPaddle.size.y &&
                ball.speed.x > 0) {
                
                ball.speed.x *= -1;
                // Add some spin based on where ball hits paddle
                float hitPos = (ball.position.y - rightPaddle.position.y) / rightPaddle.size.y;
                ball.speed.y = (hitPos - 0.5f) * 300;
            }
            
            // Scoring
            if (ball.position.x < 0) {
                rightScore++;
                gameStarted = false; // Reset for next round
            } else if (ball.position.x > gameAreaW) {
                leftScore++;
                gameStarted = false; // Reset for next round
            }
        }
        
        // Reset game if R is pressed (only when focused)
        if (IsKeyPressed(KEY_R)) {
            leftScore = 0;
            rightScore = 0;
            gameStarted = false;
            gameWon = false;
            winTimer = 0.0f;
            ball.position.x = gameAreaW / 2;
            ball.position.y = gameAreaH / 2;
        }
    }

    // Draw game content (with offset for window position)
    
    // Draw game field
    DrawLine((int)(gameAreaX + gameAreaW/2), (int)gameAreaY, 
             (int)(gameAreaX + gameAreaW/2), (int)(gameAreaY + gameAreaH), (Color){100, 100, 100, 255});
    DrawCircleLines((int)(gameAreaX + gameAreaW/2), (int)(gameAreaY + gameAreaH/2), 40, (Color){100, 100, 100, 255});
    
    // Draw paddles (offset by game area position)
    DrawRectangleV((Vector2){gameAreaX + leftPaddle.position.x, gameAreaY + leftPaddle.position.y}, 
                   leftPaddle.size, (Color){255, 255, 255, 255});
    DrawRectangleV((Vector2){gameAreaX + rightPaddle.position.x, gameAreaY + rightPaddle.position.y}, 
                   rightPaddle.size, (Color){255, 255, 255, 255});
    
    // Draw ball (offset by game area position)
    if (gameStarted) {
        DrawCircleV((Vector2){gameAreaX + ball.position.x, gameAreaY + ball.position.y}, 
                   ball.radius, (Color){255, 255, 255, 255});
    }
    
    // Draw scores
    char leftScoreStr[8], rightScoreStr[8];
    sprintf(leftScoreStr, "%d", leftScore);
    sprintf(rightScoreStr, "%d", rightScore);
    
    DrawTextEx(guiFont, leftScoreStr, 
              (Vector2){gameAreaX + gameAreaW/4 - 20, gameAreaY + 30}, 
              48, 0.0f, (Color){255, 255, 255, 255});
    DrawTextEx(guiFont, rightScoreStr, 
              (Vector2){gameAreaX + 3*gameAreaW/4 - 20, gameAreaY + 30}, 
              48, 0.0f, (Color){255, 255, 255, 255});
    
    // Draw instructions
    if (!gameStarted) {
        Vector2 titleSize = MeasureTextEx(guiFont, "XPONG", 24, 0.0f);
        DrawTextEx(guiFont, "XPONG", 
                  (Vector2){gameAreaX + (gameAreaW - titleSize.x)/2, gameAreaY + gameAreaH/2 - 60}, 
                  24, 0.0f, (Color){255, 255, 255, 255});
        
        Vector2 startSize = MeasureTextEx(guiFont, "Press SPACE to start", 16, 0.0f);
        DrawTextEx(guiFont, "Press SPACE to start", 
                  (Vector2){gameAreaX + (gameAreaW - startSize.x)/2, gameAreaY + gameAreaH/2 - 30}, 
                  16, 0.0f, (Color){180, 180, 180, 255});
        
        DrawTextEx(guiFont, "Player 1: W/S", 
                  (Vector2){gameAreaX + 20, gameAreaY + gameAreaH - 60}, 
                  12, 0.0f, (Color){150, 150, 150, 255});
        DrawTextEx(guiFont, "Player 2: UP/DOWN", 
                  (Vector2){gameAreaX + gameAreaW - 140, gameAreaY + gameAreaH - 60}, 
                  12, 0.0f, (Color){150, 150, 150, 255});
        DrawTextEx(guiFont, "Press R to reset", 
                  (Vector2){gameAreaX + (gameAreaW - 100)/2, gameAreaY + gameAreaH - 30}, 
                  12, 0.0f, (Color){120, 120, 120, 255});
    }
    
    // Draw winning message (only when game is won)
    if (gameWon) {
        if (leftScore >= 5) {
            Vector2 winSize = MeasureTextEx(guiFont, "PLAYER 1 WINS!", 20, 0.0f);
            DrawTextEx(guiFont, "PLAYER 1 WINS!", 
                      (Vector2){gameAreaX + (gameAreaW - winSize.x)/2, gameAreaY + gameAreaH/2 + 40}, 
                      20, 0.0f, (Color){0, 255, 0, 255});
        } else if (rightScore >= 5) {
            Vector2 winSize = MeasureTextEx(guiFont, "PLAYER 2 WINS!", 20, 0.0f);
            DrawTextEx(guiFont, "PLAYER 2 WINS!", 
                      (Vector2){gameAreaX + (gameAreaW - winSize.x)/2, gameAreaY + gameAreaH/2 + 40}, 
                      20, 0.0f, (Color){0, 255, 0, 255});
        }
        
        // Show countdown
        char closeMsg[64];
        sprintf(closeMsg, "Closing in %.1f seconds...", 3.0f - winTimer);
        Vector2 closeMsgSize = MeasureTextEx(guiFont, closeMsg, 12, 0.0f);
        DrawTextEx(guiFont, closeMsg, 
                  (Vector2){gameAreaX + (gameAreaW - closeMsgSize.x)/2, gameAreaY + gameAreaH/2 + 70}, 
                  12, 0.0f, (Color){200, 200, 200, 255});
    }

    // Handle window dragging
    Rectangle titleBarRect = {xpongWin->x + border, xpongWin->y + border, 
                             xpongWin->width - 2*border - closeBtnSz - 8, (float)titleH};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, titleBarRect)) {
        *xpong_dragging = 1;
        xpong_drag_offset->x = mouse.x - xpongWin->x;
        xpong_drag_offset->y = mouse.y - xpongWin->y;
        *last_clicked = WIN_XPONG;
    }
    
    // Handle drag movement
    if (*xpong_dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        xpongWin->x = mousePos.x - xpong_drag_offset->x;
        xpongWin->y = mousePos.y - xpong_drag_offset->y;
        
        // Keep window on screen
        if (xpongWin->x < 0) xpongWin->x = 0;
        if (xpongWin->y < 48) xpongWin->y = 48; // Below top bar
        if (xpongWin->x + xpongWin->width > screenWidth) xpongWin->x = screenWidth - xpongWin->width;
        if (xpongWin->y + xpongWin->height > screenHeight - 48) xpongWin->y = screenHeight - 48 - xpongWin->height; // Above taskbar
    }
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        *xpong_dragging = 0;
    }
}