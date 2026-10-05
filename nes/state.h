/* state.h -- the shared globals and the seams between the screen modules.
 *
 * One invariant to keep when adding code: the reply window at $5000 is only
 * repainted by committing a transaction. Any flow that edits something and
 * writes it back must therefore re-read first and stream out of the fresh
 * window (the way rename does) -- never hold a window across an intervening
 * transaction. fn_edit() runs no transactions, which is what makes "fetch,
 * edit, stream back" safe.
 */

#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include <fujinet-fuji.h>
#include <fujinet-nes.h>
#include <fujinet-bus-nes.h>

#include "constants.h"

extern unsigned char state;

extern unsigned char host;       /* the mounted/browsed host slot */
extern unsigned char cur;        /* selection bar row within the window */
extern unsigned char nrows;      /* rows the current list actually has */
extern unsigned char at_end;     /* the visible page ends the directory */
extern unsigned int  top;        /* first visible entry (16-bit: big dirs) */
extern unsigned char hosts_mask; /* bit i set = host slot i is nonempty */

extern char path[PATH_MAX_LEN];  /* current dir: '/'-prefixed, '/'-terminated */
extern char src_spec[SPEC_MAX_LEN]; /* custom-SSID scratch */

/* config.c */
void status_line(const char *s);            /* STATUS_ROW, cleared first */
void legend_line(const char *s);            /* LEGEND_ROW, cleared first */
void fail(const char *what);                /* "?WHAT ERROR xx" + BEEP */
void draw_frame(const char *title);         /* cls + the frame, titled */
void wait_frames(unsigned char n);          /* n vblanks, input ignored */
void bar_move(signed char d);               /* the non-paging selection bar */

/* One screen per state; each draws itself, runs its own event loop, and
 * returns once it has set `state` to something else. */
void st_check_wifi(void);
void st_set_wifi(void);
void st_connect_wifi(void);
void st_hosts(void);
void st_files(void);
void st_info(void);

/* st_files.c -- shared with the boot flow */
bool dir_seek(unsigned int pos);
bool dir_read_entry(unsigned char maxlen);
bool dir_open(void);
void files_draw(void);
void files_legend(void);
void leave_host(void);

/* st_boot.c */
void boot_reply_entry(void);                /* boot path + name-in-window;
                                               returns only on failure */
void boot_mount_swap(void);                 /* MOUNT_IMAGE + progress + boot;
                                               returns only on failure */
void boot_lobby(void);                      /* LOBBY_PATH on `host`;
                                               returns only on failure */

#endif /* STATE_H */
