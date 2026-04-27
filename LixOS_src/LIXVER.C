/* =========================================================
   LIXVER.C  -  LixOS Surum ve Sistem Bilgisi
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   Derleme: TCC -ml LIXVER.C
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "LIXUI.H"

/* ---- LixOS surum sabitleri ---- */
#define LIXOS_VER       "1.00"
#define LIXOS_BUILD     "260426"
#define LIXOS_AUTHOR    "LixOS Projesi"
#define LIXOS_LICENSE   "Acik Kaynak / MS-DOS"

/* ---- Sekme sabitleri ---- */
#define TAB_LIXOS   0
#define TAB_SISTEM  1
#define TAB_BELLEK  2
#define TAB_DISK    3
#define TAB_COUNT   4

/* =========================================================
   Sistem bilgisi veri yapisi
   ========================================================= */
typedef struct {
    /* DOS */
    int  dos_major;
    int  dos_minor;
    /* Bellek */
    unsigned int conv_kb;      /* Konvansiyonel bellek (KB) */
    unsigned long ext_kb;      /* Genisletilmis bellek (KB) */
    /* BIOS */
    char bios_date[12];        /* GG/AA/YYYY */
    /* CPU (basit tespit) */
    char cpu_name[24];
    /* Disk */
    unsigned long disk_total;  /* Bayt */
    unsigned long disk_free;   /* Bayt */
    char disk_label[12];
    /* Saat */
    int  time_h, time_m, time_s;
} SysInfo;

static SysInfo g_info;
static int     g_tab = TAB_LIXOS;

/* =========================================================
   Bilgi toplama fonksiyonlari
   ========================================================= */

/* DOS surumu  INT 21h AH=30h */
static void get_dos_version(void) {
    union REGS r;
    r.h.ah = 0x30;
    int86(0x21, &r, &r);
    g_info.dos_major = r.h.al;
    g_info.dos_minor = r.h.ah;
}

/* Konvansiyonel bellek  INT 12h */
static void get_conv_memory(void) {
    union REGS r;
    int86(0x12, &r, &r);
    g_info.conv_kb = r.x.ax;
}

/* Genisletilmis bellek  INT 15h AH=88h */
static void get_ext_memory(void) {
    union REGS r;
    r.h.ah = 0x88;
    int86(0x15, &r, &r);
    if (r.x.cflag == 0)
        g_info.ext_kb = (unsigned long)r.x.ax;
    else
        g_info.ext_kb = 0;
}

/* BIOS tarihi  F000:FFF5 adresinden oku (8 bayt: AA/GG/YYYY) */
static void get_bios_date(void) {
    char far *bdate = (char far *)0xFFFF0005L;
    int i;
    for (i = 0; i < 8; i++)
        g_info.bios_date[i] = bdate[i];
    g_info.bios_date[8] = '\0';
}

/* Basit CPU tipi tespiti (FLAGS biti kontrolu ile) */
static void get_cpu_type(void) {
    /* 8086/8088: FLAGS[12:15] her zaman 1
       286       : FLAGS[12:15] protected modda 0
       386+      : EFLAGS bit 18 (AC) degistirilebilir  */
    unsigned int flags;
    _asm {
        pushf
        pop  ax
        mov  flags, ax
    }
    if ((flags & 0xF000) == 0xF000)
        strcpy(g_info.cpu_name, "Intel 8086/8088");
    else if ((flags & 0xF000) == 0x0000)
        strcpy(g_info.cpu_name, "Intel 80286");
    else
        strcpy(g_info.cpu_name, "Intel 80386+");
}

