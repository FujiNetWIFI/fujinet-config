/**
 * FujiNet CONFIG for the S-100 card: platform-private declarations.
 *
 * One source tree builds CONFIG.COM (CP/M, console through the BDOS) and
 * CONFIG.BIN (S100_ROM: image 0 on the card, run from its boot page with no
 * OS, driving the console UART directly). Both draw on an 80x24 ANSI terminal.
 */

#ifndef S100_H
#define S100_H

#include <stdbool.h>
#include <stdint.h>

#define SCR_COLS 80
#define SCR_ROWS 24

/* Hosts and devices screen */
#define HOSTS_START_Y   3
#define DEVICES_START_Y 13
/* Mount screen: the device list alone */
#define DEVICES_START_MOUNT_Y 3
/* File list; the prev/next page markers sit just above and below it */
#define FILES_START_Y   4
#define NETWORKS_START_Y 3
#define ROW_INFO 21 /* long filenames */
#define ROW_MSG  22 /* errors and prompts */
#define ROW_HELP 23 /* key help */

/* Console types in the card info block */
#define CON_2SIO 0 /* 88-2SIO 6850 at 10h/11h */
#define CON_SIO  1 /* 88-SIO at 00h/01h */
#define CON_8251 2 /* IMSAI SIO-2 8251 at 02h/03h */
#define CON_BDOS 3

/* Card info, as the card patches it into the image or reports it in a LOAD header */
typedef struct
{
    uint8_t base;    /* pipe base port */
    uint8_t page;    /* boot page high byte */
    uint8_t console;
    uint8_t flags;
    uint8_t spt;     /* drive 0 sectors per track */
} S100Info;

#define S100_INFOF_PHANTOM 0x01

extern S100Info s100;

/* portio.asm */
uint8_t s100_in(uint8_t port) __z88dk_fastcall;
void s100_out(uint8_t port, uint8_t v);

/* con.c */
void con_init(void);
void con_putc(char c);
void con_print(const char *s);
uint8_t con_getc(void);
void con_wait_ms(uint16_t ms);
uint8_t con_getc_wait(uint8_t ms);

/* screen.c */
extern char s100_shadow[SCR_ROWS * SCR_COLS];
void s100_goto(uint8_t x, uint8_t y);
void s100_puts(uint8_t x, uint8_t y, const char *s);
void s100_putc(uint8_t x, uint8_t y, char c);
void s100_reverse(bool on);
void s100_cursor(bool on);
void s100_status(const char *help);
char *s100_utoa(uint16_t v, char *buf);

/* bar.c */
void bar_forget(void);

/* input.c */
extern bool s100_quit;

#endif /* S100_H */
