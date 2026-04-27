/* =========================================================
   CLOCK.C  -  LixOS Saat
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "LIXUI.H"

void main() {
    struct time t;
    int x = 20, y = 8, w = 40, h = 8;

    _setcursortype(_NOCURSOR);

    while (!kbhit()) {
        gettime(&t);
        lixui_draw_window(x, y, w, h, " LixOS Saat ", YELLOW, BLUE, BLACK, LIGHTGRAY);

        lixui_setcolor(BLUE, LIGHTGRAY);
        gotoxy(x + 5, y + 3);
        /* Buyuk saat gorunumu simule et */
        cprintf("      %02d : %02d : %02d      ", t.ti_hour, t.ti_min, t.ti_sec);

        lixui_setcolor(DARKGRAY, LIGHTGRAY);
        gotoxy(x + 5, y + 5);
        cprintf("   Cikmak icin bir tusa basin   ");

        delay(500);
    }
    getch();
}