/* Disk bilgisi  INT 21h AH=36h */
static void get_disk_info(void) {
    union REGS r;
    unsigned long spc, bps, free_cl, total_cl;

    r.h.ah = 0x36;
    r.h.dl = 3;  /* C: = 3 */
    int86(0x21, &r, &r);

    if (r.x.ax == 0xFFFF) {
        /* C: surucusu yok, gecersiz */
        g_info.disk_total = 0;
        g_info.disk_free  = 0;
        strcpy(g_info.disk_label, "N/A");
        return;
    }

    spc      = r.x.ax;   /* Cluster basina sektor       */
    free_cl  = r.x.bx;   /* Bos cluster sayisi          */
    bps      = r.x.cx;   /* Sektor basina bayt          */
    total_cl = r.x.dx;   /* Toplam cluster sayisi       */

    g_info.disk_total = spc * bps * total_cl;
    g_info.disk_free  = spc * bps * free_cl;

    /* Disk etiketi  INT 21h AH=71A0h veya basit yol */
    strcpy(g_info.disk_label, "LIXOS");
}

/* Saat */
static void get_time(void) {
    union REGS r;
    r.h.ah = 0x02;
    int86(0x1A, &r, &r);
    g_info.time_h = ((r.h.ch >> 4) & 0x0F) * 10 + (r.h.ch & 0x0F);
    g_info.time_m = ((r.h.cl >> 4) & 0x0F) * 10 + (r.h.cl & 0x0F);
    g_info.time_s = ((r.h.dh >> 4) & 0x0F) * 10 + (r.h.dh & 0x0F);
}

/* Tum bilgileri topla */
static void collect_info(void) {
    get_dos_version();
    get_conv_memory();
    get_ext_memory();
    get_bios_date();
    get_cpu_type();
    get_disk_info();
    get_time();
}

/* =========================================================
   Ana pencere sabitleri
   ========================================================= */
#define WIN_X   5
#define WIN_Y   3
#define WIN_W   70
#define WIN_H   19
#define BODY_X  (WIN_X + 2)
#define BODY_Y  (WIN_Y + 4)   /* Sekmelerin altindan basla */
#define BODY_W  (WIN_W - 4)
#define BODY_H  (WIN_H - 6)

/* =========================================================
   Sekme cubugunu ciz
   ========================================================= */
static void draw_tabs(void) {
    const char *labels[TAB_COUNT] = {
        " LixOS ", " Sistem ", " Bellek ", " Disk "
    };
    int i;
    int tx = WIN_X + 1;

    /* Tab arkaplan satiri */
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(WIN_X + 1, WIN_Y + 2);
    for (i = 0; i < WIN_W - 2; i++) putch(' ');

    for (i = 0; i < TAB_COUNT; i++) {
        int llen = strlen(labels[i]);
        if (i == g_tab) {
            /* Aktif sekme: beyaz arkaplan, alt cizgisi yok */
            lixui_setcolor(BLUE, WHITE);
            gotoxy(tx, WIN_Y + 2);
            cprintf("%s", labels[i]);
            /* Alt kenari sil (pencere icine baglanir) */
            lixui_setcolor(BLACK, LIGHTGRAY);
            gotoxy(tx, WIN_Y + 3);
            for (i = 0; i < llen; i++) putch(BOX_H); /* geri doldur */
            /* i dongusu bozuldu, duzelt */
            i = g_tab;
        } else {
            lixui_setcolor(WHITE, DARKGRAY);
            gotoxy(tx, WIN_Y + 2);
            cprintf("%s", labels[i]);
        }
        tx += llen + 1;
    }

    /* Sekme alt ayrac cizgisi */
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(WIN_X + 1, WIN_Y + 3);
    for (i = 0; i < WIN_W - 2; i++) putch(BOX_H);
}

/* =========================================================
   Icerik alanini temizle
   ========================================================= */
static void clear_body(void) {
    lixui_fill_rect(BODY_X, BODY_Y, BODY_W, BODY_H,
                    ' ', BLACK, LIGHTGRAY);
}

/* =========================================================
   Satir yazici (gövde icinde)
   ========================================================= */
static int g_body_row;

static void body_row_start(void) {
    g_body_row = BODY_Y;
}

