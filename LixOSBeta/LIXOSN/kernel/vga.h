#ifndef VGA_H
#define VGA_H

#include <dos.h>
#include <string.h>
#include <sys/nearptr.h>

/* VGA Mode 13h: 320x200, 256 colors */
#define SCREEN_W     320
#define SCREEN_H     200
#define VGA_MEM      ((unsigned char *)(__djgpp_conventional_base + 0xA0000))

/* Windows 3.1 style color palette indices */
#define COL_BLACK        0
#define COL_DARK_BLUE    1
#define COL_DARK_GREEN   2
#define COL_DARK_CYAN    3
#define COL_DARK_RED     4
#define COL_DARK_MAGENTA 5
#define COL_BROWN        6
#define COL_LIGHT_GRAY   7
#define COL_DARK_GRAY    8
#define COL_BLUE         9
#define COL_GREEN        10
#define COL_CYAN         11
#define COL_RED          12
#define COL_MAGENTA      13
#define COL_YELLOW       14
#define COL_WHITE        15

/* Extended palette for GUI */
#define COL_WIN_BLUE     16   /* Window title bar */
#define COL_WIN_GRAY     17   /* Window background */
#define COL_WIN_DGRAY    18   /* Window border dark */
#define COL_WIN_LGRAY    19   /* Window border light */
#define COL_DESKTOP      20   /* Desktop teal color */
#define COL_BTN_FACE     21   /* Button face */
#define COL_BTN_SHADOW   22   /* Button shadow */
#define COL_BTN_HILITE   23   /* Button highlight */
#define COL_TITLE_TEXT   24   /* Title bar text */
#define COL_MENU_BG      25   /* Menu background */
#define COL_NOTEPAD_BG   26   /* Notepad blue background */
#define COL_NOTEPAD_TEXT 27   /* Notepad text */

/* Double buffer */
extern unsigned char vga_buffer[SCREEN_H][SCREEN_W];

void vga_init(void);
void vga_setmode(int mode);
void vga_setpalette(void);
void vga_flip(void);
void vga_clear(unsigned char color);
void vga_putpixel(int x, int y, unsigned char color);
void vga_hline(int x1, int x2, int y, unsigned char color);
void vga_vline(int x, int y1, int y2, unsigned char color);
void vga_rect(int x, int y, int w, int h, unsigned char color);
void vga_fillrect(int x, int y, int w, int h, unsigned char color);
void vga_putchar(int x, int y, char c, unsigned char fg, unsigned char bg);
void vga_putstr(int x, int y, const char *s, unsigned char fg, unsigned char bg);
int  vga_strwidth(const char *s);

#endif
