/* =========================================================
   NOTEPAD.C  -  LixOS Metin Duzenleyici
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   Derleme: TCC -ml NOTEPAD.C
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include "LIXUI.H"

/* =========================================================
   Sabitler
   ========================================================= */
#define MAX_LINES       500
#define MAX_LINE_LEN    255
#define EDIT_TOP        3           /* Duzenleme alaninın baslangic satiri */
#define EDIT_ROWS       21          /* Gorunen satir sayisi (3-23)         */
#define EDIT_LEFT       1           /* Sol kenar (1-bazli)                 */
#define EDIT_COLS       80          /* Gorunen sutun sayisi                */
#define STATUS_ROW      25

/* Renkler */
#define CLR_EDIT_FG     WHITE
#define CLR_EDIT_BG     BLUE
#define CLR_MENU_FG     WHITE
#define CLR_MENU_BG     BLUE
#define CLR_SEL_FG      BLACK
#define CLR_SEL_BG      CYAN
#define CLR_STAT_FG     BLACK
#define CLR_STAT_BG     LIGHTGRAY
#define CLR_LINE_FG     DARKGRAY
#define CLR_NUM_BG      BLUE

/* Kontrol tus kodlari */
#define KEY_CTRL_N      14
#define KEY_CTRL_O      15
#define KEY_CTRL_S      19
#define KEY_CTRL_F      6
#define KEY_CTRL_HOME   119
#define KEY_CTRL_END    117
#define KEY_CTRL_LEFT   115
#define KEY_CTRL_RIGHT  116
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F5          63
#define KEY_UP          72
#define KEY_DOWN        80
#define KEY_LEFT        75
#define KEY_RIGHT       77
#define KEY_HOME        71
#define KEY_END         79
#define KEY_PGUP        73
#define KEY_PGDN        81
#define KEY_DEL         83
#define KEY_INS         82
#define KEY_ALT_F       33
#define KEY_ALT_D       32
#define KEY_ALT_Y       21

/* =========================================================
   Global metin tamponu
   ========================================================= */
static char  g_buf[MAX_LINES][MAX_LINE_LEN + 1];
static int   g_line_count   = 1;     /* Toplam satir sayisi (min 1) */
static int   g_cur_line     = 0;     /* Imle satiri (0-bazli)       */
static int   g_cur_col      = 0;     /* Imle sutunu (0-bazli)       */
static int   g_scroll_line  = 0;     /* Goruntulenen ilk satir      */
static int   g_scroll_col   = 0;     /* Goruntulenen ilk sutun      */
static int   g_modified     = 0;     /* Duzenleme yapildi mi?       */
static int   g_insert_mode  = 1;     /* 1=Ekle, 0=Uzerine yaz      */

/* Dosya */
static char  g_filename[128] = "";   /* Mevcut dosya adi            */

/* Bul */
static char  g_find_str[64]  = "";   /* Aranacak metin              */
static int   g_find_line     = 0;    /* Son bulunan satir           */
static int   g_find_col      = 0;    /* Son bulunan sutun           */

/* Calisma bayragi */
static int   g_running       = 1;

/* =========================================================
   Tampon yardimci fonksiyonlar
   ========================================================= */

/* Mevcut satirin uzunlugu */
static int cur_line_len(void) {
    return strlen(g_buf[g_cur_line]);
}

/* Imleyi goruntu sinirlarinda tut */
static void clamp_cursor(void) {
    int llen;
    if (g_cur_line < 0)             g_cur_line = 0;
    if (g_cur_line >= g_line_count) g_cur_line = g_line_count - 1;

    llen = (int)strlen(g_buf[g_cur_line]);
    if (g_cur_col < 0)    g_cur_col = 0;
    if (g_cur_col > llen) g_cur_col = llen;
}

/* Kaydirmayi imlece gore guncelle */
static void update_scroll(void) {
    if (g_cur_line < g_scroll_line)
        g_scroll_line = g_cur_line;
    if (g_cur_line >= g_scroll_line + EDIT_ROWS)
        g_scroll_line = g_cur_line - EDIT_ROWS + 1;

    if (g_cur_col < g_scroll_col)
        g_scroll_col = g_cur_col;
    if (g_cur_col >= g_scroll_col + EDIT_COLS)
        g_scroll_col = g_cur_col - EDIT_COLS + 1;

    if (g_scroll_line < 0) g_scroll_line = 0;
    if (g_scroll_col  < 0) g_scroll_col  = 0;
}

