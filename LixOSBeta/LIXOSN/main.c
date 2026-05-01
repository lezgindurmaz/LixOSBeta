/*=============================================================
  MAIN.C  -  LixOS main shell
  Compile:  gcc -O2 -o LIXOS.EXE main.c kernel/vga.c kernel/mouse.c
            kernel/window.c apps/notepad.c apps/lixver.c apps/setup.c
  (DJGPP gcc on DOS)
=============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <conio.h>

#include "kernel/vga.h"
#include "kernel/mouse.h"
#include "kernel/window.h"
#include "apps/notepad.h"
#include "apps/lixver.h"
#include "apps/setup.h"
#include "apps/calc.h"
#include "apps/fman.h"

/* ---- Desktop icons ---- */
#define ICON_W   32
#define ICON_H   32
#define ICON_LBL  8   /* pixels below icon for label */

typedef struct {
    int x, y;
    const char *label;
    void (*open)(void);
} Icon;

static Icon icons[] = {
    {  8, 10, "Notepad", notepad_open  },
    {  8, 60, "Version", lixver_open   },
    {  8,110, "Setup",   setup_open    },
    { 50, 10, "Calc",    calc_open     },
    { 50, 60, "Files",   fman_open     },
};
#define N_ICONS (sizeof(icons)/sizeof(icons[0]))

/* ---- Draw a simple icon (folder-style box) ---- */
static void draw_icon(int x, int y, const char *label, int selected) {
    unsigned char bg   = selected ? COL_WIN_BLUE : COL_DESKTOP;
    unsigned char fg   = COL_WHITE;
    unsigned char ibg  = selected ? COL_WIN_BLUE : COL_WIN_GRAY;

    /* Icon box with 3D effect */
    vga_fillrect(x, y, ICON_W, ICON_H, ibg);
    vga_rect    (x, y, ICON_W, ICON_H, COL_BLACK);
    vga_hline(x+1, x+ICON_W-2, y+1, COL_WHITE);
    vga_vline(x+1, y+1, y+ICON_H-2, COL_WHITE);

    /* Small decorative lines inside icon */
    vga_hline(x+6, x+ICON_W-7, y+ 10, COL_BLACK);
    vga_hline(x+6, x+ICON_W-7, y+16, COL_BLACK);
    vga_hline(x+6, x+ICON_W-7, y+22, COL_BLACK);

    /* Label below icon */
    {
        int lw = vga_strwidth(label);
        int lx = x + (ICON_W - lw) / 2;
        if(lx < x) lx = x;
        vga_fillrect(lx-1, y+ICON_H+1, lw+2, 9, bg);
        vga_putstr(lx, y+ICON_H+1, label, fg, bg);
    }
}

static int selected_icon = -1;

static void draw_desktop(void) {
    int i;
    /* Teal desktop */
    vga_fillrect(0, 0, SCREEN_W, SCREEN_H - 12, COL_DESKTOP);

    /* Desktop title */
    vga_putstr(50, 2, "LixOS Desktop", COL_LIGHT_GRAY, COL_DESKTOP);

    /* Icons */
    for(i = 0; i < (int)N_ICONS; i++)
        draw_icon(icons[i].x, icons[i].y, icons[i].label, selected_icon == i);
}

/* ---- Start menu ---- */
static int start_menu_open = 0;

static void draw_start_menu(void) {
    int mx = 1, my = SCREEN_H - 12 - 60;
    vga_fillrect(mx, my, 70, 60, COL_MENU_BG);
    vga_rect(mx, my, 70, 60, COL_WIN_DGRAY);

    vga_putstr(mx+4, my+4,  "Notepad", COL_BLACK, COL_MENU_BG);
    vga_hline(mx+1, mx+68, my+14, COL_WIN_DGRAY);
    vga_putstr(mx+4, my+16, "Version", COL_BLACK, COL_MENU_BG);
    vga_hline(mx+1, mx+68, my+26, COL_WIN_DGRAY);
    vga_putstr(mx+4, my+28, "Setup",   COL_BLACK, COL_MENU_BG);
    vga_hline(mx+1, mx+68, my+38, COL_WIN_DGRAY);
    vga_putstr(mx+4, my+42, "Shutdown",COL_BLACK, COL_MENU_BG);
    vga_hline(mx+1, mx+68, my+52, COL_WIN_DGRAY);
}

