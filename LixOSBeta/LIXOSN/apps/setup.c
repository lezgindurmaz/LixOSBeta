/*=============================================================
  SETUP.C  -  LixOS installer / HDD reset utility
  DJGPP / DOS

  Two modes:
    1. First-run install  : detects HDD, runs FDISK, FORMAT, copies files
    2. Already-installed  : warns user, offers HDD wipe / reinstall
=============================================================*/
#include "setup.h"
#include "../kernel/vga.h"
#include "../kernel/window.h"
#include "../kernel/mouse.h"
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define SETUP_W  260
#define SETUP_H  150

/* Setup pages */
#define PAGE_DETECT   0   /* Detecting drives */
#define PAGE_WARN     1   /* Already installed warning */
#define PAGE_CONFIRM  2   /* Final "are you SURE?" */
#define PAGE_FDISK    3   /* Running FDISK */
#define PAGE_FORMAT   4   /* Running FORMAT */
#define PAGE_COPY     5   /* Copying LixOS files */
#define PAGE_DONE     6   /* Finished */
#define PAGE_ABORT    7   /* User cancelled */

static int setup_win_id = -1;
static int setup_page   = PAGE_DETECT;
static int already_inst = 0;   /* 1 = C:\LIXOS found */
static char detected_drive[4] = "C:";
static int  prog_pct    = 0;   /* progress 0-100 */

/* ---- Helpers ---- */
static void draw_progress(int x, int y, int w, int pct) {
    vga_rect(x, y, w, 8, COL_WIN_DGRAY);
    vga_fillrect(x+1, y+1, (w-2)*pct/100, 6, COL_WIN_BLUE);
}

static void draw_centered(int win_x, int win_w, int y,
                           const char *s, unsigned char fg, unsigned char bg) {
    int tw = vga_strwidth(s);
    vga_putstr(win_x + (win_w - tw)/2, y, s, fg, bg);
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "r");
    if(f) { fclose(f); return 1; }
    return 0;
}

static void detect_drives(void) {
    /* Check if C:\LIXOS\LIXOS.EXE exists */
    already_inst = file_exists("C:\\LIXOS\\LIXOS.EXE");
    strcpy(detected_drive, "C:");
}

/* Shell out to DOS command (switches to text mode, runs, comes back) */
static void run_dos_cmd(const char *cmd) {
    vga_setmode(0x03);   /* back to text mode */
    system(cmd);
    vga_setmode(0x13);   /* back to graphics */
    vga_setpalette();
}

/* ---- Page renderers ---- */
static void page_detect(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "LixOS Setup", COL_WIN_BLUE, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);

    vga_putstr(cx+4, cy+18, "Scanning drives...", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+28, "Primary drive : C:", COL_DARK_BLUE, COL_WIN_GRAY);

    if(already_inst) {
        vga_putstr(cx+4, cy+40,
            "LixOS installation detected!", COL_DARK_RED, COL_WIN_GRAY);
        vga_putstr(cx+4, cy+50,
            "To reinstall, press Continue.", COL_BLACK, COL_WIN_GRAY);
    } else {
        vga_putstr(cx+4, cy+40,
            "No existing installation found.", COL_DARK_GREEN, COL_WIN_GRAY);
        vga_putstr(cx+4, cy+50,
            "Ready to install LixOS.", COL_BLACK, COL_WIN_GRAY);
    }

    gui_button(cx+10,  cy+SETUP_H-38, 60, 14, "Continue", 0);
    gui_button(cx+SETUP_W-74, cy+SETUP_H-38, 60, 14, "Cancel", 0);
}

static void page_warn(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "WARNING", COL_DARK_RED, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);

    vga_putstr(cx+4, cy+18,
        "LixOS is already installed.", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+30,
        "Continuing will ERASE ALL DATA", COL_DARK_RED, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+40,
        "on drive C: and reinstall.", COL_DARK_RED, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+54,
        "Are you sure you want to do this?", COL_BLACK, COL_WIN_GRAY);

    gui_button(cx+10,  cy+SETUP_H-38, 60, 14, "Yes, Wipe", 0);
    gui_button(cx+SETUP_W-74, cy+SETUP_H-38, 60, 14, "No, Exit", 0);
}

static void page_confirm(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Final Confirmation", COL_DARK_RED, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);

    vga_putstr(cx+4, cy+22,
        "LAST CHANCE!", COL_DARK_RED, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+34,
        "Drive C: will be wiped and", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+44,
        "LixOS will be reinstalled.", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+58,
        "Type YES to confirm:", COL_BLACK, COL_WIN_GRAY);
}