/* Satir sonuna karakter ekle veya ortaya ekle */
static void buf_insert_char(char c) {
    char *line = g_buf[g_cur_line];
    int   llen = strlen(line);
    int   col  = g_cur_col;
    int   i;

    if (llen >= MAX_LINE_LEN) return; /* Satir dolu */

    if (g_insert_mode) {
        /* Sagdaki karakterleri bir sag kay */
        for (i = llen; i > col; i--)
            line[i] = line[i - 1];
        line[col] = c;
        line[llen + 1] = '\0';
    } else {
        /* Uzerine yaz */
        if (col <= llen) line[col] = c;
        if (col == llen) { line[llen + 1] = '\0'; }
    }
    g_cur_col++;
    g_modified = 1;
}

/* Imle oncesindeki karakteri sil (Backspace) */
static void buf_backspace(void) {
    char *line = g_buf[g_cur_line];
    int   llen = strlen(line);
    int   col  = g_cur_col;
    int   i;

    if (col > 0) {
        /* Mevcut satirdan sil */
        for (i = col - 1; i < llen - 1; i++)
            line[i] = line[i + 1];
        line[llen - 1] = '\0';
        g_cur_col--;
    } else if (g_cur_line > 0) {
        /* Satir basindayiz: onceki satira birlestir */
        int prev_len = strlen(g_buf[g_cur_line - 1]);
        int avail    = MAX_LINE_LEN - prev_len;
        int copy_len = llen < avail ? llen : avail;

        strncat(g_buf[g_cur_line - 1], line, copy_len);

        /* Mevcut satiri sil: satirlari yukari kay */
        for (i = g_cur_line; i < g_line_count - 1; i++)
            memcpy(g_buf[i], g_buf[i + 1], MAX_LINE_LEN + 1);

        g_buf[g_line_count - 1][0] = '\0';
        g_line_count--;
        g_cur_line--;
        g_cur_col = prev_len;
    }
    g_modified = 1;
}

/* Imle konumundaki karakteri sil (Delete) */
static void buf_delete(void) {
    char *line = g_buf[g_cur_line];
    int   llen = strlen(line);
    int   col  = g_cur_col;
    int   i;

    if (col < llen) {
        /* Karakteri sil */
        for (i = col; i < llen - 1; i++)
            line[i] = line[i + 1];
        line[llen - 1] = '\0';
    } else if (g_cur_line < g_line_count - 1) {
        /* Satir sonu: sonraki satiri birlestir */
        int next_len = strlen(g_buf[g_cur_line + 1]);
        int avail    = MAX_LINE_LEN - llen;
        int copy_len = next_len < avail ? next_len : avail;

        strncat(line, g_buf[g_cur_line + 1], copy_len);

        /* Sonraki satiri sil */
        for (i = g_cur_line + 1; i < g_line_count - 1; i++)
            memcpy(g_buf[i], g_buf[i + 1], MAX_LINE_LEN + 1);

        g_buf[g_line_count - 1][0] = '\0';
        g_line_count--;
    }
    g_modified = 1;
}

/* Enter: satiri bol */
static void buf_enter(void) {
    char *line = g_buf[g_cur_line];
    int   llen = strlen(line);
    int   col  = g_cur_col;
    int   i;
    char  rest[MAX_LINE_LEN + 1];

    if (g_line_count >= MAX_LINES) return;

    /* Satiri bolunme noktasindan sonrasini al */
    strncpy(rest, line + col, MAX_LINE_LEN);
    rest[MAX_LINE_LEN] = '\0';
    line[col] = '\0';

    /* Sonraki satirlari bir asagi kay */
    for (i = g_line_count - 1; i > g_cur_line; i--)
        memcpy(g_buf[i + 1], g_buf[i], MAX_LINE_LEN + 1);

    /* Yeni satira kalanı yaz */
    g_cur_line++;
    strncpy(g_buf[g_cur_line], rest, MAX_LINE_LEN);
    g_buf[g_cur_line][MAX_LINE_LEN] = '\0';
    g_line_count++;
    g_cur_col = 0;
    g_modified = 1;
}

/* =========================================================
   Tamponu sifirla (Yeni dosya)
   ========================================================= */
static void buf_clear(void) {
    int i;
    for (i = 0; i < MAX_LINES; i++) g_buf[i][0] = '\0';
    g_line_count  = 1;
    g_cur_line    = 0;
    g_cur_col     = 0;
    g_scroll_line = 0;
    g_scroll_col  = 0;
    g_modified    = 0;
    g_filename[0] = '\0';
}

/* =========================================================
   Dosya islemi: Yükle
   ========================================================= */
