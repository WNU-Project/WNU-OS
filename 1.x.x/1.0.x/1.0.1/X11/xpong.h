#ifndef XPONG_H
#define XPONG_H

#include "raylib.h"

// XPong game window structure and functions
int draw_xpong_window(Rectangle *xpongWin, int *xpong_open, int *xpong_minimized, 
                      int *xpong_dragging, Vector2 *xpong_drag_offset,
                      int *last_clicked, int WIN_XPONG, Font guiFont, 
                      int screenWidth, int screenHeight);

#endif // XPONG_H