/* =========================================================
   LIXOS.C  -  LixOS Masaustu Kabugu (Desktop Shell)
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   Derleme: TCC -ml LIXOS.C
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "LIXUI.H"

/* ---- Surum bilgisi ---- */
#define LIXOS_VER       "1.00"
#define LIXOS_BUILD     "260426"     /* GGAAYYY */
#define LIXOS_AUTHOR    "LixOS Projesi"

/* ---- Ekran sabitleri ---- */
#define MENU_ROW        1
#define DESKTOP_TOP     3
#define DESKTOP_BOT     24
#define DESKTOP_BG      CYAN
#define MENUBAR_FG      WHITE
#define MENUBAR_BG      BLUE

/* ---- Menu indeksleri ---- */
#define MENU_NONE       -1
#define MENU_LIXOS       0
#define MENU_DOSYALAR    1
#define MENU_PROGRAMLAR  2
#define MENU_SISTEM      3
#define MENU_YARDIM      4
#define MENU_COUNT       5

/* ---- Ikon sayisi ---- */
#define ICON_COUNT      8
#define ICON_NONE       -1

/* ---- Mouse sabitleri ---- */
#define MOUSE_LEFT      1
#define MOUSE_RIGHT     2

/* =========================================================
   Veri yapilari
   ========================================================= */

typedef struct {
    const char *label;  /* Gorunen metin           */
    char        hot;    /* ALT+? tus kodu (buyuk)  */
    int         col;    /* Menubar sutun konumu     */
} MenuBarItem;

typedef struct {
    const char *label;  /* Menü öğesi metni  */
    char        key;    /* Hizli erisim tusu */
    int         sep;    /* 1 ise ayrac cizgi */
    int         id;     /* Eylem ID          */
} MenuItem;

typedef struct {
    const char *label;  /* Ikon alti yazi  */
    const char *exe;    /* Calistirilacak   */
    char        sym;    /* Ikon sembolü     */
    int         col;    /* Masaustu x konum */
    int         row;    /* Masaustu y konum */
} ProgIcon;

/* =========================================================
   Eylem kimlikleri (action IDs)
   ========================================================= */
#define ACT_NONE          0
#define ACT_EXIT          1
#define ACT_ABOUT         2
#define ACT_DOS_PROMPT    3
#define ACT_LAUNCH_NP     4   /* Notepad    */
#define ACT_LAUNCH_VER    5   /* LixVer     */
#define ACT_LAUNCH_SETUP  6   /* Setup      */
#define ACT_RESTART       7
#define ACT_SHUTDOWN      8
#define ACT_HELP          9
#define ACT_DATETIME      10
#define ACT_LAUNCH_FILEMAN 11
#define ACT_LAUNCH_CALC    12
#define ACT_LAUNCH_SNAKE   13
#define ACT_LAUNCH_CAL     14
#define ACT_LAUNCH_CLOCK   15

/* =========================================================
   Menu tanimlamalari
   ========================================================= */
static MenuBarItem g_menubar[MENU_COUNT] = {
    { " \x1e LixOS ",  'L',  1 },   /* 0 */
    { " Dosyalar ",    'D', 10 },   /* 1 */
    { " Programlar ",  'P', 21 },   /* 2 */
    { " Sistem ",      'S', 34 },   /* 3 */
    { " Yardim ",      'Y', 43 }    /* 4 */
};

/* LixOS menu */
static MenuItem g_menu0[] = {
    { "LixOS Hakkinda",   'H', 0, ACT_ABOUT   },
    { "---------------",  0,   1, ACT_NONE    },
    { "Kapat         ",   'K', 0, ACT_EXIT    },
    { NULL, 0, 0, 0 }
};

/* Dosyalar menu */
static MenuItem g_menu1[] = {
    { "Dosya Yoneticisi", 'Y', 0, ACT_LAUNCH_FILEMAN },
    { "DOS Istemi    ",   'D', 0, ACT_DOS_PROMPT },
    { "---------------",  0,   1, ACT_NONE       },
    { "Cikmak        ",   'C', 0, ACT_EXIT        },
    { NULL, 0, 0, 0 }
};