static void handle_start_menu(void) {
    int mx = 1, my = SCREEN_H - 12 - 60;
    if(!mouse.left_click) return;
    if(gui_inside(mouse.x, mouse.y, mx+2, my+4,  64, 10))
        { notepad_open(); start_menu_open=0; }
    else if(gui_inside(mouse.x, mouse.y, mx+2, my+16, 64, 10))
        { lixver_open();  start_menu_open=0; }
    else if(gui_inside(mouse.x, mouse.y, mx+2, my+28, 64, 10))
        { setup_open();   start_menu_open=0; }
    else if(gui_inside(mouse.x, mouse.y, mx+2, my+42, 64, 10)) {
        /* Shutdown: restore text mode and exit */
        vga_setmode(0x03);
        printf("\nLixOS shut down. It is now safe to turn off your computer.\n");
        exit(0);
    }
    else start_menu_open=0;
}

/* ---- Main loop ---- */
int main(void) {
    int running = 1;

    vga_init();
    if(!mouse_init()) {
        vga_setmode(0x03);
        printf("ERROR: No mouse driver found.\n");
        printf("Please load a DOS mouse driver (e.g. CTMOUSE.EXE) before starting LixOS.\n");
        return 1;
    }
    wm_init();

    while(running) {
        /* ---- Input ---- */
        mouse_update();

        /* Start menu has highest priority if open */
        if (start_menu_open) {
            handle_start_menu();
        } else {
            /* If no start menu, handle windows then desktop */
            wm_process_mouse();
        }

        /* ---- Handle Start button ---- */
        if(mouse.left_click &&
           gui_inside(mouse.x, mouse.y, 1, SCREEN_H-11, 30, 10)) {
            start_menu_open = !start_menu_open;
        }

        /* ---- Handle icon double-click (simulate: rapid click) ---- */
        {
            static int last_selected = -1;
            static int click_timer   = 0;
            int i;

            if(click_timer > 0) click_timer--;

            if(mouse.left_click) {
                int found = 0;
                for(i=0; i<(int)N_ICONS; i++) {
                    if(gui_inside(mouse.x, mouse.y,
                                  icons[i].x, icons[i].y,
                                  ICON_W, ICON_H+10)) {
                        found = 1;
                        if(last_selected == i && click_timer > 0) {
                            /* Double click! */
                            icons[i].open();
                            click_timer = 0;
                            last_selected = -1;
                        } else {
                            /* Single click */
                            selected_icon = i;
                            last_selected = i;
                            click_timer = 40; /* Increased timer for better reliability */
                        }
                        break;
                    }
                }
                if (!found) {
                    selected_icon = -1;
                    last_selected = -1;
                }
            }
        }

        /* ---- Draw ---- */
        draw_desktop();

        /* Window frames first, then content */
        wm_draw_all();

        /* Content is drawn via wm_draw_all callbacks */

        /* Start menu on top of everything */
        if(start_menu_open) {
            draw_start_menu();
        }

        /* Taskbar */
        {
            const char *atitle = (wm_focused >= 0 && windows[wm_focused].visible)
                                 ? windows[wm_focused].title : NULL;
            gui_taskbar(atitle);
        }

        /* Mouse cursor always last */
        mouse_draw_cursor();

        /* Blit to screen */
        vga_flip();

        /* ESC from desktop = shutdown */
        if(kbhit()) {
            int k = getch();
            if(k == 0) getch(); /* consume extended */
            if(k == 27) {
                vga_setmode(0x03);
                printf("\nLixOS shut down.\n");
                running = 0;
            }
        }
    }
    return 0;
}
