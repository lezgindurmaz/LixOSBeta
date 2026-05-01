/*=============================================================
  LIXVER.C  -  LixOS version / about dialog
  DJGPP / DOS
=============================================================*/
#include "lixver.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <string.h>

#define VER_W  180
#define VER_H  110

static int ver_win_id = -1;

void lixver_open(void) {
    if(ver_win_id >= 0 && windows[ver_win_id].visible) return;
    ver_win_id = wm_open(70, 40, VER_W, VER_H, "About LixOS");
}

int lixver_update(void) {
    Window *w;
    int cx, cy;

    if(ver_win_id < 0 || !windows[ver_win_id].visible) return 0;
    w  = &windows[ver_win_id];
    cx = w->x + 4;
    cy = w->y + 2 + TITLE_H + 4;

    /* ---- Content ---- */
    /* Logo text */
    vga_putstr(cx + 30, cy,      "LixOS", COL_WIN_BLUE,  COL_WIN_GRAY);
    vga_putstr(cx + 22, cy + 10, "Version 1.0", COL_BLACK, COL_WIN_GRAY);

    /* Separator */
    vga_hline(cx, cx + VER_W-10, cy+20, COL_WIN_DGRAY);

    /* Info lines */
    vga_putstr(cx, cy+24,  "Platform : MS-DOS 6.22", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx, cy+33,  "Arch     : 32-bit (DPMI)", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx, cy+42,  "Graphics : VGA 320x200", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx, cy+51,  "Compiler : DJGPP 2.x",   COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx, cy+60,  "Build    : May 2025",     COL_BLACK, COL_WIN_GRAY);

    /* Separator */
    vga_hline(cx, cx + VER_W-10, cy+71, COL_WIN_DGRAY);

    vga_putstr(cx+10, cy+74, "LixOS - Lightweight Interactive", COL_DARK_GRAY, COL_WIN_GRAY);
    vga_putstr(cx+30, cy+83,  "eXtended OS Shell", COL_DARK_GRAY, COL_WIN_GRAY);

    /* OK button */
    gui_button(cx + VER_W/2 - 20, cy + VER_H - 32, 40, 12, "OK", 0);

    /* --- Close on OK or X button --- */
    if(wm_close_clicked(ver_win_id) ||
       (mouse.left_click &&
        gui_inside(mouse.x, mouse.y,
                   cx + VER_W/2 - 20, cy + VER_H - 32, 40, 12))) {
        wm_close(ver_win_id);
        ver_win_id = -1;
        return 0;
    }
    return 1;
}
