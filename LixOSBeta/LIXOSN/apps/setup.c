#include "setup.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <stdio.h>
#include <string.h>

static int setup_win_id = -1;

void setup_update(int id);

void setup_open(void) {
    if (setup_win_id >= 0 && windows[setup_win_id].visible) return;
    setup_win_id = wm_open(40, 40, 240, 120, "LixOS Setup", setup_update);
}

void setup_update(int id) {
    Window *w;
    if (id < 0 || !windows[id].visible) return;
    w = &windows[id];

    vga_putstr(w->x + 10, w->y + TITLE_H + 10, "Welcome to LixOS Setup", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(w->x + 10, w->y + TITLE_H + 25, "This will install LixOS to C:", COL_BLACK, COL_WIN_GRAY);

    gui_button(w->x + 50, w->y + 80, 60, 20, "Install", 0);
    gui_button(w->x + 130, w->y + 80, 60, 20, "Cancel", 0);

    if (wm_close_clicked(setup_win_id)) {
        wm_close(setup_win_id);
        setup_win_id = -1;
    }
}