static void page_fdisk(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Step 1 of 3 - FDISK", COL_WIN_BLUE, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);
    vga_putstr(cx+4, cy+24,
        "Partitioning drive C: with FDISK", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+36,
        "This may require a restart.", COL_DARK_GRAY, COL_WIN_GRAY);
    draw_progress(cx+4, cy+54, SETUP_W-12, prog_pct);
    gui_button(cx+SETUP_W/2-30, cy+SETUP_H-38, 60, 14, "Run FDISK", 0);
}

static void page_format(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Step 2 of 3 - FORMAT", COL_WIN_BLUE, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);
    vga_putstr(cx+4, cy+24,
        "Formatting drive C: /S /V:LIXOS", COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+36,
        "This will take a few minutes.", COL_DARK_GRAY, COL_WIN_GRAY);
    draw_progress(cx+4, cy+54, SETUP_W-12, prog_pct);
    gui_button(cx+SETUP_W/2-30, cy+SETUP_H-38, 60, 14, "Format", 0);
}

static void page_copy(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Step 3 of 3 - Copying Files", COL_WIN_BLUE, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);
    vga_putstr(cx+4, cy+22, "Copying LixOS files to C:\\LIXOS\\",
               COL_BLACK, COL_WIN_GRAY);
    draw_progress(cx+4, cy+40, SETUP_W-12, prog_pct);
    char pstr[20];
    sprintf(pstr, "%d%%", prog_pct);
    vga_putstr(cx+SETUP_W/2-8, cy+52, pstr, COL_BLACK, COL_WIN_GRAY);
}

static void page_done(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Installation Complete!", COL_DARK_GREEN, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);
    vga_putstr(cx+4, cy+24, "LixOS has been installed on C:",
               COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+36, "Remove any floppy disks and",
               COL_BLACK, COL_WIN_GRAY);
    vga_putstr(cx+4, cy+46, "reboot to start LixOS.",
               COL_BLACK, COL_WIN_GRAY);
    gui_button(cx+SETUP_W/2-30, cy+SETUP_H-38, 60, 14, "Reboot", 0);
    gui_button(cx+SETUP_W/2+35, cy+SETUP_H-38, 44, 14, "Close", 0);
}

static void page_abort(int cx, int cy) {
    draw_centered(cx-4, SETUP_W, cy+4,
        "Setup Cancelled", COL_DARK_RED, COL_WIN_GRAY);
    vga_hline(cx, cx+SETUP_W-10, cy+14, COL_WIN_DGRAY);
    vga_putstr(cx+4, cy+30,
        "No changes were made.", COL_BLACK, COL_WIN_GRAY);
    gui_button(cx+SETUP_W/2-20, cy+SETUP_H-38, 40, 14, "OK", 0);
}

/* ---- Public API ---- */
void setup_open(void) {
    if(setup_win_id >= 0 && windows[setup_win_id].visible) return;
    setup_win_id = wm_open(30, 20, SETUP_W, SETUP_H, "LixOS Setup");
    setup_page = PAGE_DETECT;
    prog_pct   = 0;
    detect_drives();
}

/* Confirm buffer for PAGE_CONFIRM */
static char confirm_buf[8] = "";
static int  confirm_len    = 0;

