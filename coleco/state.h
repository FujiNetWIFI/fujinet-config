/* state.h -- the shared globals and the seams between the screen modules.
 *
 * One invariant to keep when adding code: the reply window at $F800 is only
 * repainted by committing a transaction. Any flow that edits something and
 * writes it back must therefore re-read first and stream out of the fresh
 * window (the way rename and the lobby claim do) -- never hold a window
 * across an intervening transaction. fn_edit() runs no transactions, which
 * is what makes "fetch, edit, stream back" safe.
 */

#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include <fujinet-fuji.h>
#include <fujinet-coleco.h>
#include <fujinet-bus-coleco.h>

#include "constants.h"

extern unsigned char state;

extern unsigned char host;       /* the mounted/browsed host slot */
extern unsigned char src_host;   /* copy: the source's host slot */
extern unsigned char copy_mode;  /* nonzero while a copy source is marked */

extern unsigned char cur;        /* selection bar row within the window */
extern unsigned char nrows;      /* rows the current list actually has */
extern unsigned char at_end;     /* the visible page ends the directory */
extern unsigned int  top;        /* first visible entry (16-bit: big dirs) */
extern unsigned char hosts_mask; /* bit i set = host slot i is nonempty */

extern char path[PATH_MAX_LEN];  /* current dir: '/'-prefixed, '/'-terminated */
extern char src_spec[SPEC_MAX_LEN]; /* copy source full path; also the
                                       custom-SSID scratch (never both) */

/* config.c */
void status_line(const char *s);            /* row 22, cleared first */
void legend_line(const char *s);            /* row 23, cleared first */
void fail(const char *what);                /* row 22 + FN_ERRCODE in hex */
void draw_frame(const char *subtitle);      /* cls + title (+ row-3 subtitle) */
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
void st_lobby(void);

/* st_files.c -- shared with the copy and boot flows */
bool dir_seek(unsigned int pos);
bool dir_read_entry(unsigned char maxlen);  /* false at end/error; guards the
                                               '.'/'..' poison, see the impl */
bool dir_open(void);                        /* OPEN_DIRECTORY: host, path,
                                               filter (= fn_entry) */
void files_draw(void);                      /* full redraw */
void files_legend(void);                    /* just rows 22-23 */
void leave_host(void);                      /* close dir, back to ST_HOSTS */

/* st_copy.c */
void copy_mark(void);                       /* keypad 5, no copy in flight */
void copy_here(void);                       /* keypad 5, copy in flight */
void copy_cancel(void);                     /* back to the source browse */

/* st_boot.c */
void boot_reply_entry(void);                /* boot path + name-in-window;
                                               returns only on failure */
void boot_mount_swap(void);                 /* MOUNT_IMAGE + progress + swap;
                                               returns only on failure */

#endif /* STATE_H */