static int file_load(const char *path) {
    FILE *f = fopen(path, "r");
    int   n = 0;
    char  tmp[MAX_LINE_LEN + 4];

    if (!f) return 0;

    for (i = 0; i < MAX_LINES; i++) g_buf[i][0] = '\0';
    g_line_count = 0;

    while (fgets(tmp, sizeof(tmp), f) && n < MAX_LINES) {
        int tlen = strlen(tmp);
        /* Satir sonu karakterlerini temizle */
        while (tlen > 0 && (tmp[tlen-1] == '\n' || tmp[tlen-1] == '\r')) {
            tmp[--tlen] = '\0';
        }
        /* MAX_LINE_LEN'e kirp */
        if (tlen > MAX_LINE_LEN) tlen = MAX_LINE_LEN;
        tmp[tlen] = '\0';
        strncpy(g_buf[n], tmp, MAX_LINE_LEN);
        n++;
    }
    fclose(f);

    if (n == 0) { g_buf[0][0] = '\0'; n = 1; }
    g_line_count  = n;
    g_cur_line    = 0;
    g_cur_col     = 0;
    g_scroll_line = 0;
    g_scroll_col  = 0;
    g_modified    = 0;
    strncpy(g_filename, path, 127);
    return 1;
}

/* Derleme hatasini onlemek icin degisken bildirimi */
static int i; /* global gecici dongu degiskeni - asagida sakinca yok */

/* =========================================================
   Dosya islemi: Kaydet
   ========================================================= */
static int file_save(const char *path) {
    FILE *f = fopen(path, "w");
    int   n;
    if (!f) return 0;

    for (n = 0; n < g_line_count; n++) {
        fprintf(f, "%s\r\n", g_buf[n]);
    }
    fclose(f);
    g_modified = 0;
    strncpy(g_filename, path, 127);
    return 1;
}

/* =========================================================
   Durum cubugunu guncelle
   ========================================================= */
static void draw_statusbar(void) {
    char stat[82];
    char fname[30];
    char ins_str[6];

    /* Dosya adi kisalt */
    if (g_filename[0]) {
        char *slash = strrchr(g_filename, '\\');
        strncpy(fname, slash ? slash + 1 : g_filename, 29);
        fname[29] = '\0';
    } else {
        strcpy(fname, "[Isimsiz]");
    }

    strcpy(ins_str, g_insert_mode ? "EKL" : "UYZ");

    sprintf(stat, " %s%s  Sat:%-4d Sut:%-3d  %s  F1=Yardim  ALT=Menu",
            fname,
            g_modified ? " *" : "  ",
            g_cur_line + 1,
            g_cur_col  + 1,
            ins_str);

    lixui_setcolor(CLR_STAT_FG, CLR_STAT_BG);
    gotoxy(1, STATUS_ROW);
    cprintf("%-80s", stat);
}

/* =========================================================
   Menu cubugu
   ========================================================= */
static void draw_menubar(int open_idx) {
    /* 0=Dosya, 1=Duzen, 2=Yardim, -1=hic biri */
    const struct { const char *lbl; int col; } mbar[] = {
        { " Dosya ",  1 },
        { " Duzen ", 10 },
        { " Yardim", 19 }
    };
    int mc = sizeof(mbar) / sizeof(mbar[0]);
    int j;

    lixui_setcolor(CLR_MENU_FG, CLR_MENU_BG);
    gotoxy(1, 1);
    for (j = 0; j < SCREEN_W; j++) putch(' ');

    /* Baslik ortalanmis */
    {
        const char *title = g_filename[0] ? g_filename : "[Isimsiz]";
        int tlen = strlen(title) + 3; /* ' * ' */
        gotoxy(SCREEN_W / 2 - tlen / 2, 1);
        lixui_setcolor(YELLOW, CLR_MENU_BG);
        cprintf("%s%s", title, g_modified ? " *" : "  ");
    }

    /* Menu ogeleri */
    for (j = 0; j < mc; j++) {
        gotoxy(mbar[j].col, 1);
        if (j == open_idx) {
            lixui_setcolor(CLR_SEL_FG, CLR_SEL_BG);
        } else {
            lixui_setcolor(CLR_MENU_FG, CLR_MENU_BG);
        }
        cputs(mbar[j].lbl);
    }

    /* Ayrac */
    lixui_setcolor(DARKGRAY, CLR_MENU_BG);
    gotoxy(1, 2);
    for (j = 0; j < SCREEN_W; j++) putch(BOX_H);
}

/* =========================================================
   Duzenleme alanini ciz
   ========================================================= */