static void body_line(int label_col, int value_col,
                       int fg_lbl, int fg_val,
                       const char *label, const char *value)
{
    lixui_setcolor(fg_lbl, LIGHTGRAY);
    gotoxy(BODY_X + label_col, g_body_row);
    cputs(label);
    if (value) {
        lixui_setcolor(fg_val, LIGHTGRAY);
        gotoxy(BODY_X + value_col, g_body_row);
        cputs(value);
    }
    g_body_row++;
}

static void body_sep(void) {
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(BODY_X, g_body_row, BODY_W, SBOX_H);
    g_body_row++;
}

static void body_blank(void) { g_body_row++; }

/* =========================================================
   Bellek cubugu  (KB cinsinden)
   ========================================================= */
static void draw_mem_bar(int x, int y, int w,
                          unsigned long used, unsigned long total,
                          const char *lbl)
{
    int pct = (total > 0) ? (int)((used * 100UL) / total) : 0;
    if (pct > 100) pct = 100;

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(x, y);
    cprintf("%-12s", lbl);

    lixui_progress(x + 13, y, w - 13, pct,
                   WHITE, BLUE, DARKGRAY, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + w - 12, y);
    cprintf("%lu/%lu KB", used, total);
}

/* =========================================================
   TAB 0: LixOS Surum Bilgisi
   ========================================================= */
static void draw_tab_lixos(void) {
    char buf[48];

    clear_body();
    body_row_start();
    body_blank();

    /* ASCII logo */
    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(BODY_X + 2, g_body_row);
    cprintf("%c%c%c", BLOCK_FULL, BLOCK_MED, BLOCK_LIGHT);
    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(BODY_X + 6, g_body_row);
    cprintf("L I X O S");
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(BODY_X + 16, g_body_row);
    cprintf("v%s (build %s)", LIXOS_VER, LIXOS_BUILD);
    g_body_row += 2;

    body_sep();

    body_line(2, 22, DARKGRAY, BLACK, "Surum          :", LIXOS_VER);
    body_line(2, 22, DARKGRAY, BLACK, "Build          :", LIXOS_BUILD);
    body_line(2, 22, DARKGRAY, BLACK, "Lisans         :", LIXOS_LICENSE);
    body_line(2, 22, DARKGRAY, BLACK, "Gelistirici    :", LIXOS_AUTHOR);
    body_blank();
    body_sep();
    body_blank();

    sprintf(buf, "%d.%02d", g_info.dos_major, g_info.dos_minor);
    body_line(2, 22, DARKGRAY, BLACK, "MS-DOS Surumu  :", buf);

    sprintf(buf, "%s", g_info.bios_date);
    body_line(2, 22, DARKGRAY, BLACK, "BIOS Tarihi    :", buf);

    body_line(2, 22, DARKGRAY, BLACK, "Sistem Saati   :", "");
    lixui_setcolor(BLUE, LIGHTGRAY);
    gotoxy(BODY_X + 22, g_body_row - 1);
    cprintf("%02d:%02d:%02d",
            g_info.time_h, g_info.time_m, g_info.time_s);
}

/* =========================================================
   TAB 1: Sistem Bilgisi
   ========================================================= */
static void draw_tab_sistem(void) {
    char buf[48];

    clear_body();
    body_row_start();
    body_blank();

    body_line(2, 0, BLUE, BLACK, "[ ISLEMCI ]", NULL);
    body_blank();
    body_line(4, 24, DARKGRAY, BLACK, "Model          :", g_info.cpu_name);
    body_line(4, 24, DARKGRAY, BLACK, "Uretici        :", "Intel / Compatible");
    body_line(4, 24, DARKGRAY, BLACK, "Mod            :", "Real Mode (16-bit)");
    body_blank();
    body_sep();
    body_blank();

    body_line(2, 0, BLUE, BLACK, "[ MS-DOS ]", NULL);
    body_blank();
    sprintf(buf, "%d.%02d", g_info.dos_major, g_info.dos_minor);
    body_line(4, 24, DARKGRAY, BLACK, "Surum          :", buf);
    body_line(4, 24, DARKGRAY, BLACK, "Uretici        :", "Microsoft");
    body_line(4, 24, DARKGRAY, BLACK, "BIOS Tarihi    :", g_info.bios_date);
}

