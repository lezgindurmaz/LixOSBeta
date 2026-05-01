/*=============================================================
  NOTEPAD.C  -  Blue-background text editor for LixOS
  DJGPP / DOS
=============================================================*/
#include "notepad.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ---- Editor state ---- */
#define NP_COLS   34    /* visible columns */
#define NP_ROWS   11    /* visible rows */
#define NP_MAX_LINES 50
#define NP_LINE_LEN  80

static char  np_lines[NP_MAX_LINES][NP_LINE_LEN+1];
static int   np_cur_line = 0;
static int   np_cur_col  = 0;
static int   np_scroll   = 0;  /* top visible line */
static int   np_line_count = 1;
static int   np_dirty    = 0;  /* unsaved changes */
static int   np_win_id   = -1;
static char  np_filename[64] = "UNTITLED.TXT";

/* Content area origin (inside window) */
static int   np_cx, np_cy;

static void np_clear_all(void) {
    int i;
    for(i=0; i<NP_MAX_LINES; i++) np_lines[i][0]='\0';
    np_cur_line = 0; np_cur_col = 0;
    np_scroll   = 0; np_line_count = 1;
    np_dirty    = 0;
    strcpy(np_filename, "UNTITLED.TXT");
}

static void np_insert_char(char c) {
    char *line = np_lines[np_cur_line];
    int len = strlen(line);
    int i;
    if(len >= NP_LINE_LEN) return;
    for(i=len; i>np_cur_col; i--) line[i]=line[i-1];
    line[np_cur_col++] = c;
    line[len+1] = '\0';
    np_dirty = 1;
}

static void np_delete_char(void) {
    char *line = np_lines[np_cur_line];
    int len = strlen(line);
    int i;
    if(np_cur_col == 0) {
        /* Merge with previous line */
        if(np_cur_line == 0) return;
        np_cur_col = strlen(np_lines[np_cur_line-1]);
        strcat(np_lines[np_cur_line-1], line);
        for(i=np_cur_line; i<np_line_count-1; i++)
            strcpy(np_lines[i], np_lines[i+1]);
        np_lines[np_line_count-1][0]='\0';
        np_line_count--;
        np_cur_line--;
    } else {
        for(i=np_cur_col-1; i<len-1; i++) line[i]=line[i+1];
        line[len-1]='\0';
        np_cur_col--;
    }
    np_dirty=1;
}

static void np_newline(void) {
    int i;
    char *cur = np_lines[np_cur_line];
    if(np_line_count >= NP_MAX_LINES) return;
    /* Shift lines down */
    for(i=np_line_count; i>np_cur_line+1; i--)
        strcpy(np_lines[i], np_lines[i-1]);
    /* Split at cursor */
    strcpy(np_lines[np_cur_line+1], cur + np_cur_col);
    cur[np_cur_col] = '\0';
    np_cur_line++;
    np_cur_col = 0;
    np_line_count++;
    np_dirty=1;
}

/* ---- Draw the editor content ---- */
static void np_draw_content(void) {
    int r, c, ln;
    /* Blue content area */
    vga_fillrect(np_cx, np_cy, NP_COLS*8, NP_ROWS*9, COL_NOTEPAD_BG);
    /* Draw text lines */
    for(r=0; r<NP_ROWS; r++) {
        ln = np_scroll + r;
        if(ln >= np_line_count) break;
        vga_putstr(np_cx, np_cy + r*9, np_lines[ln],
                   COL_NOTEPAD_TEXT, COL_NOTEPAD_BG);
    }
    /* Draw cursor (blinking simulation: always show) */
    {
        int cur_r = np_cur_line - np_scroll;
        int cur_c = np_cur_col;
        if(cur_r >= 0 && cur_r < NP_ROWS) {
            int cx2 = np_cx + cur_c * 8;
            int cy2 = np_cy + cur_r * 9;
            vga_vline(cx2, cy2, cy2+7, COL_WHITE);
        }
    }
    /* Status bar: line/col + dirty marker */
    {
        char status[40];
        int sy = np_cy + NP_ROWS*9 + 2;
        vga_fillrect(np_cx, sy, NP_COLS*8, 9, COL_WIN_DGRAY);
        sprintf(status, "Ln:%d Col:%d %s",
                np_cur_line+1, np_cur_col+1, np_dirty?"*":"");
        vga_putstr(np_cx+2, sy, status, COL_LIGHT_GRAY, COL_WIN_DGRAY);
    }
}

/* ---- File I/O ---- */
static void np_save(const char *fname) {
    FILE *f = fopen(fname, "w");
    int i;
    if(!f) return;
    for(i=0; i<np_line_count; i++) {
        fprintf(f, "%s\n", np_lines[i]);
    }
    fclose(f);
    np_dirty = 0;
    strcpy(np_filename, fname);
}

static void np_load(const char *fname) {
    FILE *f = fopen(fname, "r");
    char buf[NP_LINE_LEN+2];
    if(!f) return;
    np_clear_all();
    np_line_count = 0;
    while(fgets(buf, sizeof(buf), f) && np_line_count < NP_MAX_LINES) {
        /* strip newline */
        char *p = strchr(buf, '\n');
        if(p) *p = '\0';
        p = strchr(buf, '\r');
        if(p) *p = '\0';
        strcpy(np_lines[np_line_count++], buf);
    }
    if(np_line_count == 0) np_line_count = 1;
    fclose(f);
    np_dirty = 0;
    strcpy(np_filename, fname);
}

/* ---- Menu bar ---- */
static int np_menu_open = 0;  /* 0=none 1=File */