/* Programlar menu */
static MenuItem g_menu2[] = {
    { "Notepad       ",   'N', 0, ACT_LAUNCH_NP    },
    { "LixVer        ",   'V', 0, ACT_LAUNCH_VER   },
    { "Setup         ",   'S', 0, ACT_LAUNCH_SETUP },
    { "Dosya Yonetimi",   'Y', 0, ACT_LAUNCH_FILEMAN },
    { "Hesap Makinesi",   'H', 0, ACT_LAUNCH_CALC    },
    { "Takvim        ",   'T', 0, ACT_LAUNCH_CAL     },
    { "Saat          ",   'A', 0, ACT_LAUNCH_CLOCK   },
    { "Yilan Oyunu   ",   'O', 0, ACT_LAUNCH_SNAKE   },
    { "---------------",  0,   1, ACT_NONE         },
    { "DOS Istemi    ",   'D', 0, ACT_DOS_PROMPT   },
    { NULL, 0, 0, 0 }
};

/* Sistem menu */
static MenuItem g_menu3[] = {
    { "Tarih/Saat    ",   'T', 0, ACT_DATETIME },
    { "---------------",  0,   1, ACT_NONE     },
    { "Yeniden Baslat",   'Y', 0, ACT_RESTART  },
    { "Bilgisayari Kapat",'B', 0, ACT_SHUTDOWN },
    { NULL, 0, 0, 0 }
};

/* Yardim menu */
static MenuItem g_menu4[] = {
    { "Yardim Icerigi",   'Y', 0, ACT_HELP    },
    { "---------------",  0,   1, ACT_NONE    },
    { "LixOS Hakkinda",   'H', 0, ACT_ABOUT   },
    { NULL, 0, 0, 0 }
};

static MenuItem *g_menus[MENU_COUNT] = {
    g_menu0, g_menu1, g_menu2, g_menu3, g_menu4
};

/* =========================================================
   Program ikonlari
   ========================================================= */
static ProgIcon g_icons[ICON_COUNT] = {
    { "Notepad",    "NOTEPAD.EXE", 'N', 14, 9  },
    { "LixVer",     "LIXVER.EXE",  'V', 26, 9  },
    { "Setup",      "SETUP.EXE",   'S', 38, 9  },
    { "Dosyalar",   "FILEMAN.EXE", 'F', 50, 9  },
    { "Hesap Mak",  "CALC.EXE",    'C', 14, 15 },
    { "Takvim",     "CALENDAR.EXE",'T', 26, 15 },
    { "Saat",       "CLOCK.EXE",   'A', 38, 15 },
    { "Yilan Oyunu", "SNAKE.EXE",  'O', 50, 15 }
};

/* =========================================================
   Global durum
   ========================================================= */
static int  g_menu_open    = MENU_NONE;  /* Acik menu indeksi  */
static int  g_menu_sel     = 0;          /* Secili menu oğesi  */
static int  g_icon_sel     = ICON_NONE;  /* Secili ikon        */
static int  g_mouse_ok     = 0;          /* Mouse var mi?      */
static int  g_running      = 1;          /* Ana dongu aktif mi */
static char g_status[81]   = "";         /* Durum cubuğu       */

/* =========================================================
   BIOS saat okuma  (INT 1Ah, AH=02h)
   ========================================================= */
static void bios_get_time(int *h, int *m, int *s) {
    union REGS r;
    r.h.ah = 0x02;
    int86(0x1A, &r, &r);
    *h = ((r.h.ch >> 4) & 0x0F) * 10 + (r.h.ch & 0x0F); /* BCD */
    *m = ((r.h.cl >> 4) & 0x0F) * 10 + (r.h.cl & 0x0F);
    *s = ((r.h.dh >> 4) & 0x0F) * 10 + (r.h.dh & 0x0F);
}

/* =========================================================
   Mouse init  (INT 33h)
   ========================================================= */
static int mouse_init(void) {
    union REGS r;
    r.x.ax = 0x0000;
    int86(0x33, &r, &r);
    return (r.x.ax == 0xFFFF) ? 1 : 0;
}

static void mouse_show(void) {
    union REGS r;
    r.x.ax = 0x0001;
    int86(0x33, &r, &r);
}

static void mouse_hide(void) {
    union REGS r;
    r.x.ax = 0x0002;
    int86(0x33, &r, &r);
}

