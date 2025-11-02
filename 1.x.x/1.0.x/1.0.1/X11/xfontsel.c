#include "raylib.h"
#include "xfontsel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FONTS 50
#define FONT_NAME_LENGTH 64

typedef struct {
    char name[FONT_NAME_LENGTH];
    Font font;
    bool loaded;
} FontInfo;

// Static state for xfontsel window
static FontInfo fonts[MAX_FONTS];
static int fontCount = 0;
static int selectedFont = 0;
static int scrollOffset = 0;
static int fontSize = 20;
static bool initialized = false;

void xfontsel_init(void) {
    if (initialized) return;
    
    // Common Windows font names with their typical paths
    const char* fontNames[] = {
        "Arial",
        "Courier New",
        "Times New Roman",
        "Verdana",
        "Georgia",
        "Comic Sans MS",
        "Trebuchet MS",
        "Impact",
        "Lucida Console",
        "Tahoma"
    };
    
    const char* fontFiles[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "C:/Windows/Fonts/times.ttf",
        "C:/Windows/Fonts/verdana.ttf",
        "C:/Windows/Fonts/georgia.ttf",
        "C:/Windows/Fonts/comic.ttf",
        "C:/Windows/Fonts/trebuc.ttf",
        "C:/Windows/Fonts/impact.ttf",
        "C:/Windows/Fonts/lucon.ttf",
        "C:/Windows/Fonts/tahoma.ttf"
    };
    
    fontCount = sizeof(fontNames) / sizeof(fontNames[0]);
    
    // Load fonts from Windows fonts directory
    for (int i = 0; i < fontCount && i < MAX_FONTS; i++) {
        strncpy(fonts[i].name, fontNames[i], FONT_NAME_LENGTH - 1);
        fonts[i].font = LoadFont(fontFiles[i]);
        
        // Check if font loaded successfully (texture.id will be > 0 if loaded)
        if (fonts[i].font.texture.id > 0) {
            fonts[i].loaded = true;
            printf("Loaded font: %s\n", fontNames[i]);
        } else {
            // If font failed to load, use default
            fonts[i].font = GetFontDefault();
            fonts[i].loaded = false;
            printf("Failed to load font: %s, using default\n", fontNames[i]);
        }
    }
    
    initialized = true;
}

void xfontsel_draw(Rectangle win, Font guiFont, Color fwvm_white, Color fwvm_black, Color fwvm_gray) {
    if (!initialized) xfontsel_init();
    
    const int itemHeight = 40;
    const int visibleItems = (int)((win.height - 150) / itemHeight);
    
    Color bgColor = (Color){45, 45, 48, 255};
    Color panelColor = (Color){37, 37, 38, 255};
    Color selectedColor = (Color){0, 122, 204, 255};
    Color textColor = (Color){241, 241, 241, 255};
    Color borderColor = (Color){63, 63, 70, 255};
    Font previewFont = fonts[selectedFont].font;
    
    // Handle input when window is focused
    int mouseWheel = (int)GetMouseWheelMove();
    if (mouseWheel != 0) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, win)) {
            scrollOffset -= mouseWheel;
            if (scrollOffset < 0) scrollOffset = 0;
            if (scrollOffset > fontCount - visibleItems && fontCount > visibleItems) {
                scrollOffset = fontCount - visibleItems;
            }
        }
    }
    
    // Handle font selection clicks
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        Rectangle listArea = {win.x + 10, win.y + 80, 300, win.height - 150};
        if (CheckCollisionPointRec(mousePos, listArea)) {
            int clickedItem = (int)((mousePos.y - listArea.y) / itemHeight) + scrollOffset;
            if (clickedItem >= 0 && clickedItem < fontCount) {
                selectedFont = clickedItem;
            }
        }
    }
    
    // Font size controls (only when window is active)
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, win)) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_KP_ADD)) {
            fontSize++;
            if (fontSize > 72) fontSize = 72;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            fontSize--;
            if (fontSize < 8) fontSize = 8;
        }
    }
    
    // Draw background
    DrawRectangleRec(win, bgColor);
    
    // Title bar (handled by x11.c, so we start content below it)
    
    // Font list panel
    DrawRectangle((int)win.x + 10, (int)win.y + 40, 300, (int)win.height - 50, panelColor);
    DrawTextEx(guiFont, "Available Fonts:", (Vector2){win.x + 20, win.y + 45}, 16, 0.0f, textColor);
    
    // Draw font list with scrolling
    int startY = (int)win.y + 70;
    for (int i = scrollOffset; i < fontCount && i < scrollOffset + visibleItems; i++) {
        int yPos = startY + (i - scrollOffset) * itemHeight;
        
        if (i == selectedFont) {
            DrawRectangle((int)win.x + 15, yPos + 5, 290, itemHeight - 10, selectedColor);
        }
        
        DrawTextEx(guiFont, fonts[i].name, (Vector2){win.x + 25, (float)(yPos + 12)}, 18, 0.0f, textColor);
        
        if (i < fontCount - 1) {
            DrawLine((int)win.x + 15, yPos + itemHeight, (int)win.x + 305, yPos + itemHeight, borderColor);
        }
    }
    
    // Preview panel
    DrawRectangle((int)win.x + 320, (int)win.y + 40, (int)win.width - 330, (int)win.height - 50, panelColor);
    DrawTextEx(guiFont, "Preview:", (Vector2){win.x + 330, win.y + 45}, 16, 0.0f, textColor);
    
    // Font name display
    DrawTextEx(guiFont, "Selected Font:", (Vector2){win.x + 330, win.y + 75}, 14, 0.0f, (Color){180, 180, 180, 255});
    DrawTextEx(guiFont, fonts[selectedFont].name, (Vector2){win.x + 330, win.y + 95}, 18, 0.0f, textColor);
    
    // Font size control
    DrawTextEx(guiFont, TextFormat("Size: %d pt", fontSize), (Vector2){win.x + 330, win.y + 125}, 14, 0.0f, (Color){180, 180, 180, 255});
    DrawTextEx(guiFont, "(Use UP/DOWN or +/- to adjust)", (Vector2){win.x + 330, win.y + 145}, 12, 0.0f, (Color){150, 150, 150, 255});
    
    // Sample text preview
    DrawRectangle((int)win.x + 330, (int)win.y + 175, (int)win.width - 340, (int)win.height - 185, bgColor);
    const char* sampleText = "The quick brown fox jumps over the lazy dog";
    DrawTextEx(previewFont, sampleText, (Vector2){win.x + 340, win.y + 185}, (float)fontSize, 0.0f, textColor);
    
    // ABC preview
    if (win.height > 350) {
        DrawTextEx(previewFont, "ABC", (Vector2){win.x + 340, win.y + 245}, (float)(fontSize * 2), 0.0f, textColor);
        DrawTextEx(previewFont, "abc", (Vector2){win.x + 340, win.y + 245 + fontSize * 2 + 10}, (float)(fontSize * 2), 0.0f, textColor);
        DrawTextEx(previewFont, "123", (Vector2){win.x + 340, win.y + 245 + fontSize * 4 + 20}, (float)(fontSize * 2), 0.0f, textColor);
    }
}
