#ifndef MOUSE_H
#define MOUSE_H

/* DOS mouse driver via INT 33h */

typedef struct {
    int x, y;
    int buttons;        /* bit0=left, bit1=right, bit2=middle */
    int left;           /* left button down */
    int right;          /* right button down */
    int left_click;     /* just clicked this frame */
    int right_click;
} Mouse;

extern Mouse mouse;

int  mouse_init(void);       /* returns 1 if driver present */
void mouse_update(void);     /* call once per frame */
void mouse_show(void);
void mouse_hide(void);
void mouse_draw_cursor(void);/* draw our own software cursor */

#endif
