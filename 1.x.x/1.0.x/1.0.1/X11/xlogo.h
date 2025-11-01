#ifndef XLOGO_H
#define XLOGO_H

#include "raylib.h"

// Function to draw the XLogo window
void draw_xlogo_window(Rectangle *xlogoWin, int *xlogo_open, int *xlogo_minimized, 
                       int *xlogo_dragging, Vector2 *xlogo_drag_offset, 
                       int *focused_window, Font guiFont, Texture2D x11Logo,
                       Color fwvm_bg, Color fwvm_white, Color fwvm_dark_gray, 
                       Color fwvm_accent, int WIN_XLOGO, int *last_clicked, 
                       int screenWidth, int screenHeight);

#endif // XLOGO_H