/* col ve row: 1-bazli metin koordinati */
static void mouse_get(int *col, int *row, int *btns) {
    union REGS r;
    r.x.ax = 0x0003;
    int86(0x33, &r, &r);
    *btns = r.x.bx;
    *col  = r.x.cx / 8 + 1;
    *row  = r.x.dx / 8 + 1;
}

/* =========================================================
   Menü cubugu ciz
   ========================================================= */
static void draw_menubar(void) {
    int i;
    lixui_setcolor(MENUBAR_FG, MENUBAR_BG);
    gotoxy(1, MENU_ROW);
    for (i = 0; i < SCREEN_W; i++) putch(' ');

    for (i = 0; i < MENU_COUNT; i++) {
        if (i == g_menu_open) {
            lixui_setcolor(BLACK, LIGHTGRAY);  /* Secili menu vurgu */
        } else {
            lixui_setcolor(MENUBAR_FG, MENUBAR_BG);
        }
        gotoxy(g_menubar[i].col, MENU_ROW);
        cputs(g_menubar[i].label);
    }

    /* Saat - sag taraf */
    {
        int h, m, s;
        bios_get_time(&h, &m, &s);
        lixui_setcolor(YELLOW, MENUBAR_BG);
        gotoxy(70, MENU_ROW);
        cprintf("%02d:%02d:%02d", h, m, s);
    }

    /* Ayrac cizgi */
    lixui_setcolor(DARKGRAY, MENUBAR_BG);
    gotoxy(1, MENU_ROW + 1);
    for (i = 0; i < SCREEN_W; i++) putch(BOX_H);
}

/* =========================================================
   Masaustu arkaplanini ciz
   ========================================================= */
static void draw_desktop_bg(void) {
    int y;
    lixui_setcolor(LIGHTGRAY, CYAN);
    for (y = DESKTOP_TOP; y <= DESKTOP_BOT; y++) {
        gotoxy(1, y);
        clreol();
    }
}

/* =========================================================
   Tek bir ikon ciz
   col, row: metin koordinati (ikon sol ust kosesi)
   sel: 1 ise secili (vurgu)
   ========================================================= */
static void draw_icon(int idx, int sel) {
    ProgIcon *ic = &g_icons[idx];
    int x = ic->col;
    int y = ic->row;
    int llen = strlen(ic->label);
    int lx   = x + (8 - llen) / 2;
    int i;

    if (sel) {
        lixui_setcolor(WHITE,   BLUE);
    } else {
        lixui_setcolor(BLACK,   CYAN);
    }

    /* Ikon cercevesi 8x4 */
    gotoxy(x, y);
    putch(SBOX_TL);
    for (i = 0; i < 6; i++) putch(SBOX_H);
    putch(SBOX_TR);

    gotoxy(x, y + 1);
    putch(SBOX_V);
    gotoxy(x + 1, y + 1);
    for (i = 0; i < 6; i++) putch(' ');
    gotoxy(x + 7, y + 1); putch(SBOX_V);

    /* Sembol - ortalanmis */
    lixui_setcolor(sel ? YELLOW : BLUE, sel ? BLUE : CYAN);
    gotoxy(x + 3, y + 1);
    cprintf("[%c]", ic->sym);

    lixui_setcolor(sel ? WHITE : BLACK, sel ? BLUE : CYAN);
    gotoxy(x, y + 2);
    putch(SBOX_V);
    for (i = 0; i < 6; i++) putch(' ');
    gotoxy(x + 7, y + 2); putch(SBOX_V);

    gotoxy(x, y + 3);
    putch(SBOX_BL);
    for (i = 0; i < 6; i++) putch(SBOX_H);
    putch(SBOX_BR);

    /* Etiket - ikon alti */
    if (sel) {
        lixui_setcolor(WHITE, BLUE);
    } else {
        lixui_setcolor(BLACK, CYAN);
    }
    gotoxy(lx < x ? x : lx, y + 4);
    cprintf("%s", ic->label);
}

/* =========================================================
   Tum ikonlari ciz
   ========================================================= */
static void draw_all_icons(void) {
    int i;
    for (i = 0; i < ICON_COUNT; i++) {
        draw_icon(i, i == g_icon_sel);
    }
}

/* =========================================================
   Program grubu penceresi ciz (arka plan)
   ========================================================= */
