#include "fman.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <stdio.h>
#include <string.h>
#include <dir.h>

static int fman_win_id = -1;
static char files[15][13];
static int file_count = 0;

void fman_open(void) {
    if (fman_win_id >= 0 && windows[fman_win_id].visible) return;
    fman_win_id = wm_open(50, 40, 150, 130, "File Manager");

    struct ffblk ffblk;
    int done = findfirst("*.*", &ffblk, 0);
    file_count = 0;
    while (!done && file_count < 15) {
        strcpy(files[file_count++], ffblk.ff_name);
        done = findnext(&ffblk);
    }
}

int fman_update(void) {
    Window *w;
    if (fman_win_id < 0 || !windows[fman_win_id].visible) return 0;
    w = &windows[fman_win_id];

    for (int i = 0; i < file_count; i++) {
        vga_putstr(w->x + 10, w->y + TITLE_H + 5 + i * 9, files[i], COL_BLACK, COL_WIN_GRAY);
    }

    if (wm_close_clicked(fman_win_id)) {
        wm_close(fman_win_id);
        fman_win_id = -1;
        return 0;
    }

    return 1;
}