static void draw_editarea(void) {
    int row, col;
    int scr_row;

    for (row = 0; row < EDIT_ROWS; row++) {
        int buf_line = g_scroll_line + row;
        scr_row = EDIT_TOP + row;

        lixui_setcolor(CLR_EDIT_FG, CLR_EDIT_BG);
        gotoxy(EDIT_LEFT, scr_row);

        if (buf_line < g_line_count) {
            char *line = g_buf[buf_line];
            int   llen = strlen(line);
            int   shown = 0;

            /* Yatay kaydirilmis karakterleri yaz */
            for (col = g_scroll_col;
                 col < g_scroll_col + EDIT_COLS && col <= llen;
                 col++) {
                if (col < llen) {
                    putch(line[col]);
                }
                shown++;
            }
            /* Satir sonunu bosluklarla doldur */
            while (shown < EDIT_COLS) { putch(' '); shown++; }
        } else {
            /* Bos satir */
            for (col = 0; col < EDIT_COLS; col++) putch(' ');
        }
    }
}

/* =========================================================
   Imleci ekranda dogru konuma tasiy
   ========================================================= */
static void place_cursor(void) {
    int scr_col = EDIT_LEFT + (g_cur_col - g_scroll_col);
    int scr_row = EDIT_TOP  + (g_cur_line - g_scroll_line);
    if (scr_col < EDIT_LEFT) scr_col = EDIT_LEFT;
    if (scr_col > EDIT_LEFT + EDIT_COLS - 1)
        scr_col = EDIT_LEFT + EDIT_COLS - 1;
    gotoxy(scr_col, scr_row);
}

/* =========================================================
   Tek satiri yeniden ciz (hizli guncelleme)
   ========================================================= */
static void redraw_line(int buf_line) {
    int col, shown = 0;
    int scr_row = EDIT_TOP + (buf_line - g_scroll_line);

    if (scr_row < EDIT_TOP || scr_row >= EDIT_TOP + EDIT_ROWS) return;

    lixui_setcolor(CLR_EDIT_FG, CLR_EDIT_BG);
    gotoxy(EDIT_LEFT, scr_row);

    if (buf_line < g_line_count) {
        char *line = g_buf[buf_line];
        int   llen = strlen(line);
        for (col = g_scroll_col;
             col < g_scroll_col + EDIT_COLS;
             col++) {
            putch(col < llen ? line[col] : ' ');
            shown++;
        }
    } else {
        for (col = 0; col < EDIT_COLS; col++) putch(' ');
    }
}

/* =========================================================
   Tam ekran yenile
   ========================================================= */
static void full_redraw(void) {
    _setcursortype(_NOCURSOR);
    draw_menubar(-1);
    draw_editarea();
    draw_statusbar();
    update_scroll();
    place_cursor();
    _setcursortype(_NORMALCURSOR);
}

/* =========================================================
   Metin giris diyalogu (dosya adi, arama vb.)
   x,y,w: konum ve genislik
   buf: cikis tamponu, max_len: maksimum uzunluk
   title, default_val: baslik ve varsayilan deger
   Donus: 1=ENTER, 0=ESC
   ========================================================= */
