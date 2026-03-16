#ifdef BUILD_MSDOS

#ifndef SCREEN_MSDOS_H
#define SCREEN_MSDOS_H

#include <stdbool.h>
#include <fujinet-fuji.h>

/**
 * @brief The standard RGBI color palette for text mode.
 */
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHT_GRAY 7
#define DARK_GRAY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define YELLOW 14
#define WHITE 15

/* EDIT.EXE inspired color scheme */
#define ATTRIBUTE_HEADER   0x70  /* black on light gray  (title/section bars) */
#define ATTRIBUTE_NORMAL   0x17  /* white on blue        (body text)          */
#define ATTRIBUTE_BOLD     0x1F  /* bright white on blue (emphasized text)    */
#define ATTRIBUTE_SELECTED 0x70  /* black on light gray  (list selection bar) */
#define ATTRIBUTE_STATUS   0x70  /* black on light gray  (bottom status bar)  */
#define ATTRIBUTE_SHADOW   0x08  /* dark gray on black   (drop shadow)        */

/**
 * @brief screen variables
 */
extern unsigned char screen_mode;
extern unsigned char screen_cols;
extern unsigned char far *video;

void font_init(void);
void screen_putc(unsigned char x, unsigned char y, unsigned char a, const char c);
void screen_puts(unsigned char x, unsigned char y, unsigned char a, const char *s);
void screen_puts_center(unsigned char y, unsigned char a, const char *s);
void set_wifi_print_rssi(SSIDInfo *s, unsigned char i);
void screen_show_info(int printerEnabled, AdapterConfig *ac);

/* EDIT.EXE style UI helpers */
void screen_header(const char *title);
void screen_status(const char *text);
void screen_fill_line(unsigned char y, unsigned char a);
void screen_draw_box(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char a, bool shadow);
void screen_draw_box_titled(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char a, bool shadow, const char *title);

#endif /* SCREEN_MSDOS_H */

#endif /* BUILD_MSDOS */
