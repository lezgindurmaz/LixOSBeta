/* =========================================================
   LIXUI.H  -  LixOS Ortak Kullanici Arayuzu Kutuphanesi
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#ifndef LIXUI_H
#define LIXUI_H

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

/* ---- Renk sabitleri (Turbo C ile ayni) ---- */
#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHTGRAY    7
#define DARKGRAY     8
#define LIGHTBLUE    9
#define LIGHTGREEN  10
#define LIGHTCYAN   11
#define LIGHTRED    12
#define LIGHTMAGENTA 13
#define YELLOW      14
#define WHITE       15

/* ---- Kutu cizim karakterleri (IBM CP437) ---- */
#define BOX_TL      (char)201   /*╔ */
#define BOX_TR      (char)187   /* ╗ */
#define BOX_BL      (char)200   /* ╚ */
#define BOX_BR      (char)188   /* ╝ */
#define BOX_H       (char)205   /* ═ */
#define BOX_V       (char)186   /* ║ */
#define BOX_ML      (char)204   /* ╠ */
#define BOX_MR      (char)185   /* ╣ */
#define BOX_MH      (char)205   /* ═ */

#define SBOX_TL     (char)218   /* ┌ */
#define SBOX_TR     (char)191   /* ┐ */
#define SBOX_BL     (char)192   /* └ */
#define SBOX_BR     (char)217   /* ┘ */
#define SBOX_H      (char)196   /* ─ */
#define SBOX_V      (char)179   /* │ */

#define SHADOW_CHAR (char)176   /* ░ */
#define BLOCK_FULL  (char)219   /* █ */
#define BLOCK_MED   (char)178   /* ▓ */
#define BLOCK_LIGHT (char)177   /* ▒ */
#define ARROW_R     (char)16    /* ► */
#define ARROW_L     (char)17    /* ◄ */
#define CHECK_MARK  (char)251   /* √ */
#define BULLET      (char)254   /* ■ */

/* ---- Ekran boyutlari ---- */
#define SCREEN_W    80
#define SCREEN_H    25

/* =========================================================
   lixui_cls()  -  Ekrani temizle, arka plan rengi ata
   ========================================================= */
void lixui_cls(int bg_color) {
    textbackground(bg_color);
    textcolor(WHITE);
    clrscr();
}

/* =========================================================
   lixui_setcolor()  -  Renk ata
   ========================================================= */
void lixui_setcolor(int fg, int bg) {
    textcolor(fg);
    textbackground(bg);
}

/* =========================================================
   lixui_gotoxy()  -  Cursor konumlandir (1-bazli)
   ========================================================= */
void lixui_gotoxy(int x, int y) {
    gotoxy(x, y);
}

/* =========================================================
   lixui_hline()  -  Yatay cizgi doldur
   ========================================================= */
void lixui_hline(int x, int y, int len, char ch) {
    int i;
    gotoxy(x, y);
    for (i = 0; i < len; i++) putch(ch);
}

/* =========================================================
   lixui_fill_rect()  -  Dikdortgen bolgey karakter ile doldur
   ========================================================= */
void lixui_fill_rect(int x, int y, int w, int h, char ch, int fg, int bg) {
    int i, j;
    lixui_setcolor(fg, bg);
    for (j = 0; j < h; j++) {
        gotoxy(x, y + j);
        for (i = 0; i < w; i++) putch(ch);
    }
}

/* =========================================================
   lixui_shadow()  -  Pencere sag/alt golge ciz
   ========================================================= */
void lixui_shadow(int x, int y, int w, int h) {
    int i;
    lixui_setcolor(DARKGRAY, BLACK);
    /* Alt golge */
    gotoxy(x + 1, y + h);
    for (i = 0; i < w; i++) putch(SHADOW_CHAR);
    /* Sag golge */
    for (i = 1; i < h; i++) {
        gotoxy(x + w, y + i);
        putch(SHADOW_CHAR);
    }
}

/* =========================================================
   lixui_draw_window()
   x,y : sol ust kose (1-bazli)
   w,h : genislik, yukseklik (cerceve dahil)
   title: pencere baslik metni (NULL ise yok)
   fg_title, bg_title : baslik rengi
   fg_body, bg_body   : govde rengi
   ========================================================= */
void lixui_draw_window(int x, int y, int w, int h,
                        const char *title,
                        int fg_title, int bg_title,
                        int fg_body,  int bg_body)
{
    int i;

    /* Golge */
    lixui_shadow(x, y, w, h);

    /* Govde doldur */
    lixui_fill_rect(x, y, w, h, ' ', fg_body, bg_body);

    /* Ust cerceve */
    lixui_setcolor(fg_body, bg_body);
    gotoxy(x, y);
    putch(BOX_TL);
    for (i = 1; i < w - 1; i++) putch(BOX_H);
    putch(BOX_TR);

    /* Yan cerceveler */
    for (i = 1; i < h - 1; i++) {
        gotoxy(x, y + i);     putch(BOX_V);
        gotoxy(x + w - 1, y + i); putch(BOX_V);
    }

    /* Alt cerceve */
    gotoxy(x, y + h - 1);
    putch(BOX_BL);
    for (i = 1; i < w - 1; i++) putch(BOX_H);
    putch(BOX_BR);

    /* Baslik cubu�u */
    if (title != NULL) {
        int tlen = strlen(title);
        int tstart = x + (w - tlen - 2) / 2;
        /* Baslik arkaplan */
        lixui_setcolor(fg_title, bg_title);
        gotoxy(x + 1, y);
        for (i = 0; i < w - 2; i++) putch(BOX_H);
        /* Baslik metni */
        gotoxy(tstart, y);
        cprintf(" %s ", title);
    }
}