static void draw_proggroup(void) {
    int wx = 10, wy = 7, ww = 60, wh = 12;
    int i;

    lixui_draw_window(wx, wy, ww, wh,
        " Program Grubu - LixOS v" LIXOS_VER " ",
        YELLOW, BLUE,
        BLACK,  LIGHTGRAY);

    /* Pencere kontrol dugmeleri (sol ust) */
    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(wx + 1, wy); cprintf("[%c]", (char)254);  /* minimize */

    /* Ikon kilavuzu - alt bilgi */
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(wx + 1, wy + wh - 3, ww - 2, SBOX_H);
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(wx + 2, wy + wh - 2);
    cprintf("TAB/Ok: Sec   ENTER: Ac   1-4: Direk Ac");
}

/* =========================================================
   Durum cubuğunu guncelle
   ========================================================= */
static void update_status(const char *msg) {
    strncpy(g_status, msg, 79);
    g_status[79] = '\0';
    lixui_statusbar(g_status);
}

/* =========================================================
   Tam masaustunu yeniden ciz
   ========================================================= */
static void redraw_all(void) {
    if (g_mouse_ok) mouse_hide();

    draw_desktop_bg();
    draw_menubar();
    draw_proggroup();
    draw_all_icons();
    update_status("1=Notepad  2=LixVer  3=Setup  4=DOS  ALT=Menu  F1=Yardim  F3=Cik");

    if (g_mouse_ok) mouse_show();
}

/* =========================================================
   Dropdown menu ac ve oge sec
   Donus: secilen ACT_ degeri, MENU_NONE ise kapandi
   ========================================================= */
static int open_dropdown(int menu_idx) {
    MenuItem *items = g_menus[menu_idx];
    int count = 0;
    int sel   = 0;
    int i, ch;
    int mx, my, mw;
    int item_start;

    /* Oge sayisini say */
    while (items[count].label != NULL) count++;

    /* Dropdown boyutu */
    mw        = 20;
    mx        = g_menubar[menu_idx].col;
    my        = MENU_ROW + 1;
    item_start = my + 1;

    /* Ilk gecerli (ayrac olmayan) ogeye git */
    while (sel < count && items[sel].sep) sel++;

    /* Menü ciz fonksiyonu - lambda yok, inline yap */
    #define DRAW_DROPDOWN() do { \
        lixui_draw_window(mx, my, mw, count + 2, NULL, \
            WHITE, BLUE, BLACK, LIGHTGRAY); \
        for (i = 0; i < count; i++) { \
            if (items[i].sep) { \
                lixui_setcolor(DARKGRAY, LIGHTGRAY); \
                lixui_hline(mx + 1, item_start + i, mw - 2, SBOX_H); \
            } else if (i == sel) { \
                lixui_setcolor(WHITE, BLUE); \
                gotoxy(mx + 1, item_start + i); \
                cprintf("%-*s", mw - 2, items[i].label); \
            } else { \
                lixui_setcolor(BLACK, LIGHTGRAY); \
                gotoxy(mx + 1, item_start + i); \
                cprintf("%-*s", mw - 2, items[i].label); \
            } \
        } \
    } while(0)

    DRAW_DROPDOWN();
    draw_menubar(); /* Secili menu vurgusu */

    for (;;) {
        DRAW_DROPDOWN();

        ch = getch();
        if (ch == 0) {
            ch = getch();
            if (ch == 72) {  /* Yukari ok */
                do {
                    sel = (sel - 1 + count) % count;
                } while (items[sel].sep);
            } else if (ch == 80) {  /* Asagi ok */
                do {
                    sel = (sel + 1) % count;
                } while (items[sel].sep);
            } else if (ch == 75) {  /* Sol ok - onceki menu */
                g_menu_open = (menu_idx - 1 + MENU_COUNT) % MENU_COUNT;
                return MENU_NONE;
            } else if (ch == 77) {  /* Sag ok - sonraki menu */
                g_menu_open = (menu_idx + 1) % MENU_COUNT;
                return MENU_NONE;
            } else if (ch == 59) {  /* F1 */
                return ACT_HELP;
            } else if (ch == 61) {  /* F3 */
                return ACT_EXIT;
            }
        } else if (ch == 13) {  /* ENTER */
            return items[sel].id;
        } else if (ch == 27) {  /* ESC */
            g_menu_open = MENU_NONE;
            return MENU_NONE;
        } else {
            /* Hizli erisim tusu */
            char uc = (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
            for (i = 0; i < count; i++) {
                if (!items[i].sep && items[i].key == uc) {
                    return items[i].id;
                }
            }
        }
    }

    #undef DRAW_DROPDOWN
}