int setup_update(void) {
    Window *w;
    int cx, cy;

    if(setup_win_id < 0 || !windows[setup_win_id].visible) return 0;
    w  = &windows[setup_win_id];
    cx = w->x + 4;
    cy = w->y + 2 + TITLE_H + 2;

    /* ---- Draw current page ---- */
    switch(setup_page) {
        case PAGE_DETECT:  page_detect(cx, cy);  break;
        case PAGE_WARN:    page_warn(cx, cy);     break;
        case PAGE_CONFIRM: page_confirm(cx, cy);  break;
        case PAGE_FDISK:   page_fdisk(cx, cy);    break;
        case PAGE_FORMAT:  page_format(cx, cy);   break;
        case PAGE_COPY:    page_copy(cx, cy);     break;
        case PAGE_DONE:    page_done(cx, cy);     break;
        case PAGE_ABORT:   page_abort(cx, cy);    break;
    }

    /* ---- Buttons ---- */
    if(mouse.left_click) {
        int bx1 = cx+10;
        int bx2 = cx+SETUP_W-74;
        int by  = cy+SETUP_H-38;

        switch(setup_page) {
            case PAGE_DETECT:
                if(gui_inside(mouse.x,mouse.y, bx1,by,60,14)) {
                    setup_page = already_inst ? PAGE_WARN : PAGE_FDISK;
                }
                if(gui_inside(mouse.x,mouse.y, bx2,by,60,14))
                    setup_page = PAGE_ABORT;
                break;

            case PAGE_WARN:
                if(gui_inside(mouse.x,mouse.y, bx1,by,60,14))
                    setup_page = PAGE_CONFIRM;
                if(gui_inside(mouse.x,mouse.y, bx2,by,60,14))
                    setup_page = PAGE_ABORT;
                break;

            case PAGE_FDISK:
                if(gui_inside(mouse.x,mouse.y,
                              cx+SETUP_W/2-30, by, 60, 14)) {
                    prog_pct = 0;
                    /* Launch FDISK in text mode */
                    run_dos_cmd("FDISK");
                    prog_pct = 100;
                    setup_page = PAGE_FORMAT;
                }
                break;

            case PAGE_FORMAT:
                if(gui_inside(mouse.x,mouse.y,
                              cx+SETUP_W/2-30, by, 60, 14)) {
                    prog_pct = 0;
                    run_dos_cmd("FORMAT C: /S /V:LIXOS");
                    prog_pct = 100;
                    setup_page = PAGE_COPY;
                }
                break;

            case PAGE_COPY: break; /* handled by keyboard / auto */

            case PAGE_DONE:
                /* Reboot */
                if(gui_inside(mouse.x,mouse.y,
                              cx+SETUP_W/2-30, by, 60, 14)) {
                    run_dos_cmd("CTTY NUL & REBOOT");  /* or use INT 19h */
                    /* INT 19h bootstrap reboot */
                    {
                        union REGS r;
                        int86(0x19, &r, &r);
                    }
                }
                /* Close */
                if(gui_inside(mouse.x,mouse.y,
                              cx+SETUP_W/2+35, by, 44, 14)) {
                    wm_close(setup_win_id);
                    setup_win_id=-1; return 0;
                }
                break;

            case PAGE_ABORT:
                if(gui_inside(mouse.x,mouse.y,
                              cx+SETUP_W/2-20, by, 40, 14)) {
                    wm_close(setup_win_id);
                    setup_win_id=-1; return 0;
                }
                break;
        }
    }

    /* ---- PAGE_CONFIRM keyboard input (type YES) ---- */
    if(setup_page == PAGE_CONFIRM) {
        int ky;
        /* Show what user typed */
        vga_putstr(cx+4+100, cy+58, confirm_buf, COL_DARK_RED, COL_WIN_GRAY);
        if(kbhit()) {
            ky = getch();
            if(ky == 27) { setup_page=PAGE_ABORT; confirm_len=0; confirm_buf[0]='\0'; }
            else if(ky == 13) {
                if(strcmp(confirm_buf,"YES")==0) {
                    confirm_len=0; confirm_buf[0]='\0';
                    setup_page = PAGE_FDISK;
                } else {
                    confirm_len=0; confirm_buf[0]='\0';
                }
            } else if(ky == 8 && confirm_len>0) {
                confirm_buf[--confirm_len]='\0';
            } else if(confirm_len < 5 && ky>='A' && ky<='Z') {
                confirm_buf[confirm_len++]=(char)ky;
                confirm_buf[confirm_len]='\0';
            } else if(confirm_len < 5 && ky>='a' && ky<='z') {
                confirm_buf[confirm_len++]=(char)(ky-32);
                confirm_buf[confirm_len]='\0';
            }
        }
    }

    /* ---- PAGE_COPY: simulate file copy progress ---- */
    if(setup_page == PAGE_COPY) {
        static int copy_tick = 0;
        copy_tick++;
        if(copy_tick > 5) {
            prog_pct += 2;
            copy_tick = 0;
        }
        if(prog_pct >= 100) {
            prog_pct = 100;
            /* Actually do the copy via XCOPY */
            run_dos_cmd("MD C:\\LIXOS");
            run_dos_cmd("XCOPY LIXOS\\*.* C:\\LIXOS\\ /S /E /Y");
            /* Write AUTOEXEC.BAT */
            {
                FILE *f = fopen("C:\\AUTOEXEC.BAT","w");
                if(f) {
                    fprintf(f, "@ECHO OFF\r\n");
                    fprintf(f, "SET PATH=C:\\LIXOS;C:\\DOS\r\n");
                    fprintf(f, "C:\\LIXOS\\LIXOS.EXE\r\n");
                    fclose(f);
                }
            }
            setup_page = PAGE_DONE;
        }
    }

    /* X button */
    if(wm_close_clicked(setup_win_id)) {
        wm_close(setup_win_id);
        setup_win_id=-1; return 0;
    }

    return 1;
}
