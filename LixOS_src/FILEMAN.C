/* =========================================================
   FILEMAN.C  -  LixOS Dosya Yoneticisi
   Turbo C 2.0 / 3.0  |  MS-DOS 6.22
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <dos.h>
#include <conio.h>
#include "LIXUI.H"

#define MAX_FILES 200

typedef struct {
    char name[13];
    long size;
    int attrib;
} FileInfo;

static FileInfo g_files[MAX_FILES];
static int g_file_count = 0;
static int g_selected = 0;
static char g_cwd[64];

/* ---- Dosyalari listele ---- */
void read_files() {
    struct ffblk ff;
    int done;
    g_file_count = 0;

    /* Once dizinleri oku */
    done = findfirst("*.*", &ff, FA_DIREC);
    while (!done && g_file_count < MAX_FILES) {
        if (ff.ff_attrib & FA_DIREC) {
            if (strcmp(ff.ff_name, ".") != 0) {
                strcpy(g_files[g_file_count].name, ff.ff_name);
                g_files[g_file_count].size = 0;
                g_files[g_file_count].attrib = ff.ff_attrib;
                g_file_count++;
            }
        }
        done = findnext(&ff);
    }

    /* Sonra dosyalari oku */
    done = findfirst("*.*", &ff, FA_ARCH | FA_RDONLY);
    while (!done && g_file_count < MAX_FILES) {
        if (!(ff.ff_attrib & FA_DIREC)) {
            strcpy(g_files[g_file_count].name, ff.ff_name);
            g_files[g_file_count].size = ff.ff_fsize;
            g_files[g_file_count].attrib = ff.ff_attrib;
            g_file_count++;
        }
        done = findnext(&ff);
    }
}

void draw_file_list() {
    int i;
    int win_x = 5, win_y = 4, win_w = 70, win_h = 18;

    lixui_draw_window(win_x, win_y, win_w, win_h, " LixOS Dosya Yoneticisi ", WHITE, BLUE, BLACK, LIGHTGRAY);

    lixui_setcolor(BLACK, LIGHTGRAY);
    gotoxy(win_x + 2, win_y + 1);
    cprintf("Dizin: %s", g_cwd);

    lixui_setcolor(DARKGRAY, LIGHTGRAY);
    lixui_hline(win_x + 1, win_y + 2, win_w - 2, SBOX_H);

    for (i = 0; i < 14; i++) {
        int idx = i; /* Kaydirma ekleyebiliriz ilerde */
        if (idx >= g_file_count) break;

        if (idx == g_selected) {
            lixui_setcolor(WHITE, BLUE);
        } else {
            lixui_setcolor(BLACK, LIGHTGRAY);
        }

        gotoxy(win_x + 2, win_y + 3 + i);
        if (g_files[idx].attrib & FA_DIREC) {
            cprintf("[%-12s]       <DIR>", g_files[idx].name);
        } else {
            cprintf("%-12s    %8ld bayt", g_files[idx].name, g_files[idx].size);
        }
    }

    lixui_statusbar("Oklar: Sec  ENTER: Ac/Gir  DEL: Sil  ESC: Cik");
}

int main() {
    int ch;
    getcwd(g_cwd, 64);
    read_files();

    _setcursortype(_NOCURSOR);

    while (1) {
        draw_file_list();
        ch = getch();
        if (ch == 0) {
            ch = getch();
            if (ch == 72 && g_selected > 0) g_selected--;
            if (ch == 80 && g_selected < g_file_count - 1) g_selected++;
            if (ch == 83) { /* DEL */
                if (lixui_yesno("Dosya Sil", "Secili dosyayi silmek istiyor musunuz?")) {
                    remove(g_files[g_selected].name);
                    read_files();
                }
            }
        } else if (ch == 13) {
            if (g_files[g_selected].attrib & FA_DIREC) {
                chdir(g_files[g_selected].name);
                getcwd(g_cwd, 64);
                read_files();
                g_selected = 0;
            }
        } else if (ch == 27) {
            break;
        }
    }

    _setcursortype(_NORMALCURSOR);
    return 0;
}