/* =========================================================
   Hakkinda diyalogu
   ========================================================= */
static void show_about(void) {
    int x = 18, y = 7, w = 44, h = 12;
    int i;

    lixui_draw_window(x, y, w, h,
        " LixOS Hakkinda ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    /* Logo */
    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(x + 2, y + 2);
    cprintf("  %c%c%c  LixOS v%s", BLOCK_FULL, BLOCK_MED, BLOCK_FULL,
            LIXOS_VER);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(x + 2, y + 3);
    cprintf("  %c%c%c  Build %s", BLOCK_LIGHT, BLOCK_LIGHT, BLOCK_LIGHT,
            LIXOS_BUILD);

    lixui_setcolor(BLACK, LIGHTGRAY);
    lixui_hline(x + 1, y + 4, w - 2, SBOX_H);

    gotoxy(x + 3, y + 5);
    cprintf("MS-DOS 6.22 uzerinde calisir");
    gotoxy(x + 3, y + 6);
    cprintf("Turbo C 2.0 ile derlenmistir");
    gotoxy(x + 3, y + 7);
    cprintf("Gelistirici: %s", LIXOS_AUTHOR);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 9, w - 2, SBOX_H);
    lixui_draw_button(x + (w - 8) / 2, y + 10, "  Tamam  ", 1);

    update_status("LixOS v" LIXOS_VER " - ENTER ile kapat");
    while (getch() != 13);
}

/* =========================================================
   Yardim diyalogu
   ========================================================= */
static void show_help(void) {
    int x = 10, y = 5, w = 60, h = 14;

    lixui_draw_window(x, y, w, h,
        " LixOS Yardim ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);

    gotoxy(x + 3, y + 2);  cprintf("KLAVYE KISAYOLLARI:");
    gotoxy(x + 3, y + 3);  cprintf("  1-4        : Programi dogrudan ac");
    gotoxy(x + 3, y + 4);  cprintf("  TAB / Oklar: Ikon sec");
    gotoxy(x + 3, y + 5);  cprintf("  ENTER      : Secili ikonu ac");
    gotoxy(x + 3, y + 6);  cprintf("  ALT+L/D/P/S/Y : Menuyu ac");
    gotoxy(x + 3, y + 7);  cprintf("  F1         : Bu yardim ekrani");
    gotoxy(x + 3, y + 8);  cprintf("  F3         : LixOS'tan cik");
    gotoxy(x + 3, y + 9);  cprintf("  ESC        : Menuyu kapat");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 11, w - 2, SBOX_H);
    lixui_draw_button(x + (w - 10) / 2, y + 12, "   Tamam   ", 1);

    update_status("F1=Yardim  ENTER=Kapat");
    while (getch() != 13);
}

/* =========================================================
   Tarih/Saat ekrani
   ========================================================= */
static void show_datetime(void) {
    int x = 22, y = 9, w = 36, h = 8;
    int hh, mm, ss;

    lixui_draw_window(x, y, w, h,
        " Tarih / Saat ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    bios_get_time(&hh, &mm, &ss);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 2);
    cprintf("Sistem saati:");

    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(x + 3, y + 3);
    cprintf("  %02d:%02d:%02d", hh, mm, ss);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 5, w - 2, SBOX_H);
    lixui_draw_button(x + (w - 8) / 2, y + 6, "  Tamam  ", 1);

    while (getch() != 13);
}

/* =========================================================
   Program calistir
   exe: calistirilacak dosya adi
   ========================================================= */
static void launch_program(const char *exe) {
    char cmd[64];

    /* DOS ekranina gec */
    _setcursortype(_NORMALCURSOR);
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();

    printf("LixOS: %s baslatiliyor...\n\n", exe);

    sprintf(cmd, "%s", exe);
    system(cmd);

    /* Geri donus */
    _setcursortype(_NOCURSOR);
    redraw_all();
}

