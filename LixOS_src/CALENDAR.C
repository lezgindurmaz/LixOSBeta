/* =========================================================
   CALENDAR.C  -  LixOS Takvim
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include "LIXUI.H"

const char* MONTHS[] = {"Ocak", "Subat", "Mart", "Nisan", "Mayis", "Haziran",
                        "Temmuz", "Agustos", "Eylul", "Ekim", "Kasim", "Aralik"};

void main() {
    struct date d;
    int x = 25, y = 6, w = 32, h = 13;
    int i, day, start_day;
    char title[40];

    getdate(&d);
    sprintf(title, " %s %d ", MONTHS[d.da_mon - 1], d.da_year);

    lixui_draw_window(x, y, w, h, title, WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(x + 2, y + 2);
    cprintf(" Pz Pt Sa Ca Pe Cu Ct");
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 3, w - 2, SBOX_H);

    /* Basit bir takvim algoritmasi (ay basi hangi gun bulma kismi simule edilmistir) */
    lixui_setcolor(BLACK, LIGHTGRAY);
    for (i = 1; i <= 31; i++) {
        int row = (i + 3) / 7;
        int col = (i + 3) % 7;
        gotoxy(x + 2 + col * 4, y + 4 + row);
        if (i == d.da_day) lixui_setcolor(WHITE, RED);
        else lixui_setcolor(BLACK, LIGHTGRAY);
        cprintf("%2d", i);
    }

    lixui_statusbar("ESC: Cikis");
    getch();
}