/* =========================================================
   lixui_draw_button()
   Basit buton, secili ise vurgu rengiyle
   ========================================================= */
void lixui_draw_button(int x, int y, const char *label, int selected) {
    int len = strlen(label);
    if (selected) {
        lixui_setcolor(BLACK, LIGHTGRAY);
        gotoxy(x, y);
        cprintf("[ %s ]", label);
    } else {
        lixui_setcolor(WHITE, DARKGRAY);
        gotoxy(x, y);
        cprintf("[ %s ]", label);
    }
}

/* =========================================================
   lixui_msgbox()  -  Mesaj kutusu, tek OK butonu
   Merkeze hizalar, ENTER ile kapanir
   ========================================================= */
void lixui_msgbox(const char *title, const char *msg,
                   int fg_t, int bg_t, int fg_b, int bg_b)
{
    int mlen  = strlen(msg);
    int tlen  = strlen(title);
    int w     = (mlen > tlen ? mlen : tlen) + 6;
    int h     = 7;
    int x     = (SCREEN_W - w) / 2 + 1;
    int y     = (SCREEN_H - h) / 2 + 1;

    if (w < 20) w = 20;

    lixui_draw_window(x, y, w, h, title, fg_t, bg_t, fg_b, bg_b);

    /* Mesaj */
    lixui_setcolor(fg_b, bg_b);
    gotoxy(x + (w - mlen) / 2, y + 2);
    cputs(msg);

    /* Ayrac cizgi */
    lixui_setcolor(DARKGRAY, bg_b);
    gotoxy(x + 1, y + h - 3);
    lixui_hline(x + 1, y + h - 3, w - 2, BOX_H);

    /* OK butonu */
    lixui_draw_button(x + (w - 8) / 2, y + h - 2, " OK ", 1);

    while (getch() != 13); /* ENTER bekle */
}

/* =========================================================
   lixui_yesno()  -  Evet/Hayir sorusu
   Donus: 1=Evet, 0=Hayir
   ========================================================= */
int lixui_yesno(const char *title, const char *question) {
    int mlen = strlen(question);
    int tlen = strlen(title);
    int w    = (mlen > tlen ? mlen : tlen) + 8;
    int h    = 8;
    int x    = (SCREEN_W - w) / 2 + 1;
    int y    = (SCREEN_H - h) / 2 + 1;
    int sel  = 0; /* 0=Evet, 1=Hayir */
    int ch;
    int btn_y;
    int evet_x, hayir_x;

    if (w < 26) w = 26;

    lixui_draw_window(x, y, w, h, title, WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + (w - mlen) / 2, y + 2);
    cputs(question);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

    btn_y  = y + h - 2;
    evet_x  = x + (w / 2) - 9;
    hayir_x = x + (w / 2) + 1;

    do {
        lixui_draw_button(evet_x,  btn_y, "  Evet  ", sel == 0);
        lixui_draw_button(hayir_x, btn_y, "  Hayir ", sel == 1);

        ch = getch();
        if (ch == 0) ch = getch(); /* ozel tus */

        if (ch == 75) sel = 0;      /* Sol ok */
        if (ch == 77) sel = 1;      /* Sag ok */
        if (ch ==  9) sel = !sel;   /* TAB */
    } while (ch != 13);

    return (sel == 0) ? 1 : 0;
}

/* =========================================================
   lixui_progress()  -  Ilerleme cubugu ciz
   x,y: konum, w: genislik, pct: 0-100
   ========================================================= */
void lixui_progress(int x, int y, int w, int pct,
                     int fg_done, int bg_done,
                     int fg_left, int bg_left)
{
    int filled = (w * pct) / 100;
    int i;

    gotoxy(x, y);
    lixui_setcolor(fg_done, bg_done);
    for (i = 0; i < filled; i++) putch(BLOCK_FULL);
    lixui_setcolor(fg_left, bg_left);
    for (i = filled; i < w; i++) putch(BLOCK_LIGHT);
}

/* =========================================================
   lixui_statusbar()  -  Alt durum cubugu (satir 25)
   ========================================================= */
void lixui_statusbar(const char *msg) {
    int i;
    lixui_setcolor(WHITE, BLUE);
    gotoxy(1, 25);
    cprintf(" %-78s", msg);
}

/* =========================================================
   lixui_desktop()  -  Masaustu arkaplan (LixOS kabuğu icin)
   ========================================================= */
void lixui_desktop(const char *os_name, const char *version) {
    int i;
    lixui_setcolor(LIGHTGRAY, CYAN);
    clrscr();

    /* Ust baslik barı */
    lixui_setcolor(WHITE, BLUE);
    gotoxy(1, 1);
    for (i = 0; i < SCREEN_W; i++) putch(' ');
    gotoxy(1, 1);
    cprintf("  %c %s v%s", BULLET, os_name, version);

    /* Alt status barı */
    lixui_statusbar("F1=Yardim  F2=Dosyalar  F3=Kapat");
}

/* =========================================================
   lixui_wait_key()  -  "Devam etmek icin bir tusa basin..."
   ========================================================= */
void lixui_wait_key(void) {
    lixui_statusbar("Devam etmek icin bir tusa basin...");
    getch();
}

#endif /* LIXUI_H */
