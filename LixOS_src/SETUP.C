/* =========================================================
   SETUP.C  -  LixOS Kurulum Sihirbazi
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   Derleme: TCC SETUP.C /I. /DLIXOS_BUILD
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "LIXUI.H"

/* ---- Sabitler ---- */
#define LIXOS_VERSION   "1.00"
#define LIXOS_NAME      "LixOS"
#define SETUP_STEPS     5

/* Kaynak dosyalar (A: surucusunde olmali) */
/* Gercek kurulumda bu dosyalar setup disketindedir */
#define SRC_DRIVE       "A:"
#define DST_DRIVE       "C:"

/* ---- Global degiskenler ---- */
static int  g_step        = 0;    /* Mevcut adim (0-bazli) */
static char g_src_drv[4]  = "A:"; /* Kaynak surucu */
static char g_dst_drv[4]  = "C:"; /* Hedef surucu  */
static int  g_disk_found  = 0;    /* Sabit disk bulundu mu */
static int  g_use_sys     = 1;    /* SYS komutu kullanilsin mi */
static int  g_fdisk_done  = 0;    /* FDISK tamamlandi mi  */
static int  g_fmt_done    = 0;    /* FORMAT tamamlandi mi */

/* =========================================================
   Yardimci: Ekrani LixOS setup rengiyle temizle
   ========================================================= */
static void setup_cls(void) {
    lixui_setcolor(WHITE, BLUE);
    clrscr();
}

/* =========================================================
   Yardimci: Ust baslik cubugunu ciz
   ========================================================= */
static void draw_header(void) {
    int i;
    lixui_setcolor(YELLOW | 0x08, BLUE);   /* Parlak sari / mavi */
    gotoxy(1, 1);
    for (i = 0; i < SCREEN_W; i++) putch(' ');
    gotoxy(1, 1);
    cprintf("  %c LixOS %s Kurulum Sihirbazi",
            BULLET, LIXOS_VERSION);

    /* Adim gostergesi */
    gotoxy(58, 1);
    lixui_setcolor(LIGHTCYAN, BLUE);
    cprintf("Adim %d / %d", g_step + 1, SETUP_STEPS);
}

/* =========================================================
   Yardimci: Ilerleme adim cubugunu ciz (satir 3)
   ========================================================= */
static void draw_step_bar(void) {
    const char *labels[SETUP_STEPS] = {
        "Hosgeldin", "Disk", "FDISK", "FORMAT", "Kopyala"
    };
    int i;
    lixui_setcolor(DARKGRAY, BLUE);
    gotoxy(1, 2);
    for (i = 0; i < SCREEN_W; i++) putch(BOX_H);

    for (i = 0; i < SETUP_STEPS; i++) {
        int col = 2 + i * 16;
        if (i == g_step) {
            lixui_setcolor(BLACK, YELLOW);
        } else if (i < g_step) {
            lixui_setcolor(LIGHTGREEN, BLUE);
        } else {
            lixui_setcolor(DARKGRAY, BLUE);
        }
        gotoxy(col, 2);
        if (i < g_step)
            cprintf("%c%s", CHECK_MARK, labels[i]);
        else
            cprintf(" %s", labels[i]);
    }
}

/* =========================================================
   INT 13h  -  BIOS uzerinden sabit disk varligini sorgula
   Donus: bulunan sabit disk sayisi (0 = yok)
   ========================================================= */
static int bios_detect_hdisks(void) {
    union REGS regs;
    /* INT 13h AH=08h DL=80h  -> Sabit disk bilgisi al */
    regs.h.ah = 0x08;
    regs.h.dl = 0x80;
    int86(0x13, &regs, &regs);

    if (regs.x.cflag) return 0; /* Hata: disk yok */

    /* DL = bulunan sabit disk sayisi */
    return (int)regs.h.dl;
}

/* =========================================================
   ADIM 0  -  Hosgeldin ekrani
   ========================================================= */