/* =========================================================
   DOS istemine gec
   ========================================================= */
static void goto_dos(void) {
    _setcursortype(_NORMALCURSOR);
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    printf("LixOS'tan gecici olarak cikiliyor.\n");
    printf("Geri donmek icin: EXIT yazin\n\n");
    system("COMMAND.COM");
    _setcursortype(_NOCURSOR);
    redraw_all();
}

/* =========================================================
   Eylem isleme (action dispatcher)
   Donus: 0 = devam, 1 = cikis
   ========================================================= */
static int dispatch_action(int act) {
    switch (act) {
        case ACT_EXIT:
            if (lixui_yesno("LixOS'tan Cik",
                    "LixOS'tan cikmak istiyor musunuz?")) {
                return 1;
            }
            redraw_all();
            break;

        case ACT_ABOUT:
            show_about();
            redraw_all();
            break;

        case ACT_HELP:
            show_help();
            redraw_all();
            break;

        case ACT_DATETIME:
            show_datetime();
            redraw_all();
            break;

        case ACT_DOS_PROMPT:
            goto_dos();
            break;

        case ACT_LAUNCH_NP:
            launch_program("NOTEPAD.EXE");
            break;

        case ACT_LAUNCH_VER:
            launch_program("LIXVER.EXE");
            break;

        case ACT_LAUNCH_SETUP:
            if (lixui_yesno("Setup Baslat",
                    "Kurulum programi baslatilsin mi?")) {
                launch_program("SETUP.EXE");
            } else {
                redraw_all();
            }
            break;

        case ACT_LAUNCH_FILEMAN: launch_program("FILEMAN.EXE"); break;
        case ACT_LAUNCH_CALC:    launch_program("CALC.EXE");    break;
        case ACT_LAUNCH_SNAKE:   launch_program("SNAKE.EXE");   break;
        case ACT_LAUNCH_CAL:     launch_program("CALENDAR.EXE");break;
        case ACT_LAUNCH_CLOCK:   launch_program("CLOCK.EXE");   break;

        case ACT_RESTART: {
            union REGS r;
            lixui_setcolor(LIGHTGRAY, BLACK);
            clrscr();
            printf("Yeniden baslatiliyor...\n");
            delay(800);
            int86(0x19, &r, &r);
            break;
        }

        case ACT_SHUTDOWN:
            if (lixui_yesno("Bilgisayari Kapat",
                    "Sistemi kapatmak istiyor musunuz?")) {
                lixui_setcolor(LIGHTGRAY, BLACK);
                clrscr();
                printf("Guc kapatilabilir.\n");
                printf("LixOS guvende kapatildi.\n");
                return 1;
            }
            redraw_all();
            break;
    }
    return 0;
}

/* =========================================================
   ALT tus -> hangi menu?
   ========================================================= */
static int alt_to_menu(int scan) {
    /* scan: ALT+harf skan kodu (DOS extended) */
    /* ALT+L=0x26, ALT+D=0x20, ALT+P=0x19, ALT+S=0x1F, ALT+Y=0x15 */
    switch (scan) {
        case 0x26: return MENU_LIXOS;
        case 0x20: return MENU_DOSYALAR;
        case 0x19: return MENU_PROGRAMLAR;
        case 0x1F: return MENU_SISTEM;
        case 0x15: return MENU_YARDIM;
    }
    return MENU_NONE;
}

/* =========================================================
   Saat guncelleme (saati yeniden ciz, menu yoksa)
   ========================================================= */
static void refresh_clock(void) {
    int hh, mm, ss;
    if (g_menu_open != MENU_NONE) return;
    bios_get_time(&hh, &mm, &ss);
    lixui_setcolor(YELLOW, MENUBAR_BG);
    if (g_mouse_ok) mouse_hide();
    gotoxy(70, MENU_ROW);
    cprintf("%02d:%02d:%02d", hh, mm, ss);
    if (g_mouse_ok) mouse_show();
}

/* =========================================================
   Mouse tiklama: ikon mi, menubar mi?
   ========================================================= */
