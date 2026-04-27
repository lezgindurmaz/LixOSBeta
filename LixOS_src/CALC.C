/* =========================================================
   CALC.C  -  LixOS Hesap Makinesi
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include "LIXUI.H"

void draw_calc(double val, const char* op_str) {
    int x = 25, y = 7, w = 30, h = 12;
    lixui_draw_window(x, y, w, h, " Hesap Makinesi ", WHITE, BLUE, BLACK, LIGHTGRAY);

    /* Ekran */
    lixui_fill_rect(x + 2, y + 2, w - 4, 3, ' ', WHITE, DARKGRAY);
    lixui_setcolor(YELLOW, DARKGRAY);
    gotoxy(x + 3, y + 3);
    cprintf("%15.4f", val);
    gotoxy(x + 3, y + 4);
    cprintf("Islem: %s", op_str);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 2, y + 6);  cprintf(" 7  8  9   / ");
    gotoxy(x + 2, y + 7);  cprintf(" 4  5  6   * ");
    gotoxy(x + 2, y + 8);  cprintf(" 1  2  3   - ");
    gotoxy(x + 2, y + 9);  cprintf(" C  0  =   + ");

    lixui_statusbar("Rakamlar ve Islem Tuslarini Kullanin. ESC: Cik");
}

int main() {
    double total = 0, current = 0;
    char op = ' ';
    int ch;
    char op_text[10] = "";

    _setcursortype(_NOCURSOR);

    while (1) {
        draw_calc(current, op_text);
        ch = getch();

        if (ch >= '0' && ch <= '9') {
            current = current * 10 + (ch - '0');
        } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            total = current;
            current = 0;
            op = ch;
            sprintf(op_text, "%c", op);
        } else if (ch == 13 || ch == '=') {
            if (op == '+') total += current;
            if (op == '-') total -= current;
            if (op == '*') total *= current;
            if (op == '/') if (current != 0) total /= current;
            current = total;
            strcpy(op_text, "Sonuc");
        } else if (ch == 'c' || ch == 'C') {
            total = 0;
            current = 0;
            op = ' ';
            strcpy(op_text, "");
        } else if (ch == 27) {
            break;
        }
    }

    return 0;
}