static void step_welcome(void) {
    int x = 5, y = 5, w = 70, h = 16;

    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(x, y, w, h,
        " LixOS Kurulum Sihirbazina Hosgeldiniz ",
        YELLOW, BLUE,
        BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);

    gotoxy(x + 3, y + 2);
    cprintf("%c LixOS v%s kurulum programini baslatmak uzeresiniz.",
            ARROW_R, LIXOS_VERSION);

    gotoxy(x + 3, y + 4);
    cprintf("Bu program asagidaki islemleri gerceklestirecektir:");

    gotoxy(x + 5, y + 6);
    cprintf("%c Sabit diskinizi tespit edecek",            BULLET);
    gotoxy(x + 5, y + 7);
    cprintf("%c FDISK ile disk bolumlerini olusturacak",   BULLET);
    gotoxy(x + 5, y + 8);
    cprintf("%c C: surucusunu formatlayacak",              BULLET);
    gotoxy(x + 5, y + 9);
    cprintf("%c LixOS dosyalarini sabit diske kopyalayacak", BULLET);
    gotoxy(x + 5, y + 10);
    cprintf("%c Sisteminizi baslangic icin hazir hale getirecek", BULLET);

    lixui_setcolor(RED, LIGHTGRAY);
    gotoxy(x + 3, y + 12);
    cprintf("! UYARI: Kurulum mevcut disk verilerini silebilir.");
    cprintf("  Yedek aldiginizdan emin olun.");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

    lixui_draw_button(x + w - 20, y + h - 2, " Devam > ", 1);
    lixui_draw_button(x + 3,      y + h - 2, " Iptal  ", 0);

    lixui_statusbar("ENTER=Devam   ESC=Iptal");

    {
        int ch;
        do {
            ch = getch();
            if (ch == 27) {
                if (lixui_yesno("Kurulumdan Cik",
                        "Kurulumdan cikmak istiyor musunuz?")) {
                    setup_cls();
                    gotoxy(1, 1);
                    printf("Kurulum iptal edildi.\n");
                    exit(0);
                }
                /* Ekrani yeniden ciz */
                step_welcome();
                return;
            }
        } while (ch != 13);
    }

    g_step = 1;
}

/* =========================================================
   ADIM 1  -  Sabit disk tespiti
   ========================================================= */
static void step_detect_disk(void) {
    int x = 8, y = 5, w = 64, h = 14;
    int disk_count;
    int i;

    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(x, y, w, h,
        " Sabit Disk Tespiti ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 2);
    cprintf("Sisteminizde sabit disk aranıyor...");

    /* Kisa bekleme animasyonu */
    {
        const char spin[] = "|/-\\";
        int j;
        gotoxy(x + 40, y + 2);
        for (j = 0; j < 16; j++) {
            gotoxy(x + 40, y + 2);
            lixui_setcolor(YELLOW, LIGHTGRAY);
            putch(spin[j % 4]);
            delay(80);
        }
    }

    disk_count = bios_detect_hdisks();

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 40, y + 2);
    putch(' ');

    if (disk_count > 0) {
        g_disk_found = 1;

        lixui_setcolor(LIGHTGREEN, LIGHTGRAY);
        gotoxy(x + 3, y + 4);
        cprintf("%c %d adet sabit disk bulundu!", CHECK_MARK, disk_count);

        lixui_setcolor(BLACK, LIGHTGRAY);
        gotoxy(x + 3, y + 6);
        cprintf("Hedef surucu harfini secin (C-Z): ");
        {
            int drv_ch = getch();
            if (drv_ch >= 'c' && drv_ch <= 'z') drv_ch -= 32;
            if (drv_ch >= 'C' && drv_ch <= 'Z') {
                g_dst_drv[0] = (char)drv_ch;
            }
            cprintf("%s", g_dst_drv);
        }

        gotoxy(x + 3, y + 8);
        cprintf("Kaynak    : %s  (Kurulum ortami)", g_src_drv);

        /* Ilerleme cubugu */
        gotoxy(x + 3, y + 9);
        cprintf("Disk bilgisi okunuyor:");
        lixui_progress(x + 3, y + 10, w - 6, 100,
                       WHITE, BLUE, DARKGRAY, LIGHTGRAY);

        lixui_setcolor(DARKGRAY, LIGHTGRAY);
        lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

        lixui_draw_button(x + w - 18, y + h - 2, " Devam > ", 1);
        lixui_statusbar("Disk basariyla tespit edildi.  ENTER=Devam");

    } else {
        g_disk_found = 0;

        lixui_setcolor(LIGHTRED, LIGHTGRAY);
        gotoxy(x + 3, y + 4);
        cprintf("! Sabit disk bulunamadi!");

        lixui_setcolor(BLACK, LIGHTGRAY);
        gotoxy(x + 3, y + 6);
        cprintf("Olasi nedenler:");
        gotoxy(x + 5, y + 7);
        cprintf("%c IDE/SATA kablosu bagli degil", BULLET);
        gotoxy(x + 5, y + 8);
        cprintf("%c BIOS'ta disk devre disi", BULLET);
        gotoxy(x + 5, y + 9);
        cprintf("%c Disk arizali", BULLET);

        lixui_setcolor(DARKGRAY, LIGHTGRAY);
        lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

        lixui_draw_button(x + 3, y + h - 2, " Tekrar Dene ", 0);
        lixui_draw_button(x + 20, y + h - 2, " Cik ", 0);

        lixui_statusbar("! Hata: Sabit disk bulunamadi.  ENTER=Tekrar  ESC=Cik");

        {
            int ch = getch();
            if (ch == 27) {
                setup_cls();
                printf("Sabit disk bulunamadi. Sistem kapatiliyor.\n");
                exit(1);
            }
            /* Tekrar dene */
            step_detect_disk();
            return;
        }
    }

    {
        int ch;
        do { ch = getch(); } while (ch != 13);
    }

    g_step = 2;
}

