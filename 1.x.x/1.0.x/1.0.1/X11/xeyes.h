#ifndef XEYES_H
#define XEYES_H

#include "raylib.h"

// XEyes function declaration
void draw_xeyes_window(Rectangle *xeyesWin, int *xeyes_open, int *xeyes_minimized, 
                      int *xeyes_dragging, Vector2 *xeyes_drag_offset,
                      int *last_clicked, int WIN_XEYES, Font guiFont,
                      int screenWidth, int screenHeight);

#endif // XEYES_H