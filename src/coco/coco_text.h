#ifndef COCO_TEXT_H
#define COCO_TEXT_H

/* Character-cell drawing, implemented by text_hires.c (CoCo 1/2) and text_gime.c (CoCo 3). */

#ifdef COCO3
#define SCREEN_COLS    40
#define NUM_COLORSETS  10
#else
#define SCREEN_COLS    42
#define NUM_COLORSETS  4
#endif
#define SCREEN_ROWS    24

/* CoCo 1/2 draws TITLE and WRITE bold and KEY reversed; the rest plain. */
enum
{
  ROLE_TEXT,
  ROLE_TITLE,
  ROLE_BOX,
  ROLE_KEY,
  ROLE_ERROR,
  ROLE_STATUS,
  ROLE_EMPTY,
  ROLE_READ,
  ROLE_WRITE
};

extern byte colorset;
#ifdef COCO3
extern byte composite;
#endif

void txt_open(void);
void txt_close(void);
/* Before Basic runs again: unhooks hirestxt's printf (CoCo 1/2). Screen stays up. */
void txt_release(void);
void txt_clear(void);
void txt_put(byte x, byte y, byte c, byte role);
void txt_invert(byte x, byte y);
/* Applies colorset; true if the screen must be redrawn to show it. */
bool txt_colors(void);

#endif
