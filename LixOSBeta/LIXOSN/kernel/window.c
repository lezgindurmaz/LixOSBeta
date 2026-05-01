/*=============================================================
  WINDOW.C  -  Windows 3.1 style window manager for LixOS
=============================================================*/
#include "window.h"
#include "vga.h"
#include "mouse.h"
#include <string.h>

Window windows[MAX_WINDOWS];
int    wm_focused = -1;

void wm_init(void) {
    int i;
    for(i=0; i<MAX_WINDOWS; i++) {
        windows[i].visible  = 0;
        windows[i].id       = i;
        windows[i].dragging = 0;
        windows[i].focused  = 0;
    }
    wm_focused = -1;
}

int wm_open(int x, int y, int w, int h, const char *title) {
    int i;
    for(i=0; i<MAX_WINDOWS; i++) {
        if(!windows[i].visible) {
            windows[i].x       = x;
            windows[i].y       = y;
            windows[i].w       = w;
            windows[i].h       = h;
            windows[i].visible = 1;
            windows[i].focused = 1;
            windows[i].dragging= 0;
            strncpy(windows[i].title, title, 31);
            windows[i].title[31]='\0';
            if(wm_focused >= 0)
                windows[wm_focused].focused = 0;
            wm_focused = i;
            return i;
        }
    }
    return -1;
}

void wm_close(int id) {
    if(id < 0 || id >= MAX_WINDOWS) return;
    windows[id].visible  = 0;
    windows[id].focused  = 0;
    windows[id].dragging = 0;
    if(wm_focused == id) {
        int i;
        wm_focused = -1;
        for(i=MAX_WINDOWS-1; i>=0; i--) {
            if(windows[i].visible) { wm_focused=i; break; }
        }
    }
}

/* ---- Draw one window frame (Win 3.1 style) ---- */
void wm_draw_frame(int id) {
    Window *w = &windows[id];
    int x=w->x, y=w->y, ww=w->w, wh=w->h;
    int tx, tw;

    if(!w->visible) return;

    /* Outer border black */
    vga_rect(x, y, ww, wh, COL_BLACK);

    /* 3D effect border */
    vga_hline(x+1, x+ww-2, y+1, COL_WIN_LGRAY);
    vga_vline(x+1, y+1, y+wh-2, COL_WIN_LGRAY);
    vga_hline(x+1, x+ww-2, y+wh-2, COL_WIN_DGRAY);
    vga_vline(x+ww-2, y+1, y+wh-2, COL_WIN_DGRAY);

    /* Title bar */
    if(w->focused)
        vga_fillrect(x+2, y+2, ww-4, TITLE_H, COL_WIN_BLUE);
    else
        vga_fillrect(x+2, y+2, ww-4, TITLE_H, COL_DARK_GRAY);

    /* Title text centered */
    tw = vga_strwidth(w->title);
    tx = x + 2 + (ww-4-tw)/2;
    if(tx < x+4) tx = x+4;
    if(w->focused)
        vga_putstr(tx, y+4, w->title, COL_WHITE, COL_WIN_BLUE);
    else
        vga_putstr(tx, y+4, w->title, COL_LIGHT_GRAY, COL_DARK_GRAY);

    /* Close button (top-right corner) */
    vga_fillrect(x+ww-14, y+3, 11, 9, COL_BTN_FACE);
    vga_hline(x+ww-14, x+ww-4, y+3,    COL_BTN_HILITE);
    vga_vline(x+ww-14, y+3,  y+11,     COL_BTN_HILITE);
    vga_hline(x+ww-14, x+ww-4, y+11,   COL_BTN_SHADOW);
    vga_vline(x+ww-4,  y+3,  y+11,     COL_BTN_SHADOW);
    /* X mark */
    vga_putchar(x+ww-12, y+3, 'x', COL_BLACK, COL_BTN_FACE);

    /* Window background */
    vga_fillrect(x+2, y+2+TITLE_H, ww-4, wh-4-TITLE_H, COL_WIN_GRAY);

    /* Title bar separator line */
    vga_hline(x+2, x+ww-3, y+2+TITLE_H, COL_WIN_DGRAY);
}

void wm_draw_all(void) {
    int i;
    for(i=0; i<MAX_WINDOWS; i++)
        if(windows[i].visible)
            wm_draw_frame(i);
}

