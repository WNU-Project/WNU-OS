#ifndef XCALC_H
#define XCALC_H

#include "raylib.h" /* Rectangle, Font, Texture2D, Vector2, etc. */

int xcalc(void);

/* Prototype for the xcalc widget. The implementation in X11/x11.c
    defines this function as static; to keep the header safe to include
    across translation units, the prototype here is also declared static
    (so it matches the definition and avoids linkage conflicts). */
static void draw_xcalc_widget(Rectangle *xcalcWin, int *xcalc_open, int *xcalc_minimized, int *xcalc_resizing, int *xcalc_close_hover,
                                      int *xcalc_workspace, int *xcalc_sticky, Font guiFont, Texture2D xcalclogo,
                                      int *last_clicked, int WIN_XCALC,
                                      int *focused_window, int workspace, int screenWidth, int screenHeight);

#endif // XCALC_H