/* =========================================================
   TAB 2: Bellek Bilgisi
   ========================================================= */
static void draw_tab_bellek(void) {
    char buf[48];
    unsigned long conv_used;

    clear_body();
    body_row_start();
    body_blank();

    body_line(2, 0, BLUE, BLACK, "[ BELLEK HARITASI ]", NULL);
    g_body_row++;

    /* Konvansiyonel bellek */
    conv_used = (unsigned long)g_info.conv_kb - 400UL; /* tahmini: ~400KB OS */
    if ((long)conv_used < 0) conv_used = 0;

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(BODY_X + 2, g_body_row);
    cprintf("Konvansiyonel  (0 - 640 KB)");
    g_body_row++;

    draw_mem_bar(BODY_X + 2, g_body_row, BODY_W - 4,
                 conv_used, 640UL, "Kullanilan:");
    g_body_row += 2;

    /* Genisletilmis bellek */
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    gotoxy(BODY_X + 2, g_body_row);
    cprintf("Genisletilmis  (> 1 MB)");
    g_body_row++;

    if (g_info.ext_kb > 0) {
        draw_mem_bar(BODY_X + 2, g_body_row, BODY_W - 4,
                     0UL, g_info.ext_kb, "Bos      :");
    } else {
        lixui_setcolor(RED, LIGHTGRAY);
        gotoxy(BODY_X + 4, g_body_row);
        cprintf("Genisletilmis bellek bulunamadi");
    }
    g_body_row += 2;

    body_sep();
    body_blank();

    sprintf(buf, "%u KB  (%u bayt)",
            g_info.conv_kb, (unsigned int)(g_info.conv_kb * 1024UL));
    body_line(2, 26, DARKGRAY, BLACK, "Toplam Conv.  :", buf);

    sprintf(buf, "%lu KB", g_info.ext_kb);
    body_line(2, 26, DARKGRAY, BLACK, "Toplam Ext.   :", g_info.ext_kb > 0 ? buf : "Yok");
}

/* =========================================================
   TAB 3: Disk Bilgisi
   ========================================================= */
static void draw_tab_disk(void) {
    char buf[48];
    unsigned long total_mb, free_mb, used_mb;

    clear_body();
    body_row_start();
    body_blank();

    if (g_info.disk_total == 0) {
        body_blank();
        lixui_setcolor(RED, LIGHTGRAY);
        gotoxy(BODY_X + 4, g_body_row);
        cprintf("C: surucusu bulunamadi veya erisim hatasi.");
        return;
    }

    total_mb = g_info.disk_total / (1024UL * 1024UL);
    free_mb  = g_info.disk_free  / (1024UL * 1024UL);
    used_mb  = total_mb - free_mb;

    body_line(2, 0, BLUE, BLACK, "[ C: SURUCUSU ]", NULL);
    g_body_row++;

    /* Disk kullanim cubugu */
    draw_mem_bar(BODY_X + 2, g_body_row, BODY_W - 4,
                 used_mb, total_mb, "Kullanilan:");
    g_body_row += 2;

    body_sep();
    body_blank();

    sprintf(buf, "%lu MB  (%lu bayt)", total_mb, g_info.disk_total);
    body_line(2, 22, DARKGRAY, BLACK, "Toplam Alan   :", buf);

    sprintf(buf, "%lu MB  (%lu bayt)", free_mb, g_info.disk_free);
    body_line(2, 22, DARKGRAY, BLACK, "Bos Alan      :", buf);

    sprintf(buf, "%lu MB  (%lu bayt)", used_mb,
            g_info.disk_total - g_info.disk_free);
    body_line(2, 22, DARKGRAY, BLACK, "Kullanilan    :", buf);

    body_blank();
    body_sep();
    body_blank();

    body_line(2, 22, DARKGRAY, BLACK, "Disk Etiketi  :", g_info.disk_label);
    body_line(2, 22, DARKGRAY, BLACK, "Surucu        :", "C:");
    body_line(2, 22, DARKGRAY, BLACK, "Dosya Sistemi :", "FAT16");
}