/* ---- Returns 1 if the close [x] button was mouse-clicked ---- */
int wm_close_clicked(int id) {
    Window *w = &windows[id];
    if(!w->visible) return 0;
    return (mouse.left_click &&
            gui_inside(mouse.x, mouse.y,
                       w->x + w->w - 14, w->y + 3, 11, 9));
}

/* ---- Handle dragging and focus switching ---- */
void wm_process_mouse(void) {
    int i;

    /* If start menu is open, let it handle mouse first */
    /* This logic is in main.c, but we should be careful here */

    /* Release drag */
    if(!mouse.left) {
        for(i=0; i<MAX_WINDOWS; i++)
            windows[i].dragging = 0;
    }
    /* Move if already dragging */
    for(i=0; i<MAX_WINDOWS; i++) {
        if(windows[i].dragging) {
            windows[i].x = mouse.x - windows[i].drag_ox;
            windows[i].y = mouse.y - windows[i].drag_oy;
            /* Clamp to screen */
            if(windows[i].x < 0) windows[i].x = 0;
            if(windows[i].y < 0) windows[i].y = 0;
            if(windows[i].x + windows[i].w > SCREEN_W)
                windows[i].x = SCREEN_W - windows[i].w;
            if(windows[i].y + windows[i].h > SCREEN_H - 12)
                windows[i].y = SCREEN_H - 12 - windows[i].h;
            return;
        }
    }
    /* Start drag on title bar click */
    if(mouse.left_click) {
        for(i=MAX_WINDOWS-1; i>=0; i--) {
            Window *w = &windows[i];
            if(!w->visible) continue;
            if(gui_inside(mouse.x, mouse.y, w->x+2, w->y+2, w->w-16, TITLE_H)) {
                /* Focus */
                if(wm_focused >= 0) windows[wm_focused].focused=0;
                wm_focused = i;
                w->focused = 1;
                /* Start drag */
                w->dragging = 1;
                w->drag_ox = mouse.x - w->x;
                w->drag_oy = mouse.y - w->y;
                return;
            }
        }
    }
}

/* ---- Win 3.1 style button ---- */
void gui_button(int x, int y, int w, int h,
                const char *label, int pressed) {
    int tx, ty, tw;
    vga_fillrect(x, y, w, h, COL_BTN_FACE);
    if(!pressed) {
        vga_hline(x, x+w-1, y,   COL_BTN_HILITE);
        vga_vline(x, y, y+h-1,   COL_BTN_HILITE);
        vga_hline(x, x+w-1,y+h-1,COL_BTN_SHADOW);
        vga_vline(x+w-1,y,y+h-1, COL_BTN_SHADOW);
    } else {
        vga_hline(x, x+w-1, y,   COL_BTN_SHADOW);
        vga_vline(x, y, y+h-1,   COL_BTN_SHADOW);
        vga_hline(x, x+w-1,y+h-1,COL_BTN_HILITE);
        vga_vline(x+w-1,y,y+h-1, COL_BTN_HILITE);
    }
    tw = vga_strwidth(label);
    tx = x + (w - tw) / 2 + (pressed ? 1 : 0);
    ty = y + (h - 8)  / 2 + (pressed ? 1 : 0);
    vga_putstr(tx, ty, label, COL_BLACK, COL_BTN_FACE);
}

int gui_inside(int mx, int my, int x, int y, int w, int h) {
    return (mx >= x && mx < x+w && my >= y && my < y+h);
}

/* ---- Taskbar (12px, bottom of screen) ---- */
void gui_taskbar(const char *active_title) {
    int y = SCREEN_H - 12;
    /* Bar background */
    vga_fillrect(0, y, SCREEN_W, 12, COL_BTN_FACE);
    vga_hline(0, SCREEN_W-1, y, COL_BTN_HILITE);
    /* Start button */
    gui_button(1, y+1, 30, 10, "Start", 0);
    /* Separator */
    vga_vline(34, y+1, y+10, COL_BTN_SHADOW);
    vga_vline(35, y+1, y+10, COL_BTN_HILITE);
    /* Active window name */
    if(active_title)
        vga_putstr(38, y+2, active_title, COL_BLACK, COL_BTN_FACE);
    /* Clock placeholder */
    vga_putstr(270, y+2, "LixOS", COL_DARK_BLUE, COL_BTN_FACE);
}
