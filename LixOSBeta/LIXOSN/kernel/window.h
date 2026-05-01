#ifndef WINDOW_H
#define WINDOW_H

#include "vga.h"
#include "mouse.h"

#define MAX_WINDOWS  8
#define TITLE_H      12   /* title bar height in pixels */
#define BORDER       2

typedef struct {
    int  id;
    int  x, y, w, h;
    char title[32];
    int  visible;
    int  focused;
    int  dragging;
    int  drag_ox, drag_oy;  /* offset when drag started */
} Window;

/* Shared window array */
extern Window windows[MAX_WINDOWS];
extern int    wm_focused;   /* index of focused window */

/* Window manager */
void wm_init(void);
int  wm_open(int x, int y, int w, int h, const char *title);
void wm_close(int id);
void wm_draw_all(void);          /* draws all windows (no content) */
void wm_draw_frame(int id);      /* draws one frame */
void wm_process_mouse(void);     /* dragging, focus, close */
int  wm_close_clicked(int id);   /* returns 1 if X button was clicked */

/* Draw a Win3.1 style button */
void gui_button(int x, int y, int w, int h,
                const char *label, int pressed);

/* Returns 1 if point (mx,my) is inside rect */
int gui_inside(int mx, int my, int x, int y, int w, int h);

/* Taskbar at bottom */
void gui_taskbar(const char *title);

#endif