static int input_dialog(const char *title, const char *prompt,
                         char *buf, int max_len,
                         const char *default_val)
{
    int dw   = max_len + 6;
    int dx   = (SCREEN_W - dw) / 2 + 1;
    int dy   = 10;
    int dh   = 8;
    int pos;
    int ch;

    if (dw < 40) dw = 40;

    strncpy(buf, default_val ? default_val : "", max_len);
    buf[max_len] = '\0';
    pos = strlen(buf);

    _setcursortype(_NOCURSOR);
    lixui_draw_window(dx, dy, dw, dh, title,
                      WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(dx + 2, dy + 2);
    cputs(prompt);

    /* Input kutusu cercevesi */
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(dx + 2, dy + 4);
    putch(SBOX_TL);
    {int j; for (j = 0; j < dw - 4; j++) putch(SBOX_H);}
    putch(SBOX_TR);
    gotoxy(dx + 2, dy + 5);
    putch(SBOX_V);
    lixui_setcolor(BLACK, WHITE);
    {int j; for (j = 0; j < dw - 4; j++) putch(' ');}
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    putch(SBOX_V);
    gotoxy(dx + 2, dy + 6);
    putch(SBOX_BL);
    {int j; for (j = 0; j < dw - 4; j++) putch(SBOX_H);}
    putch(SBOX_BR);

    lixui_statusbar("ENTER=Onayla  ESC=Iptal  Geri Tus=Sil");
    _setcursortype(_NORMALCURSOR);

    for (;;) {
        /* Input metnini goster */
        lixui_setcolor(BLACK, WHITE);
        gotoxy(dx + 3, dy + 5);
        cprintf("%-*s", dw - 5, buf);
        gotoxy(dx + 3 + pos, dy + 5);

        ch = getch();
        if (ch == 0) { getch(); continue; } /* Extended tus - yok say */

        if (ch == 13) { _setcursortype(_NOCURSOR); return 1; }
        if (ch == 27) { buf[0] = '\0'; _setcursortype(_NOCURSOR); return 0; }

        if (ch == 8) { /* Backspace */
            if (pos > 0) { pos--; buf[pos] = '\0'; }
        } else if (isprint(ch) && pos < max_len) {
            buf[pos++] = (char)ch;
            buf[pos]   = '\0';
        }
    }
}

/* =========================================================
   Kaydedilmemis degisiklik kontrolu
   Donus: 1 = devam et, 0 = iptal
   ========================================================= */
static int check_modified(void) {
    if (!g_modified) return 1;
    return lixui_yesno("Kaydedilmemis Degisiklik",
                        "Degisiklikler kaybolacak. Devam?");
}

/* =========================================================
   Yeni dosya
   ========================================================= */
static void cmd_new(void) {
    if (!check_modified()) return;
    buf_clear();
    full_redraw();
}

/* =========================================================
   Dosya ac
   ========================================================= */
static void cmd_open(void) {
    char path[128];
    if (!check_modified()) return;
    if (!input_dialog(" Dosya Ac ", "Dosya adi:", path, 80, g_filename))
        { full_redraw(); return; }
    if (!file_load(path)) {
        lixui_msgbox(" Hata ", "Dosya acilamadi!",
                     WHITE, RED, BLACK, LIGHTGRAY);
    }
    full_redraw();
}

/* =========================================================
   Kaydet
   ========================================================= */
static void cmd_save(void) {
    char path[128];
    if (g_filename[0]) {
        if (!file_save(g_filename)) {
            lixui_msgbox(" Hata ", "Dosya kaydedilemedi!",
                         WHITE, RED, BLACK, LIGHTGRAY);
        }
        draw_menubar(-1);
        draw_statusbar();
        return;
    }
    /* Dosya adi yok: Farkli Kaydet gibi davran */
    if (!input_dialog(" Farkli Kaydet ", "Dosya adi:", path, 80, ""))
        { full_redraw(); return; }
    if (!file_save(path)) {
        lixui_msgbox(" Hata ", "Dosya kaydedilemedi!",
                     WHITE, RED, BLACK, LIGHTGRAY);
    }
    full_redraw();
}

/* =========================================================
   Farkli Kaydet
   ========================================================= */
static void cmd_saveas(void) {
    char path[128];
    if (!input_dialog(" Farkli Kaydet ", "Dosya adi:", path, 80, g_filename))
        { full_redraw(); return; }
    if (path[0] == '\0') { full_redraw(); return; }
    if (!file_save(path)) {
        lixui_msgbox(" Hata ", "Dosya kaydedilemedi!",
                     WHITE, RED, BLACK, LIGHTGRAY);
    }
    full_redraw();
}

/* =========================================================
   Bul
   ========================================================= */
static void cmd_find(void) {
    char srch[64];
    strncpy(srch, g_find_str, 63);
    if (!input_dialog(" Metin Bul ", "Aranacak metin:", srch, 60, srch))
        { full_redraw(); return; }
    if (srch[0] == '\0') { full_redraw(); return; }
    strncpy(g_find_str, srch, 63);
    g_find_line = g_cur_line;
    g_find_col  = g_cur_col + 1; /* Bir sonrakinden ara */

    /* cmd_find_next ile ara */
    {
        int n, c, slen = strlen(g_find_str);
        int found = 0;

        for (n = g_find_line; n < g_line_count && !found; n++) {
            char *line = g_buf[n];
            int   start = (n == g_find_line) ? g_find_col : 0;
            char *hit;

            hit = strstr(line + start, g_find_str);
            if (hit) {
                g_cur_line  = n;
                g_cur_col   = (int)(hit - line);
                g_find_line = n;
                g_find_col  = g_cur_col;
                found = 1;
            }
        }

        if (!found) {
            lixui_msgbox(" Bul ", "Metin bulunamadi.",
                         WHITE, BLUE, BLACK, LIGHTGRAY);
        }
    }

    update_scroll();
    full_redraw();
}

/* Sonrakini bul (F3) */
static void cmd_find_next(void) {
    int slen, n;
    int found = 0;

    if (g_find_str[0] == '\0') { cmd_find(); return; }

    slen = strlen(g_find_str);
    g_find_col++;

    for (n = g_find_line; n < g_line_count && !found; n++) {
        char *line  = g_buf[n];
        int   start = (n == g_find_line) ? g_find_col : 0;
        char *hit   = strstr(line + start, g_find_str);

        if (hit) {
            g_cur_line  = n;
            g_cur_col   = (int)(hit - line);
            g_find_line = n;
            g_find_col  = g_cur_col;
            found = 1;
        }
    }

    if (!found) {
        /* Basa don */
        g_find_line = 0;
        g_find_col  = 0;
        lixui_msgbox(" Bul ", "Dosyanin sonuna gelindi.",
                     WHITE, BLUE, BLACK, LIGHTGRAY);
    }

    update_scroll();
    full_redraw();
}

/* =========================================================
   Dropdown menüler
   ========================================================= */

/* Dosya menusu */
static void menu_dosya(void) {
    /* ogeler: [etiket, id] */
    typedef struct { const char *lbl; int id; int sep; char key; } MI;
    MI items[] = {
        { " Yeni       Ctrl+N", 1, 0, 'Y' },
        { " Ac...      Ctrl+O", 2, 0, 'A' },
        { " Kaydet     Ctrl+S", 3, 0, 'K' },
        { " Farkli Kaydet... ", 4, 0, 'F' },
        { "-", 0, 1, 0 },
        { " Cikmak           ", 5, 0, 'C' },
        { NULL, 0, 0, 0 }
    };
    int count = 6;
    int sel   = 0;
    int mx = 1, my = 2, mw = 22;
    int ch, j, act = 0;

    #define DD_DRAW() do { \
        lixui_draw_window(mx, my, mw, count + 2, NULL, \
                          WHITE, BLUE, BLACK, LIGHTGRAY); \
        for (j = 0; j < count; j++) { \
            if (items[j].sep) { \
                lixui_setcolor(DARKGRAY, LIGHTGRAY); \
                lixui_hline(mx+1, my+1+j, mw-2, SBOX_H); \
            } else if (j == sel) { \
                lixui_setcolor(WHITE, BLUE); \
                gotoxy(mx+1, my+1+j); \
                cprintf("%-*s", mw-2, items[j].lbl); \
            } else { \
                lixui_setcolor(BLACK, LIGHTGRAY); \
                gotoxy(mx+1, my+1+j); \
                cprintf("%-*s", mw-2, items[j].lbl); \
            } \
        } \
    } while(0)

    draw_menubar(0);
    for (;;) {
        DD_DRAW();
        ch = getch();
        if (ch == 0) {
            ch = getch();
            if (ch == KEY_UP) { do { sel=(sel-1+count)%count; } while(items[sel].sep); }
            else if (ch == KEY_DOWN) { do { sel=(sel+1)%count; } while(items[sel].sep); }
            else if (ch == KEY_RIGHT) { draw_menubar(1); menu_dosya(); goto done; }
        } else if (ch == 13) {
            act = items[sel].id; break;
        } else if (ch == 27) { break; }
        else {
            char uc = (ch >= 'a' && ch <= 'z') ? ch-32 : ch;
            for (j = 0; j < count; j++) {
                if (!items[j].sep && items[j].key == uc) { act = items[j].id; goto exec; }
            }
        }
    }
    exec:
    #undef DD_DRAW
    done:
    draw_menubar(-1);
    full_redraw();

    switch (act) {
        case 1: cmd_new();    break;
        case 2: cmd_open();   break;
        case 3: cmd_save();   break;
        case 4: cmd_saveas(); break;
        case 5:
            if (check_modified()) g_running = 0;
            else full_redraw();
            break;
    }
}