static void np_draw_menu(int win_x, int win_y, int win_w) {
    int my = win_y + 2 + TITLE_H + 1;
    vga_fillrect(win_x+2, my, win_w-4, 10, COL_MENU_BG);
    vga_hline(win_x+2, win_x+win_w-3, my+10, COL_WIN_DGRAY);
    /* File menu */
    if(np_menu_open == 1) {
        vga_fillrect(win_x+3, my, 24, 10, COL_WIN_BLUE);
        vga_putstr(win_x+4,  my+1, "File", COL_WHITE, COL_WIN_BLUE);
        /* Dropdown */
        vga_fillrect(win_x+3, my+10, 60, 50, COL_MENU_BG);
        vga_rect(win_x+3, my+10, 60, 50, COL_WIN_DGRAY);
        vga_putstr(win_x+6, my+12, "New     ", COL_BLACK, COL_MENU_BG);
        vga_putstr(win_x+6, my+22, "Open... ", COL_BLACK, COL_MENU_BG);
        vga_putstr(win_x+6, my+32, "Save    ", COL_BLACK, COL_MENU_BG);
        vga_putstr(win_x+6, my+42, "Close   ", COL_BLACK, COL_MENU_BG);
    } else {
        vga_putstr(win_x+4, my+1, "File", COL_BLACK, COL_MENU_BG);
    }
}

/* ---- Public API ---- */
void notepad_update(int id);

void notepad_open(void) {
    if(np_win_id >= 0 && windows[np_win_id].visible) return;
    np_win_id = wm_open(20, 10, NP_COLS*8+8, NP_ROWS*9+40, "Notepad - LixOS", notepad_update);
    np_clear_all();
}

void notepad_update(int id) {
    Window *w;
    int key = 0;

    if(id < 0 || !windows[id].visible) return;
    w = &windows[id];

    /* Content origin */
    np_cx = w->x + 4;
    np_cy = w->y + 2 + TITLE_H + 12; /* below menu */

    /* --- Draw --- */
    np_draw_menu(w->x, w->y, w->w);
    np_draw_content();

    /* --- Mouse: menu toggle --- */
    if(mouse.left_click) {
        int my = w->y + 2 + TITLE_H + 1;
        if(gui_inside(mouse.x, mouse.y, w->x+3, my, 24, 10)) {
            np_menu_open = (np_menu_open == 1) ? 0 : 1;
        } else if(np_menu_open == 1) {
            /* Dropdown items */
            if(gui_inside(mouse.x, mouse.y, w->x+6, my+12, 52, 9))
                { np_clear_all(); np_menu_open=0; }
            else if(gui_inside(mouse.x, mouse.y, w->x+6, my+22, 52, 9))
                { np_load("TEST.TXT"); np_menu_open=0; } /* Simple hardcoded for now */
            else if(gui_inside(mouse.x, mouse.y, w->x+6, my+32, 52, 9))
                { np_save(np_filename); np_menu_open=0; }
            else if(gui_inside(mouse.x, mouse.y, w->x+6, my+42, 52, 9))
                { wm_close(np_win_id); np_win_id=-1; return; }
            else np_menu_open=0;
        }
        /* Click inside text area to move cursor */
        if(gui_inside(mouse.x, mouse.y, np_cx, np_cy,
                       NP_COLS*8, NP_ROWS*9)) {
            int ln = (mouse.y - np_cy) / 9 + np_scroll;
            int co = (mouse.x - np_cx) / 8;
            if(ln < np_line_count) {
                np_cur_line = ln;
                int slen = strlen(np_lines[ln]);
                np_cur_col = (co > slen) ? slen : co;
            }
        }
    }

    /* --- Keyboard --- */
    if(kbhit()) {
        key = getch();
        if(key == 0) {
            /* Extended key */
            key = getch();
            switch(key) {
                case 72: /* Up    */
                    if(np_cur_line > 0) np_cur_line--;
                    if(np_cur_col > (int)strlen(np_lines[np_cur_line]))
                        np_cur_col = strlen(np_lines[np_cur_line]);
                    break;
                case 80: /* Down  */
                    if(np_cur_line < np_line_count-1) np_cur_line++;
                    if(np_cur_col > (int)strlen(np_lines[np_cur_line]))
                        np_cur_col = strlen(np_lines[np_cur_line]);
                    break;
                case 75: /* Left  */
                    if(np_cur_col > 0) np_cur_col--;
                    break;
                case 77: /* Right */
                    if(np_cur_col < (int)strlen(np_lines[np_cur_line]))
                        np_cur_col++;
                    break;
                case 73: /* PgUp  */
                    np_cur_line -= NP_ROWS;
                    if(np_cur_line < 0) np_cur_line=0;
                    break;
                case 81: /* PgDn  */
                    np_cur_line += NP_ROWS;
                    if(np_cur_line >= np_line_count)
                        np_cur_line = np_line_count-1;
                    break;
            }
        } else {
            switch(key) {
                case 13:  np_newline(); break;          /* Enter */
                case 8:   np_delete_char(); break;      /* Backspace */
                case 27:  /* Escape -> close */
                    wm_close(np_win_id); np_win_id=-1; return;
                default:
                    if(key >= 32 && key <= 126)
                        np_insert_char((char)key);
                    break;
            }
        }
        /* Scroll into view */
        if(np_cur_line < np_scroll) np_scroll = np_cur_line;
        if(np_cur_line >= np_scroll + NP_ROWS)
            np_scroll = np_cur_line - NP_ROWS + 1;
    }

}
