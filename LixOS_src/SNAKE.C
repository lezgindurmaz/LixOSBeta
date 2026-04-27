/* =========================================================
   SNAKE.C  -  LixOS Yilan Oyunu
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include "LIXUI.H"

#define MAX_SNAKE 100
#define GAME_W 40
#define GAME_H 15

typedef struct {
    int x, y;
} Point;

void main() {
    Point snake[MAX_SNAKE];
    int length = 3;
    int dir = 77; /* Sag */
    Point food;
    int i, ch;
    int score = 0;
    int x = 20, y = 5;
    int game_over = 0;

    randomize();
    snake[0].x = 10; snake[0].y = 5;
    snake[1].x = 9;  snake[1].y = 5;
    snake[2].x = 8;  snake[2].y = 5;

    food.x = random(GAME_W - 2) + 1;
    food.y = random(GAME_H - 2) + 1;

    _setcursortype(_NOCURSOR);

    while (!game_over) {
        /* Cizim */
        lixui_draw_window(x, y, GAME_W + 2, GAME_H + 2, " LixSnake ", YELLOW, RED, BLACK, LIGHTGRAY);

        lixui_setcolor(RED, LIGHTGRAY);
        gotoxy(x + 1 + food.x, y + 1 + food.y);
        putch('@');

        lixui_setcolor(GREEN, LIGHTGRAY);
        for (i = 0; i < length; i++) {
            gotoxy(x + 1 + snake[i].x, y + 1 + snake[i].y);
            putch(i == 0 ? 'O' : 'o');
        }

        lixui_statusbar("Oklar: Hareket  ESC: Cikis");
        gotoxy(x + 2, y + GAME_H + 1);
        lixui_setcolor(BLACK, LIGHTGRAY);
        cprintf(" Skor: %d ", score);

        delay(150);

        if (kbhit()) {
            ch = getch();
            if (ch == 0) ch = getch();
            if (ch == 27) break;
            if (ch == 72 && dir != 80) dir = 72;
            if (ch == 80 && dir != 72) dir = 80;
            if (ch == 75 && dir != 77) dir = 75;
            if (ch == 77 && dir != 75) dir = 77;
        }

        /* Hareket */
        for (i = length - 1; i > 0; i--) snake[i] = snake[i-1];

        if (dir == 72) snake[0].y--;
        if (dir == 80) snake[0].y++;
        if (dir == 75) snake[0].x--;
        if (dir == 77) snake[0].x++;

        /* Carpma kontrolu */
        if (snake[0].x < 0 || snake[0].x >= GAME_W || snake[0].y < 0 || snake[0].y >= GAME_H) game_over = 1;
        for (i = 1; i < length; i++) if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) game_over = 1;

        /* Yemek yeme */
        if (snake[0].x == food.x && snake[0].y == food.y) {
            score += 10;
            if (length < MAX_SNAKE) length++;
            food.x = random(GAME_W - 2) + 1;
            food.y = random(GAME_H - 2) + 1;
        }
    }

    if (game_over) {
        lixui_msgbox("Oyun Bitti", "Kaybettiniz!", WHITE, RED, BLACK, LIGHTGRAY);
    }
}