/* Duzen menusu */
static void menu_duzen(void) {
    typedef struct { const char *lbl; int id; int sep; char key; } MI;
    MI items[] = {
        { " Bul...     Ctrl+F", 1, 0, 'B' },
        { " Sonraki    F3    ", 2, 0, 'S' },
        { "-", 0, 1, 0 },
        { " Basa Git  Ctrl+Hm", 3, 0, 'G' },
        { " Sona Git  Ctrl+En", 4, 0, 'N' },
        { NULL, 0, 0, 0 }
    };
    int count = 5;
    int sel = 0;
    int mx = 10, my = 2, mw = 22;
    int ch, j, act = 0;

    #define DD_DRAW2() do { \
        lixui_draw_window(mx, my, mw, count + 2, NULL, \
                          WHITE, BLUE, BLACK, LIGHTGRAY); \
        for (j = 0; j < count; j++) { \
            if (items[j].sep) { \
                lixui_setcolor(DARKGRAY, LIGHTGRAY); \
                lixui_hline(mx+1, my+1+j, mw-2, SBOX_H); \
            } else if (j == sel) { \
                lixui_setcolor(WHITE, BLUE); \
                gotoxy(mx+1, my+1+j); \
                cprintf("%-*s", mw-2, items[j].lbl); \
            } else { \
                lixui_setcolor(BLACK, LIGHTGRAY); \
                gotoxy(mx+1, my+1+j); \
                cprintf("%-*s", mw-2, items[j].lbl); \
            } \
        } \
    } while(0)

    draw_menubar(1);
    for (;;) {
        DD_DRAW2();
        ch = getch();
        if (ch == 0) {
            ch = getch();
            if (ch == KEY_UP)   { do { sel=(sel-1+count)%count; } while(items[sel].sep); }
            else if (ch == KEY_DOWN) { do { sel=(sel+1)%count; } while(items[sel].sep); }
        } else if (ch == 13) {
            act = items[sel].id; break;
        } else if (ch == 27) { break; }
        else {
            char uc = (ch >= 'a' && ch <= 'z') ? ch-32 : ch;
            for (j = 0; j < count; j++) {
                if (!items[j].sep && items[j].key == uc) { act = items[j].id; goto exec2; }
            }
        }
    }
    exec2:
    #undef DD_DRAW2
    draw_menubar(-1);
    full_redraw();

    switch (act) {
        case 1: cmd_find();      break;
        case 2: cmd_find_next(); break;
        case 3:
            g_cur_line = 0; g_cur_col = 0;
            update_scroll(); full_redraw(); break;
        case 4:
            g_cur_line = g_line_count - 1;
            g_cur_col  = strlen(g_buf[g_cur_line]);
            update_scroll(); full_redraw(); break;
    }
}

