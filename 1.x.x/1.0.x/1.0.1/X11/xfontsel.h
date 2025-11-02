#ifndef XFONTSEL_H
#define XFONTSEL_H

#include "raylib.h"

// Initialize xfontsel window
void xfontsel_init(void);

// Draw xfontsel window content
void xfontsel_draw(Rectangle win, Font guiFont, Color fwvm_white, Color fwvm_black, Color fwvm_gray);

#endif // XFONTSEL_H