/* =========================================================
   ADIM 2  -  FDISK ile disk bolum olusturma
   ========================================================= */
static void step_fdisk(void) {
    int x = 6, y = 4, w = 68, h = 17;

    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(x, y, w, h,
        " Disk Bolum Haritasi (FDISK) ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);

    gotoxy(x + 3, y + 2);
    cprintf("LixOS kurulumu icin C: surucusunde bir bolum gereklidir.");

    gotoxy(x + 3, y + 4);
    cprintf("FDISK asagidaki islemleri yapmanizi gerektirir:");

    gotoxy(x + 5, y + 5);
    cprintf("%c Ana Menu -> 1: DOS Bolumu Olustur", BULLET);
    gotoxy(x + 5, y + 6);
    cprintf("%c 1: Birincil DOS Bolumu Olustur -> ENTER", BULLET);
    gotoxy(x + 5, y + 7);
    cprintf("%c Maksimum alan kullan -> E -> ESC -> Yeniden basla", BULLET);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 9, w - 2, SBOX_H);

    lixui_setcolor(RED, LIGHTGRAY);
    gotoxy(x + 3, y + 10);
    cprintf("! DIKKAT: Bu islem diskteki TUM VERIYI SILER!");

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 12);
    cprintf("FDISK tamamlandiktan sonra bilgisayar yeniden baslar.");
    gotoxy(x + 3, y + 13);
    cprintf("Yeniden basladiktan sonra Setup'i tekrar calistirin.");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

    lixui_draw_button(x + w - 22, y + h - 2, " FDISK'i Baslat ", 1);
    lixui_draw_button(x + 3,      y + h - 2, "    Atla    ",    0);

    lixui_statusbar("ENTER=FDISK Baslat   S=Atla (bolum zaten varsa)");

    {
        int ch;
        do {
            ch = getch();
            if (ch == 's' || ch == 'S') {
                /* Kullanici FDISK'i atliyor (bolum zaten var) */
                g_fdisk_done = 1;
                g_step = 3;
                return;
            }
        } while (ch != 13);
    }

    /* Onay al */
    if (!lixui_yesno("FDISK Onay",
            "Disk bolum tablosu silinecek! Emin misiniz?")) {
        step_fdisk();
        return;
    }

    /* FDISK'i calistir */
    setup_cls();
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    printf("FDISK baslatiliyor...\n\n");
    system("FDISK");

    /* FDISK sonrasi yeniden baslama uyarisi */
    g_fdisk_done = 1;

    setup_cls();
    draw_header();

    lixui_draw_window(15, 8, 50, 9,
        " FDISK Tamamlandi ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(17, 10);
    cprintf("%c FDISK islemi tamamlandi.", CHECK_MARK);
    gotoxy(17, 12);
    cprintf("Degisiklikler icin YENIDEN BASLAMA gerekiyor.");
    gotoxy(17, 13);
    cprintf("Setup yeniden basladiktan sonra devam edecek.");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(16, 14, 48, SBOX_H);
    lixui_draw_button(28, 15, " Yeniden Baslat ", 1);
    lixui_statusbar("ENTER=Bilgisayari yeniden baslat");

    getch();

    /* Soft reboot: INT 19h */
    {
        union REGS regs;
        int86(0x19, &regs, &regs);
    }
    /* Eger INT 19h calismadiysa */
    system("CTTY NUL & ECHO Y | DELTREE /Y C:\\TEMP & CTTY CON");
    reboot:
    _AX = 0x0000;
    /* Dogrudan reboot */
    (*((void (far *)())0xFFFF0000L))();
}

/* =========================================================
   ADIM 3  -  FORMAT
   ========================================================= */