/* Yardim menusu */
static void menu_yardim(void) {
    int x = 19, y = 5, w = 52, h = 14;
    int j;

    lixui_draw_window(x, y, w, h,
        " Notepad Yardim ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);

    #define HL(row, txt) do { gotoxy(x+2, y+(row)); cputs(txt); } while(0)
    HL(2,  "DOSYA ISLEMLERI:");
    HL(3,  "  Ctrl+N  Yeni dosya");
    HL(4,  "  Ctrl+O  Dosya ac");
    HL(5,  "  Ctrl+S  Kaydet");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x+1, y+6, w-2, SBOX_H);
    lixui_setcolor(BLACK, LIGHTGRAY);

    HL(7,  "HAREKET:");
    HL(8,  "  Ok tus. / Home / End / PgUp / PgDn");
    HL(9,  "  Ctrl+Home=Basa  Ctrl+End=Sona");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x+1, y+10, w-2, SBOX_H);
    lixui_setcolor(BLACK, LIGHTGRAY);

    HL(11, "  Ctrl+F / F3   Bul / Sonraki");
    HL(12, "  Ins           Ekle/Uzerine yaz");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_draw_button(x + (w-10)/2, y+h-2, "   Tamam  ", 1);

    #undef HL
    lixui_statusbar("ENTER ile kapat");
    while (getch() != 13);
    full_redraw();
}

/* =========================================================
   Ana event dongusu
   ========================================================= */