static int handle_mouse_click(int col, int row, int btns) {
    int i;

    /* Sol tik degil */
    if (!(btns & MOUSE_LEFT)) return ACT_NONE;

    /* Menubar kontrolu */
    if (row == MENU_ROW) {
        for (i = 0; i < MENU_COUNT; i++) {
            int mc  = g_menubar[i].col;
            int mlen = strlen(g_menubar[i].label);
            if (col >= mc && col < mc + mlen) {
                g_menu_open = i;
                return MENU_NONE;
            }
        }
    }

    /* Ikon tiklamalari */
    for (i = 0; i < ICON_COUNT; i++) {
        int ix = g_icons[i].col;
        int iy = g_icons[i].row;
        if (col >= ix && col <= ix + 7 &&
            row >= iy && row <= iy + 4) {
            if (g_icon_sel == i) {
                /* Cift tik gibi davran: ac */
                int acts[] = {
                    ACT_LAUNCH_NP, ACT_LAUNCH_VER,
                    ACT_LAUNCH_SETUP, ACT_LAUNCH_FILEMAN,
                    ACT_LAUNCH_CALC, ACT_LAUNCH_CAL,
                    ACT_LAUNCH_CLOCK, ACT_LAUNCH_SNAKE
                };
                return acts[i];
            }
            g_icon_sel = i;
            draw_all_icons();
            return ACT_NONE;
        }
    }

    return ACT_NONE;
}

/* =========================================================
   Ana event dongusu
   ========================================================= */
static void main_loop(void) {
    int ch, act;
    int prev_sec = -1;

    while (g_running) {

        /* Saati her saniye guncelle */
        {
            int hh, mm, ss;
            bios_get_time(&hh, &mm, &ss);
            if (ss != prev_sec) {
                prev_sec = ss;
                refresh_clock();
            }
        }

        /* Mouse kontrolu */
        if (g_mouse_ok) {
            int mc, mr, mb;
            mouse_get(&mc, &mr, &mb);
            if (mb & MOUSE_LEFT) {
                act = handle_mouse_click(mc, mr, mb);
                if (act != ACT_NONE) {
                    if (dispatch_action(act)) { g_running = 0; break; }
                }
                /* Mouse butonunun birakilmasini bekle */
                while (1) {
                    mouse_get(&mc, &mr, &mb);
                    if (!(mb & MOUSE_LEFT)) break;
                }
                /* Menu acik mi? */
                if (g_menu_open != MENU_NONE) {
                    draw_menubar();
                    act = open_dropdown(g_menu_open);
                    if (act != MENU_NONE && act != ACT_NONE) {
                        if (dispatch_action(act)) { g_running = 0; break; }
                    }
                    /* open_dropdown icinden baska menu istenebilir */
                    while (g_menu_open != MENU_NONE) {
                        int nm = g_menu_open;
                        g_menu_open = MENU_NONE;
                        draw_menubar();
                        act = open_dropdown(nm);
                        if (act != MENU_NONE && act != ACT_NONE) {
                            if (dispatch_action(act)) { g_running = 0; break; }
                        }
                    }
                    g_menu_open = MENU_NONE;
                    redraw_all();
                }
            }
        }

        /* Klavye kontrolu */
        if (!kbhit()) continue;

        ch = getch();

        if (ch == 0) {
            /* Extended tus */
            ch = getch();

            switch (ch) {
                /* F1 = Yardim */
                case 59:
                    dispatch_action(ACT_HELP);
                    redraw_all();
                    break;

                /* F3 = Cikis */
                case 61:
                    if (dispatch_action(ACT_EXIT)) { g_running = 0; }
                    break;

                /* Sag ok - ikon sec */
                case 77:
                    if (g_icon_sel == ICON_NONE) g_icon_sel = 0;
                    else g_icon_sel = (g_icon_sel + 1) % ICON_COUNT;
                    draw_all_icons();
                    break;

                /* Sol ok - ikon sec */
                case 75:
                    if (g_icon_sel == ICON_NONE) g_icon_sel = 0;
                    else g_icon_sel = (g_icon_sel - 1 + ICON_COUNT) % ICON_COUNT;
                    draw_all_icons();
                    break;

                default:
                    /* ALT+harf -> menu */
                    {
                        int mi = alt_to_menu(ch);
                        if (mi != MENU_NONE) {
                            g_menu_open = mi;
                            draw_menubar();
                            act = open_dropdown(mi);
                            /* Baska menu istegi? */
                            while (g_menu_open != MENU_NONE && act == MENU_NONE) {
                                int nm = g_menu_open;
                                g_menu_open = MENU_NONE;
                                draw_menubar();
                                act = open_dropdown(nm);
                            }
                            g_menu_open = MENU_NONE;
                            if (act != MENU_NONE && act != ACT_NONE) {
                                if (dispatch_action(act)) { g_running = 0; }
                            }
                            redraw_all();
                        }
                    }
                    break;
            }

        } else if (ch == 9) {
            /* TAB - sonraki ikon */
            if (g_icon_sel == ICON_NONE) g_icon_sel = 0;
            else g_icon_sel = (g_icon_sel + 1) % ICON_COUNT;
            draw_all_icons();

        } else if (ch == 13) {
            /* ENTER - secili ikonu ac */
            if (g_icon_sel != ICON_NONE) {
                int acts[] = {
                    ACT_LAUNCH_NP, ACT_LAUNCH_VER,
                    ACT_LAUNCH_SETUP, ACT_DOS_PROMPT
                };
                if (dispatch_action(acts[g_icon_sel])) { g_running = 0; }
            }

        } else if (ch == 27) {
            /* ESC - ikon secimini kaldir */
            g_icon_sel = ICON_NONE;
            draw_all_icons();
            update_status("1=Notepad  2=LixVer  3=Setup  4=DOS  ALT=Menu  F1=Yardim  F3=Cik");

        } else if (ch >= '1' && ch <= '8') {
            /* Rakam - direk program ac */
            int idx = ch - '1';
            int acts[] = {
                ACT_LAUNCH_NP, ACT_LAUNCH_VER,
                ACT_LAUNCH_SETUP, ACT_LAUNCH_FILEMAN,
                ACT_LAUNCH_CALC, ACT_LAUNCH_CAL,
                ACT_LAUNCH_CLOCK, ACT_LAUNCH_SNAKE
            };
            g_icon_sel = idx;
            draw_all_icons();
            delay(150);
            if (dispatch_action(acts[idx])) { g_running = 0; }
        }
    }
}