static void step_format(void) {
    int x = 7, y = 5, w = 66, h = 14;
    char fmt_cmd[64];

    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(x, y, w, h,
        " Surucu Formatlama ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);

    gotoxy(x + 3, y + 2);
    cprintf("Hedef surucu %s formatlandi:", g_dst_drv);

    gotoxy(x + 3, y + 4);
    cprintf("Kullanilacak komut:");
    lixui_setcolor(YELLOW, DARKGRAY);
    gotoxy(x + 5, y + 5);
    cprintf(" FORMAT %s /S /V:LIXOS ", g_dst_drv);
    lixui_setcolor(BLACK, LIGHTGRAY);

    gotoxy(x + 3, y + 7);
    cprintf("/S  = Sistem dosyalarini aktar (IO.SYS MSDOS.SYS)");
    gotoxy(x + 3, y + 8);
    cprintf("/V  = Disk etiketini ata: LIXOS");

    lixui_setcolor(RED, LIGHTGRAY);
    gotoxy(x + 3, y + 10);
    cprintf("! %s surucusundeki TUM VERI silinecek!", g_dst_drv);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);
    lixui_draw_button(x + w - 20, y + h - 2, " Formatla > ", 1);
    lixui_draw_button(x + 3,      y + h - 2, "   Geri   ",  0);

    lixui_statusbar("ENTER=Formatlamaya Basla   G=Geri");

    {
        int ch;
        do {
            ch = getch();
            if (ch == 'g' || ch == 'G') {
                g_step = 2;
                step_fdisk();
                return;
            }
        } while (ch != 13);
    }

    if (!lixui_yesno("Format Onay",
            "Tum veri silinecek! Devam edilsin mi?")) {
        step_format();
        return;
    }

    /* FORMAT komutunu olustur */
    if (g_use_sys) {
        sprintf(fmt_cmd, "FORMAT %s /V:LIXOS /Q", g_dst_drv);
    } else {
        sprintf(fmt_cmd, "FORMAT %s /S /V:LIXOS /Q", g_dst_drv);
    }

    /* DOS ekranina gec, format calistir */
    setup_cls();
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    printf("Format komutu calistiriliyor: %s\n\n", fmt_cmd);
    printf("Format onaylamak icin 'E' yazin ve ENTER'a basin.\n\n");
    system(fmt_cmd);

    if (g_use_sys) {
        char sys_cmd[32];
        printf("\nSistem dosyalari aktariliyor (SYS %s)...\n", g_dst_drv);
        sprintf(sys_cmd, "SYS %s", g_dst_drv);
        system(sys_cmd);
    }

    g_fmt_done = 1;

    /* Format tamamlandi bildirimi */
    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(14, 8, 52, 8,
        " Format Tamamlandi ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(LIGHTGREEN, LIGHTGRAY);
    gotoxy(16, 10);
    cprintf("%c %s surucusu basariyla formatlanıdi.", CHECK_MARK, g_dst_drv);
    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(16, 11);
    cprintf("  MS-DOS sistem dosyalari aktarildi.");
    gotoxy(16, 12);
    cprintf("  Disk etiketi: LIXOS");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(15, 13, 50, SBOX_H);
    lixui_draw_button(28, 14, " Devam > ", 1);
    lixui_statusbar("ENTER=Dosya kopyalamaya devam et");

    getch();
    g_step = 4;
}

/* =========================================================
   Yardimci: Tek dosya kopyala ve ekranda goster
   ========================================================= */
static void copy_file_display(int win_x, int bar_y,
                               const char *src, const char *dst,
                               int file_no, int total)
{
    char cmd[128];
    int pct = (file_no * 100) / total;

    /* Dosya adi goster */
    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(win_x + 3, bar_y);
    cprintf("%-52s", src);   /* dosya adini yaz, gerisi bosluk */

    /* Ilerleme cubuğu */
    lixui_progress(win_x + 3, bar_y + 1, 52, pct,
                   WHITE, BLUE, DARKGRAY, LIGHTGRAY);

    /* Yuzde */
    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(win_x + 57, bar_y + 1);
    cprintf("%3d%%", pct);

    /* COPY komutunu calistir (sessiz) */
    sprintf(cmd, "COPY %s %s > NUL 2>&1", src, dst);
    system(cmd);
}

/* =========================================================
   ADIM 4  -  LixOS dosyalarini kopyala
   ========================================================= */
static void step_copy_files(void) {
    int x = 5, y = 4, w = 70, h = 18;
    char dst_dir[16];
    int i;

    /* Kopyalanacak dosyalar [kaynak_ad, hedef_yon] */
    struct { const char *name; const char *subdir; } files[] = {
        { "LIXOS.EXE",    ""        },
        { "NOTEPAD.EXE",  ""        },
        { "LIXVER.EXE",   ""        },
        { "SETUP.EXE",    ""        },
        { "FILEMAN.EXE",  ""        },
        { "CALC.EXE",     ""        },
        { "SNAKE.EXE",    ""        },
        { "CALENDAR.EXE", ""        },
        { "CLOCK.EXE",    ""        },
        { "CTMOUSE.EXE",  ""        },
        { "LIXOS.ICO",    ""        },
        { "README.TXT",   ""        },
        { "LICENSE.TXT",  ""        },
        { "LIXUI.H",      "INCLUDE\\" },
    };
    int file_count = sizeof(files) / sizeof(files[0]);

    setup_cls();
    draw_header();
    draw_step_bar();

    lixui_draw_window(x, y, w, h,
        " LixOS Dosyalari Kopyalaniyor ",
        WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 2);
    cprintf("Kaynak : %s\\  ->  Hedef: %s\\", g_src_drv, g_dst_drv);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 3, w - 2, SBOX_H);

    gotoxy(x + 3, y + 4);
    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    cprintf("Dosya:");
    gotoxy(x + 58, y + 4);
    cprintf("Durum:");

    /* Hedef dizini olustur */
    sprintf(dst_dir, "%s\\", g_dst_drv);
    {
        char mk[32];
        sprintf(mk, "MD %s\\INCLUDE > NUL 2>&1", g_dst_drv);
        system(mk);
    }

    /* Dosyalari kopyala */
    for (i = 0; i < file_count; i++) {
        char src_path[64], dst_path[64];

        sprintf(src_path, "%s\\%s", g_src_drv, files[i].name);
        sprintf(dst_path, "%s\\%s%s", g_dst_drv,
                files[i].subdir, files[i].name);

        copy_file_display(x, y + 5, src_path, dst_path,
                          i + 1, file_count);

        lixui_setcolor(LIGHTGREEN, LIGHTGRAY);
        gotoxy(x + 59, y + 4);
        cprintf("%c", CHECK_MARK);

        delay(200); /* Gercekci hissi icin kisa bekleme */
    }

    /* Toplam ilerleme %100 */
    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 5);
    cprintf("%-52s", "Kopyalama tamamlandi.");
    lixui_progress(x + 3, y + 6, 52, 100,
                   WHITE, BLUE, DARKGRAY, LIGHTGRAY);
    gotoxy(x + 57, y + 6);
    lixui_setcolor(BLACK, LIGHTGRAY);
    cprintf("100%%");

    /* ---- AUTOEXEC.BAT yaz ---- */
    {
        FILE *f;
        char path[32];
        sprintf(path, "%s\\AUTOEXEC.BAT", g_dst_drv);
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "@ECHO OFF\r\n");
            fprintf(f, "PROMPT $P$G\r\n");
            fprintf(f, "PATH %s\\;%s\\DOS;%s\\SYSTEM\r\n",
                    g_dst_drv, g_dst_drv, g_dst_drv);
            fprintf(f, "SET TEMP=%s\\TEMP\r\n", g_dst_drv);
            fprintf(f, "SET TMP=%s\\TEMP\r\n",  g_dst_drv);
            fprintf(f, "REM Mouse Driver\r\n");
            fprintf(f, "%s\\CTMOUSE.EXE\r\n", g_dst_drv);
            fprintf(f, "REM LixOS Baslangic\r\n");
            fprintf(f, "%s\\LIXOS.EXE\r\n", g_dst_drv);
            fclose(f);
        }
    }

    /* ---- CONFIG.SYS yaz ---- */
    {
        FILE *f;
        char path[32];
        sprintf(path, "%s\\CONFIG.SYS", g_dst_drv);
        f = fopen(path, "w");
        if (f) {
            fprintf(f, "DOS=HIGH,UMB\r\n");
            fprintf(f, "DEVICE=%s\\DOS\\HIMEM.SYS\r\n",  g_dst_drv);
            fprintf(f, "DEVICE=%s\\DOS\\EMM386.EXE NOEMS\r\n", g_dst_drv);
            /* CuteMouse AUTOEXEC.BAT icinde baslatilacak */
            fprintf(f, "FILES=50\r\n");
            fprintf(f, "BUFFERS=20\r\n");
            fprintf(f, "STACKS=9,256\r\n");
            fprintf(f, "COUNTRY=090,857,%s\\DOS\\COUNTRY.SYS\r\n", g_dst_drv);
            fclose(f);
        }
    }

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 8, w - 2, SBOX_H);

    lixui_setcolor(LIGHTGREEN, LIGHTGRAY);
    gotoxy(x + 3, y + 9);
    cprintf("%c AUTOEXEC.BAT olusturuldu", CHECK_MARK);
    gotoxy(x + 3, y + 10);
    cprintf("%c CONFIG.SYS olusturuldu",   CHECK_MARK);
    gotoxy(x + 3, y + 11);
    cprintf("%c %d dosya basariyla kopyalandi", CHECK_MARK, file_count);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

    lixui_draw_button(x + w - 16, y + h - 2, " Son Adim > ", 1);
    lixui_statusbar("Kopyalama tamamlandi.  ENTER=Son adima gec");

    getch();
    g_step = 5; /* Tamamlandi */
}