static void main_loop(void) {
    int ch;

    while (g_running) {
        _setcursortype(_NORMALCURSOR);
        update_scroll();
        place_cursor();
        draw_statusbar();

        if (!kbhit()) continue;

        ch = getch();

        /* ---- Extended tus ---- */
        if (ch == 0) {
            ch = getch();

            switch (ch) {
                /* Yon tuslari */
                case KEY_UP:
                    if (g_cur_line > 0) {
                        g_cur_line--;
                        clamp_cursor();
                    }
                    break;
                case KEY_DOWN:
                    if (g_cur_line < g_line_count - 1) {
                        g_cur_line++;
                        clamp_cursor();
                    }
                    break;
                case KEY_LEFT:
                    if (g_cur_col > 0) {
                        g_cur_col--;
                    } else if (g_cur_line > 0) {
                        g_cur_line--;
                        g_cur_col = strlen(g_buf[g_cur_line]);
                    }
                    break;
                case KEY_RIGHT:
                    if (g_cur_col < (int)strlen(g_buf[g_cur_line])) {
                        g_cur_col++;
                    } else if (g_cur_line < g_line_count - 1) {
                        g_cur_line++;
                        g_cur_col = 0;
                    }
                    break;

                /* Home / End */
                case KEY_HOME:
                    g_cur_col = 0;
                    break;
                case KEY_END:
                    g_cur_col = strlen(g_buf[g_cur_line]);
                    break;

                /* PgUp / PgDn */
                case KEY_PGUP:
                    g_cur_line -= EDIT_ROWS;
                    if (g_cur_line < 0) g_cur_line = 0;
                    clamp_cursor();
                    update_scroll();
                    draw_editarea();
                    break;
                case KEY_PGDN:
                    g_cur_line += EDIT_ROWS;
                    if (g_cur_line >= g_line_count)
                        g_cur_line = g_line_count - 1;
                    clamp_cursor();
                    update_scroll();
                    draw_editarea();
                    break;

                /* Ctrl+Home */
                case KEY_CTRL_HOME:
                    g_cur_line = 0;
                    g_cur_col  = 0;
                    update_scroll();
                    draw_editarea();
                    break;

                /* Ctrl+End */
                case KEY_CTRL_END:
                    g_cur_line = g_line_count - 1;
                    g_cur_col  = strlen(g_buf[g_cur_line]);
                    update_scroll();
                    draw_editarea();
                    break;

                /* Ctrl+Sol - kelime basi */
                case KEY_CTRL_LEFT:
                    while (g_cur_col > 0 &&
                           !isspace((unsigned char)g_buf[g_cur_line][g_cur_col-1]))
                        g_cur_col--;
                    while (g_cur_col > 0 &&
                           isspace((unsigned char)g_buf[g_cur_line][g_cur_col-1]))
                        g_cur_col--;
                    break;

                /* Ctrl+Sag - kelime sonu */
                case KEY_CTRL_RIGHT:
                    {
                        int llen = strlen(g_buf[g_cur_line]);
                        while (g_cur_col < llen &&
                               isspace((unsigned char)g_buf[g_cur_line][g_cur_col]))
                            g_cur_col++;
                        while (g_cur_col < llen &&
                               !isspace((unsigned char)g_buf[g_cur_line][g_cur_col]))
                            g_cur_col++;
                    }
                    break;

                /* Delete */
                case KEY_DEL:
                    buf_delete();
                    update_scroll();
                    draw_editarea();
                    break;

                /* Insert - mod degistir */
                case KEY_INS:
                    g_insert_mode = !g_insert_mode;
                    break;

                /* F2 = Kaydet */
                case KEY_F2:
                    cmd_save();
                    break;

                /* F3 = Sonraki bul */
                case KEY_F3:
                    cmd_find_next();
                    break;

                /* F5 = Bul */
                case KEY_F5:
                    cmd_find();
                    break;

                /* ALT+F = Dosya menusu */
                case KEY_ALT_F:
                    _setcursortype(_NOCURSOR);
                    menu_dosya();
                    break;

                /* ALT+D = Duzen menusu */
                case KEY_ALT_D:
                    _setcursortype(_NOCURSOR);
                    menu_duzen();
                    break;

                /* ALT+Y = Yardim menusu */
                case KEY_ALT_Y:
                    _setcursortype(_NOCURSOR);
                    menu_yardim();
                    break;

                /* F1 = Yardim */
                case 59:
                    _setcursortype(_NOCURSOR);
                    menu_yardim();
                    break;
            }

        /* ---- Normal tus ---- */
        } else {

            switch (ch) {
                /* Backspace */
                case 8:
                    buf_backspace();
                    update_scroll();
                    draw_editarea();
                    break;

                /* Enter */
                case 13:
                    buf_enter();
                    update_scroll();
                    draw_editarea();
                    break;

                /* Tab -> 4 bosluk ekle */
                case 9:
                    buf_insert_char(' ');
                    buf_insert_char(' ');
                    buf_insert_char(' ');
                    buf_insert_char(' ');
                    update_scroll();
                    draw_editarea();
                    break;

                /* ESC - menüye odaklan (F10 gibi) */
                case 27:
                    _setcursortype(_NOCURSOR);
                    draw_menubar(0);
                    {
                        int k = getch();
                        if (k == 0) k = getch();
                        if (k == 13 || k == KEY_DOWN)
                            menu_dosya();
                        else
                            { draw_menubar(-1); full_redraw(); }
                    }
                    break;

                /* Ctrl+N */
                case KEY_CTRL_N:
                    cmd_new();
                    break;

                /* Ctrl+O */
                case KEY_CTRL_O:
                    cmd_open();
                    break;

                /* Ctrl+S */
                case KEY_CTRL_S:
                    cmd_save();
                    break;

                /* Ctrl+F */
                case KEY_CTRL_F:
                    cmd_find();
                    break;

                /* Yazilabilir karakter */
                default:
                    if (ch >= 32 && ch <= 126) {
                        buf_insert_char((char)ch);
                        update_scroll();
                        redraw_line(g_cur_line);
                    }
                    break;
            }
        }
    } /* while running */
}

/* =========================================================
   main()
   ========================================================= */
int main(int argc, char *argv[]) {
    /* Tamponu sifirla */
    buf_clear();

    /* Komut satirindan dosya adi geldi mi? */
    if (argc > 1) {
        if (!file_load(argv[1])) {
            /* Dosya yok: yeni dosya olarak ac, adi ata */
            strncpy(g_filename, argv[1], 127);
        }
    }

    /* Ekrani hazirla */
    _setcursortype(_NOCURSOR);
    lixui_setcolor(CLR_EDIT_FG, CLR_EDIT_BG);
    clrscr();

    /* Ilk cizim */
    full_redraw();

    /* Ana dongu */
    main_loop();

    /* Cikis */
    _setcursortype(_NORMALCURSOR);
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();

    return 0;
}