/* =========================================================
   Giris ekrani / splash (kisaca)
   ========================================================= */
static void show_splash(void) {
    int i;
    lixui_setcolor(WHITE, BLUE);
    clrscr();

    /* Cerceve */
    lixui_draw_window(20, 8, 40, 9,
        NULL,
        WHITE, BLUE, WHITE, BLUE);

    /* Logo satirlari */
    lixui_setcolor(YELLOW, BLUE);
    gotoxy(28, 9);  cprintf("L i x O S");
    lixui_setcolor(WHITE, BLUE);
    gotoxy(25, 10); cprintf("Surum %s  Build %s", LIXOS_VER, LIXOS_BUILD);
    lixui_setcolor(DARKGRAY, BLUE);
    gotoxy(24, 11); cprintf("MS-DOS icin Masaustu Kabugu");

    /* Ilerleme cubugu */
    lixui_setcolor(DARKGRAY, BLUE);
    gotoxy(22, 13); cprintf("[                                    ]");
    for (i = 0; i <= 36; i++) {
        lixui_setcolor(LIGHTCYAN, BLUE);
        gotoxy(22 + i, 13);
        putch(BLOCK_FULL);
        delay(30);
    }

    lixui_setcolor(WHITE, BLUE);
    gotoxy(25, 14); cprintf("Yukleniyor...");
    delay(400);
}

/* =========================================================
   main()
   ========================================================= */
int main(void) {
    _setcursortype(_NOCURSOR);

    /* Mouse init dene */
    g_mouse_ok = mouse_init();
    if (g_mouse_ok) mouse_show();

    /* Splash ekrani */
    show_splash();

    /* Ana masaustu */
    g_menu_open = MENU_NONE;
    g_icon_sel  = ICON_NONE;
    g_running   = 1;

    redraw_all();

    /* Event dongusu */
    main_loop();

    /* Temizlik ve cikis */
    if (g_mouse_ok) mouse_hide();
    _setcursortype(_NORMALCURSOR);
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    printf("LixOS'tan cikiliyor. Gorusuruz!\n");

    return 0;
}
