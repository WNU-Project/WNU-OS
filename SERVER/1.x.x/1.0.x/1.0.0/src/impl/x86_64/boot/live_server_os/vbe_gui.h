#ifndef VBE_GUI_H
#define VBE_GUI_H

// Color constants for 8-bit VGA palette
#define VBE_COLOR_BLACK_8BIT         0
#define VBE_COLOR_BLUE_8BIT          1
#define VBE_COLOR_GREEN_8BIT         2
#define VBE_COLOR_CYAN_8BIT          3
#define VBE_COLOR_RED_8BIT           4
#define VBE_COLOR_MAGENTA_8BIT       5
#define VBE_COLOR_BROWN_8BIT         6
#define VBE_COLOR_WHITE_8BIT         7
#define VBE_COLOR_GRAY_8BIT          8
#define VBE_COLOR_LIGHT_BLUE_8BIT    9
#define VBE_COLOR_LIGHT_GREEN_8BIT   10
#define VBE_COLOR_LIGHT_CYAN_8BIT    11
#define VBE_COLOR_LIGHT_RED_8BIT     12
#define VBE_COLOR_LIGHT_MAGENTA_8BIT 13
#define VBE_COLOR_YELLOW_8BIT        14
#define VBE_COLOR_BRIGHT_WHITE_8BIT  15

// Legacy 32-bit color constants (converted to 8-bit equivalents)
#define VBE_COLOR_BLACK         VBE_COLOR_BLACK_8BIT
#define VBE_COLOR_WHITE         VBE_COLOR_WHITE_8BIT
#define VBE_COLOR_RED           VBE_COLOR_RED_8BIT
#define VBE_COLOR_GREEN         VBE_COLOR_GREEN_8BIT
#define VBE_COLOR_BLUE          VBE_COLOR_BLUE_8BIT
#define VBE_COLOR_YELLOW        VBE_COLOR_YELLOW_8BIT
#define VBE_COLOR_CYAN          VBE_COLOR_CYAN_8BIT
#define VBE_COLOR_MAGENTA       VBE_COLOR_MAGENTA_8BIT
#define VBE_COLOR_GRAY          VBE_COLOR_GRAY_8BIT
#define VBE_COLOR_LIGHT_GRAY    VBE_COLOR_BRIGHT_WHITE_8BIT
#define VBE_COLOR_DARK_GRAY     VBE_COLOR_GRAY_8BIT
#define VBE_COLOR_DARK_RED      VBE_COLOR_RED_8BIT
#define VBE_COLOR_DARK_GREEN    VBE_COLOR_GREEN_8BIT
#define VBE_COLOR_DARK_BLUE     VBE_COLOR_BLUE_8BIT
#define VBE_COLOR_DARK_CYAN     VBE_COLOR_CYAN_8BIT
#define VBE_COLOR_BROWN         VBE_COLOR_BROWN_8BIT
#define VBE_COLOR_ORANGE        VBE_COLOR_YELLOW_8BIT

// Function declarations
int vbe_get_controller_info(void);
int vbe_get_mode_info(unsigned short mode);
unsigned short vbe_find_mode(unsigned int width, unsigned int height, unsigned int depth);
int vbe_set_mode(unsigned short mode);
void vbe_set_pixel(int x, int y, unsigned int color);
unsigned int vbe_get_pixel(int x, int y);
void vbe_fill_rect(int x, int y, int width, int height, unsigned int color);
void vbe_draw_line(int x0, int y0, int x1, int y1, unsigned int color);
void vbe_draw_rect(int x, int y, int width, int height, unsigned int color);
void vbe_clear_screen(unsigned int color);
void vbe_draw_char(int x, int y, char c, unsigned int fg_color, unsigned int bg_color);
void vbe_draw_string(int x, int y, const char* str, unsigned int fg_color, unsigned int bg_color);
void save_cursor_background(void);
void restore_cursor_background(void);
void draw_cursor(void);
unsigned char read_keyboard(void);
void handle_keyboard(void);
void draw_window(int x, int y, int width, int height, const char* title);
void draw_desktop(void);
void start_vbe_gui(void);

#endif // VBE_GUI_H