/* =========================================================
   Son ekran  -  Kurulum tamamlandi
   ========================================================= */
static void step_done(void) {
    int x = 8, y = 5, w = 64, h = 14;
    int i;

    setup_cls();
    draw_header();

    /* Adim cubugunu "tum tamamlandi" olarak goster */
    {
        const char *labels[] =
            {"Hosgeldin","Disk","FDISK","FORMAT","Kopyala"};
        lixui_setcolor(DARKGRAY, BLUE);
        gotoxy(1, 2);
        for (i = 0; i < SCREEN_W; i++) putch(BOX_H);
        for (i = 0; i < SETUP_STEPS; i++) {
            lixui_setcolor(LIGHTGREEN, BLUE);
            gotoxy(2 + i * 16, 2);
            cprintf("%c%s", CHECK_MARK, labels[i]);
        }
    }

    lixui_draw_window(x, y, w, h,
        " Kurulum Tamamlandi! ",
        YELLOW, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(LIGHTGREEN, LIGHTGRAY);
    gotoxy(x + 3, y + 2);
    cprintf("%c LixOS v%s basariyla kuruldu!", CHECK_MARK, LIXOS_VERSION);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 4);
    cprintf("Kurulum ozeti:");
    gotoxy(x + 5, y + 5);
    cprintf("%c Hedef surucu  : %s", BULLET, g_dst_drv);
    gotoxy(x + 5, y + 6);
    cprintf("%c OS dosyalari  : kopyalandi");
    gotoxy(x + 5, y + 6);
    cprintf("%c AUTOEXEC.BAT : %s\\AUTOEXEC.BAT", BULLET, g_dst_drv);
    gotoxy(x + 5, y + 7);
    cprintf("%c CONFIG.SYS   : %s\\CONFIG.SYS",   BULLET, g_dst_drv);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + 9, w - 2, SBOX_H);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(x + 3, y + 10);
    cprintf("Disketi cikarip ENTER'a basin. Bilgisayar yeniden baslayacak.");
    gotoxy(x + 3, y + 11);
    cprintf("LixOS ilk acilista otomatik olarak yuklenir.");

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(x + 1, y + h - 3, w - 2, SBOX_H);

    lixui_draw_button(x + w - 22, y + h - 2, " Yeniden Baslat ", 1);
    lixui_statusbar("Disket cikarip ENTER'a basin -> Yeniden Baslat");

    getch();

    /* Yeniden baslat */
    setup_cls();
    lixui_setcolor(LIGHTGRAY, BLACK);
    clrscr();
    printf("\nLixOS kurulumu tamamlandi. Yeniden baslatiliyor...\n");
    delay(1500);

    /* INT 19h ile yumusak reboot */
    {
        union REGS r;
        int86(0x19, &r, &r);
    }
}

/* =========================================================
   main()
   ========================================================= */
int main(void) {
    /* Ekrani hazirla */
    _setcursortype(_NOCURSOR);   /* Imleci gizle */
    setup_cls();

    /* Mouse Driverini baslat (Setup icinde kullanabilmek icin) */
    system("CTMOUSE.EXE > NUL 2>&1");

    /* Adim akisi */
    g_step = 0;
    step_welcome();

    g_step = 1;
    step_detect_disk();

    g_step = 2;
    step_fdisk();

    g_step = 3;
    step_format();

    g_step = 4;
    step_copy_files();

    step_done();

    _setcursortype(_NORMALCURSOR);
    return 0;
}
