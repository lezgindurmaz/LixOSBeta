#include "calc.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <stdio.h>
#include <string.h>

static int calc_win_id = -1;
static char calc_display[16] = "0";
static double calc_value = 0;
static double calc_last_value = 0;
static char calc_op = ' ';
static int calc_new_number = 1;

static void calc_add_digit(int d) {
    if (calc_new_number) {
        sprintf(calc_display, "%d", d);
        calc_new_number = 0;
    } else {
        if (strlen(calc_display) < 15) {
            char buf[2];
            sprintf(buf, "%d", d);
            strcat(calc_display, buf);
        }
    }
}

static void calc_do_op(char op) {
    double val = atof(calc_display);
    if (calc_op == '+') calc_value = calc_last_value + val;
    else if (calc_op == '-') calc_value = calc_last_value - val;
    else if (calc_op == '*') calc_value = calc_last_value * val;
    else if (calc_op == '/') calc_value = (val != 0) ? calc_last_value / val : 0;
    else calc_value = val;

    sprintf(calc_display, "%g", calc_value);
    calc_last_value = calc_value;
    calc_op = op;
    calc_new_number = 1;
}

void calc_open(void) {
    if (calc_win_id >= 0 && windows[calc_win_id].visible) return;
    calc_win_id = wm_open(100, 50, 100, 120, "Calculator", calc_update);
    strcpy(calc_display, "0");
    calc_value = 0;
    calc_op = ' ';
    calc_new_number = 1;
}

void calc_update(int id) {
    Window *w;
    if (id < 0 || !windows[id].visible) return;
    w = &windows[id];

    /* Draw display */
    vga_fillrect(w->x + 5, w->y + TITLE_H + 5, w->w - 10, 15, COL_WHITE);
    vga_rect(w->x + 5, w->y + TITLE_H + 5, w->w - 10, 15, COL_BLACK);
    vga_putstr(w->x + 10, w->y + TITLE_H + 8, calc_display, COL_BLACK, COL_WHITE);

    /* Draw buttons */
    int bx = w->x + 5, by = w->y + TITLE_H + 25;
    char *btns[] = {"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "C", "0", "=", "+"};
    for (int i = 0; i < 16; i++) {
        int row = i / 4;
        int col = i % 4;
        int x = bx + col * 23;
        int y = by + row * 22;
        gui_button(x, y, 20, 18, btns[i], 0);

        if (mouse.left_click && gui_inside(mouse.x, mouse.y, x, y, 20, 18)) {
            char c = btns[i][0];
            if (c >= '0' && c <= '9') calc_add_digit(c - '0');
            else if (c == 'C') { strcpy(calc_display, "0"); calc_value = 0; calc_op = ' '; calc_new_number = 1; }
            else if (c == '=') calc_do_op(' ');
            else calc_do_op(c);
        }
    }

    if (wm_close_clicked(calc_win_id)) {
        wm_close(calc_win_id);
        calc_win_id = -1;
    }

}