/* =========================================================
   Aktif sekmeyi ciz
   ========================================================= */
static void draw_active_tab(void) {
    switch (g_tab) {
        case TAB_LIXOS:  draw_tab_lixos();  break;
        case TAB_SISTEM: draw_tab_sistem(); break;
        case TAB_BELLEK: draw_tab_bellek(); break;
        case TAB_DISK:   draw_tab_disk();   break;
    }
}

/* =========================================================
   Alt buton cubugu
   ========================================================= */
static void draw_buttons(void) {
    int by = WIN_Y + WIN_H - 2;

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(WIN_X + 1, WIN_Y + WIN_H - 3, WIN_W - 2, SBOX_H);

    lixui_draw_button(WIN_X + 3,          by, "  < Geri  ", 0);
    lixui_draw_button(WIN_X + 17,         by, "  Ileri > ", 0);
    lixui_draw_button(WIN_X + 31,         by, "  Yenile  ", 0);
    lixui_draw_button(WIN_X + WIN_W - 16, by, "   Kapat  ", 1);
}

/* =========================================================
   Tam ekrani yeniden ciz
   ========================================================= */
static void redraw(void) {
    lixui_setcolor(WHITE, BLUE);
    clrscr();

    /* Masaustu arkaplan (LixOS kabugundan donulmus gibi) */
    lixui_setcolor(LIGHTGRAY, CYAN);
    {
        int y;
        for (y = 1; y <= 25; y++) {
            gotoxy(1, y); clreol();
        }
    }

    /* Ana pencere */
    lixui_draw_window(WIN_X, WIN_Y, WIN_W, WIN_H,
        " LixOS Surum Bilgisi ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    /* Sekmeler */
    draw_tabs();

    /* Icerik */
    draw_active_tab();

    /* Butonlar */
    draw_buttons();

    /* Durum cubugu */
    lixui_statusbar(
        "TAB/Sol-Sag: Sekme Degistir  Y=Yenile  ESC/Q=Kapat");
}

/* =========================================================
   main()
   ========================================================= */
int main(void) {
    int ch;
    int running = 1;

    _setcursortype(_NOCURSOR);

    /* Bilgileri topla */
    collect_info();

    /* Ilk cizim */
    g_tab = TAB_LIXOS;
    redraw();

    while (running) {
        ch = getch();

        if (ch == 0) {
            ch = getch();
            switch (ch) {
                case 75:  /* Sol ok - onceki sekme */
                    g_tab = (g_tab - 1 + TAB_COUNT) % TAB_COUNT;
                    redraw();
                    break;
                case 77:  /* Sag ok - sonraki sekme */
                    g_tab = (g_tab + 1) % TAB_COUNT;
                    redraw();
                    break;
            }
        } else {
            switch (ch) {
                case 9:    /* TAB - sonraki sekme */
                    g_tab = (g_tab + 1) % TAB_COUNT;
                    redraw();
                    break;
                case 'y':
                case 'Y':  /* Yenile */
                    collect_info();
                    redraw();
                    break;
                case '1': g_tab = TAB_LIXOS;  redraw(); break;
                case '2': g_tab = TAB_SISTEM; redraw(); break;
                case '3': g_tab = TAB_BELLEK; redraw(); break;
                case '4': g_tab = TAB_DISK;   redraw(); break;
                case 27:   /* ESC */
                case 'q':
                case 'Q':
                    running = 0;
                    break;
            }
        }
    }

    _setcursortype(_NORMALCURSOR);
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    return 0;
}
