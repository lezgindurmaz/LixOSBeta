/*=============================================================
  MOUSE.C  -  DOS INT 33h mouse driver for LixOS
=============================================================*/
#include "mouse.h"
#include "vga.h"
#include <dos.h>

Mouse mouse;

static int prev_left  = 0;
static int prev_right = 0;

int mouse_init(void) {
    union REGS r;
    r.x.ax = 0x0000;   /* reset + get status */
    int86(0x33, &r, &r);
    if(r.x.ax == 0) return 0;  /* no driver */
    /* Set horizontal range 0..639 (standard for VGA) */
    r.x.ax = 0x0007;
    r.x.cx = 0;
    r.x.dx = 639;
    int86(0x33, &r, &r);
    /* Set vertical range 0..199 */
    r.x.ax = 0x0008;
    r.x.cx = 0;
    r.x.dx = 199;
    int86(0x33, &r, &r);
    mouse.x = 160;
    mouse.y = 100;
    return 1;
}

void mouse_update(void) {
    union REGS r;
    /* INT 33h AX=3: get button status + position */
    r.x.ax = 0x0003;
    int86(0x33, &r, &r);
    mouse.buttons = r.x.bx;
    /* In mode 13h (320 wide) the BIOS reports x in 0..639, divide by 2 */
    mouse.x = r.x.cx / 2;
    mouse.y = r.x.dx;
    /* Clamp */
    if(mouse.x < 0)   mouse.x = 0;
    if(mouse.x > 319) mouse.x = 319;
    if(mouse.y < 0)   mouse.y = 0;
    if(mouse.y > 199) mouse.y = 199;
    mouse.left  = (mouse.buttons & 1);
    mouse.right = (mouse.buttons & 2) >> 1;
    /* Rising edge detection */
    mouse.left_click  = (mouse.left  && !prev_left);
    mouse.right_click = (mouse.right && !prev_right);
    prev_left  = mouse.left;
    prev_right = mouse.right;
}

/* Draw a 7x11 arrow cursor directly into the buffer */
void mouse_draw_cursor(void) {
    static const unsigned char cur[11][7] = {
        {1,0,0,0,0,0,0},
        {1,1,0,0,0,0,0},
        {1,2,1,0,0,0,0},
        {1,2,2,1,0,0,0},
        {1,2,2,2,1,0,0},
        {1,2,2,2,2,1,0},
        {1,2,2,2,2,2,1},
        {1,2,2,1,1,0,0},
        {1,2,1,0,0,0,0},
        {1,1,0,0,0,0,0},
        {1,0,0,0,0,0,0},
    };
    int r, c, px, py;
    for(r=0; r<11; r++) {
        for(c=0; c<7; c++) {
            px = mouse.x + c;
            py = mouse.y + r;
            if(px>=0 && px<SCREEN_W && py>=0 && py<SCREEN_H) {
                if(cur[r][c] == 1)
                    vga_buffer[py][px] = COL_BLACK;
                else if(cur[r][c] == 2)
                    vga_buffer[py][px] = COL_WHITE;
            }
        }
    }
}
